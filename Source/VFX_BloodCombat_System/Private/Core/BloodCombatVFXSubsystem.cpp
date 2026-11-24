
#include "Core/BloodCombatVFXSubsystem.h"
#include "Data/VfxBloodCombatConfigDataAsset.h"
#include "Components/DecalComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"

void UBloodCombatVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// The plugin owns a single config asset so worlds work without extra setup; projects can override it with SetConfigDataAsset.
	static const TCHAR* ConfigAssetPath = TEXT("/VFX_BloodCombat_System/Data/VFX_DataAsset.VFX_DataAsset");
	ConfigDataAsset = LoadObject<UVfxBloodCombatConfigDataAsset>(nullptr, ConfigAssetPath);

	if (!ConfigDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("BloodCombatVFX: no se pudo cargar el Data Asset en %s"), ConfigAssetPath);
	}
}

void UBloodCombatVFXSubsystem::Deinitialize()
{
	// Destroy pooled components and handlers explicitly: subsystems outlive actors, but their spawned objects do not survive the world.
	for (const TObjectPtr<UNiagaraComponent>& ComponentPtr : AllComponents)
	{
		if (UNiagaraComponent* Component = ComponentPtr.Get())
		{
			Component->OnSystemFinished.RemoveAll(this);
			Component->DeactivateImmediate();
			Component->DestroyComponent();
		}
	}
	for (const TObjectPtr<AActor>& HandlerPtr : AllDecalHandlers)
	{
		if (AActor* Handler = HandlerPtr.Get())
		{
			Handler->Destroy();
		}
	}
	AllDecalHandlers.Empty();
	AvailableDecalHandlers.Empty();
	ActiveDecalHandlers.Empty();

	AllComponents.Empty();
	AvailableComponents.Empty();
	ActiveComponentSystems.Empty();

	if (UProceduralMeshComponent* Brush = PersistentBrushMesh.Get())
	{
		Brush->DestroyComponent();
	}
	PersistentBrushMesh = nullptr;
	BrushState = FBloodBrushState();

	Super::Deinitialize();
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnSystem(UNiagaraSystem* System, const FBloodCombatVFXParams& Params)
{
	return SpawnSystemInternal(System, Params, nullptr);
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnSystemInternal(
	UNiagaraSystem* System,
	const FBloodCombatVFXParams& Params,
	TFunction<void(UNiagaraComponent*)> SetupComponent)
{
	if (!System || !GetWorld())
	{
		return nullptr;
	}

	UNiagaraComponent* Component = AcquireComponent(System);
	if (!Component)
	{
		return nullptr;
	}

	// An explicitly zero rotation means "face along the surface normal".
	const FRotator SpawnRotation = Params.Rotation.IsNearlyZero() ? Params.Normal.Rotation() : Params.Rotation;
	Component->SetWorldLocationAndRotation(Params.Location, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);

	ApplyParams(Component, Params);

	// Trails and blood bursts use this hook to inject their own Niagara user variables before activation.
	if (SetupComponent)
	{
		SetupComponent(Component);
	}

	Component->Activate(true);

	return Component;
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnImpact(ECombatImpactType Type, const FBloodCombatVFXParams& Params)
{
	UNiagaraSystem* System = ResolveImpactSystem(Type, Params.SurfaceType);
	if (!System)
	{
		return nullptr;
	}

	return SpawnSystem(System, Params);
}

void UBloodCombatVFXSubsystem::SpawnImpactEffects(ECombatImpactType Type, const FBloodCombatVFXParams& Params)
{
	if (!ConfigDataAsset)
	{
		return;
	}

	SpawnSystem(ConfigDataAsset->GetImpactSparkSystem(Type, Params.SurfaceType), Params);
	SpawnSystem(ConfigDataAsset->GetImpactFlashSystem(Type, Params.SurfaceType), Params);
}

UDecalComponent* UBloodCombatVFXSubsystem::SpawnBloodDecal(const FBloodCombatVFXParams& Params)
{
	if (!ConfigDataAsset || !GetWorld())
	{
		return nullptr;
	}

  UMaterialInterface* Material = ConfigDataAsset->GetBloodDecalMaterial();
	if (!Material)
	{
		return nullptr;
	}

	const FRotator DecalRotation = Params.Rotation.IsNearlyZero()
		? Params.Normal.Rotation()
		: Params.Rotation;

	// Attached decals follow moving geometry; convert world-space impact data into the hit component's local space.
	if (UPrimitiveComponent* HitComponent = Params.HitComponent.Get())
	{
		const FTransform ComponentTransform = HitComponent->GetComponentTransform();
		return UGameplayStatics::SpawnDecalAttached(
			Material,
			Params.DecalSize,
			HitComponent,
			NAME_None,
			ComponentTransform.InverseTransformPosition(Params.Location),
			ComponentTransform.InverseTransformRotation(DecalRotation.Quaternion()).Rotator(),
			EAttachLocation::KeepRelativeOffset,
			Params.DecalLifeSpan);
	}

	// Without a hit component, offset along the normal to avoid z-fighting with the surface.
	return UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(),
		Material,
		Params.DecalSize,
		Params.Location + Params.Normal * 0.5f,
		DecalRotation,
		Params.DecalLifeSpan);
}

void UBloodCombatVFXSubsystem::SetConfigDataAsset(UVfxBloodCombatConfigDataAsset* NewConfigDataAsset)
{
	ConfigDataAsset = NewConfigDataAsset;
}

AActor* UBloodCombatVFXSubsystem::AcquireDecalHandler()
{
	if (!GetWorld())
	{
		return nullptr;
	}

	// Prefer a free handler; stale weak pointers from destroyed actors are discarded while scanning.
	while (AvailableDecalHandlers.Num() > 0)
	{
		AActor* Handler = AvailableDecalHandlers.Pop(EAllowShrinking::No).Get();
		if (IsValid(Handler))
		{
			return Handler;
		}
	}

	// Loaded by path so the C++ module has no hard dependency on the Blueprint-generated class.
	static const TCHAR* HandlerClassPath = TEXT("/VFX_BloodCombat_System/Blueprints/BP_DecalComponent.BP_DecalComponent_C");
	UClass* HandlerClass = LoadClass<AActor>(nullptr, HandlerClassPath);
	if (!HandlerClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("BloodCombatVFX: no se pudo cargar el handler de decals en %s"), HandlerClassPath);
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Handler = GetWorld()->SpawnActor<AActor>(
		HandlerClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams);

	if (!Handler)
	{
		return nullptr;
	}

	AllDecalHandlers.Add(Handler);
	return Handler;
}

void UBloodCombatVFXSubsystem::SetupDecalHandler(
	UNiagaraComponent* Component,
	EBloodEffectType Type,
	const FBloodCombatVFXParams& Params)
{
	if (!Component || !ConfigDataAsset)
	{
		return;
	}

	UMaterialInterface* DecalMaterial = ConfigDataAsset->GetBloodDecalMaterialForEffect(Type);
	if (!DecalMaterial)
	{
		return;
	}

	AActor* Handler = AcquireDecalHandler();
	if (!Handler)
	{
		return;
	}

	// Property names are set by reflection: the handler is a Blueprint actor owned by content, not by this module.
	SetObjectProperty(Handler, FName(TEXT("MaterialDecal")), DecalMaterial);
	SetObjectProperty(Handler, FName(TEXT("NiagaraSystemBurst")), Component->GetAsset());
	SetVectorProperty(Handler, FName(TEXT("TamanoDecal")), Params.DecalSize);

	// The Niagara graph reads this handler back when it needs to spawn the decal.
	Component->SetVariableObject(FName(TEXT("User.CallbackHandler")), Handler);
	ActiveDecalHandlers.Add(Component, Handler);
}

void UBloodCombatVFXSubsystem::ReleaseDecalHandler(UNiagaraComponent* Component)
{
	AActor** HandlerPtr = ActiveDecalHandlers.Find(Component);
	if (!HandlerPtr)
	{
		return;
	}

	AActor* Handler = *HandlerPtr;
	if (Component)
	{
		Component->SetVariableObject(FName(TEXT("User.CallbackHandler")), nullptr);
	}

	if (IsValid(Handler))
	{
		SetObjectProperty(Handler, FName(TEXT("MaterialDecal")), nullptr);
		SetObjectProperty(Handler, FName(TEXT("NiagaraSystemBurst")), nullptr);
		AvailableDecalHandlers.Add(Handler);
	}

	ActiveDecalHandlers.Remove(Component);
}

void UBloodCombatVFXSubsystem::SetObjectProperty(AActor* Actor, FName PropertyName, UObject* Value)
{
	if (!Actor)
	{
		return;
	}

	// Blueprint variables are reflected like native properties, so no generated header is needed here.
	if (FObjectProperty* Property = FindFProperty<FObjectProperty>(Actor->GetClass(), PropertyName))
	{
		Property->SetObjectPropertyValue(Property->ContainerPtrToValuePtr<void>(Actor), Value);
	}
}

void UBloodCombatVFXSubsystem::SetVectorProperty(AActor* Actor, FName PropertyName, const FVector& Value)
{
	if (!Actor)
	{
		return;
	}

	if (FStructProperty* Property = FindFProperty<FStructProperty>(Actor->GetClass(), PropertyName))
	{
		if (Property->Struct == TBaseStructure<FVector>::Get())
		{
			*Property->ContainerPtrToValuePtr<FVector>(Actor) = Value;
		}
	}
}

void UBloodCombatVFXSubsystem::SpawnBloodBurst(const FBloodCombatVFXParams& Params)
{
	SpawnBlood(EBloodEffectType::Burst, Params);
	return;
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnBlood(EBloodEffectType Type, const FBloodCombatVFXParams& Params)
{
    if (!ConfigDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("BloodCombatVFX: no hay Data Asset configurado para el efecto %d."), static_cast<int32>(Type));
		return nullptr;
	}

	UNiagaraSystem* System = ResolveBloodSystem(Type);
	if (!System)
	{
     UE_LOG(LogTemp, Warning, TEXT("BloodCombatVFX: no hay Niagara configurado para el efecto de sangre %d."), static_cast<int32>(Type));
		return nullptr;
	}

	return SpawnSystemInternal(System, Params, [this, Type, Params](UNiagaraComponent* Component)
	{
		SetupDecalHandler(Component, Type, Params);
	});
}

void UBloodCombatVFXSubsystem::SpawnBloodSpray(const FBloodCombatVFXParams& Params)
{
	SpawnBlood(EBloodEffectType::Spray, Params);
	return;
}

void UBloodCombatVFXSubsystem::SpawnBloodTrail(const FBloodCombatVFXParams& Params)
{
	SpawnBlood(EBloodEffectType::Trail, Params);
	return;
}

UNiagaraComponent* UBloodCombatVFXSubsystem::StartBloodTrail(const FBloodCombatVFXParams& Params)
{
	return StartTrail(EBloodEffectType::Trail, Params);
}

UNiagaraComponent* UBloodCombatVFXSubsystem::StartSwordBloodTrail(
	const FBloodCombatVFXParams& Params,
	USkeletalMeshComponent* SwordMesh,
	const TArray<FName>& FilteredSockets)
{
	if (!SwordMesh || FilteredSockets.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("BloodCombatVFX: no se pudo iniciar el trail de espada; faltan malla o sockets."));
		return nullptr;
	}

	return StartTrail(EBloodEffectType::SwordBloodTrail, Params, [SwordMesh, FilteredSockets](UNiagaraComponent* TrailComponent)
	{
		// Bind the skeletal mesh data interface and restrict it to the sockets that form the blade.
		UNiagaraFunctionLibrary::OverrideSystemUserVariableSkeletalMeshComponent(
			TrailComponent, TEXT("User.SwordMesh"), SwordMesh);
		UNiagaraFunctionLibrary::SetSkeletalMeshDataInterfaceFilteredSockets(
			TrailComponent, TEXT("User.SwordMesh"), FilteredSockets);
	});
}

UNiagaraComponent* UBloodCombatVFXSubsystem::StartTrail(
	EBloodEffectType Type,
	const FBloodCombatVFXParams& Params,
	TFunction<void(UNiagaraComponent*)> SetupComponent)
{
	UNiagaraSystem* System = ResolveBloodSystem(Type);
	if (!System)
	{
		UE_LOG(LogTemp, Warning, TEXT("BloodCombatVFX: no hay Niagara configurado para el trail de sangre (%d)."), static_cast<int32>(Type));
		return nullptr;
	}

	return SpawnSystemInternal(System, Params, [Params, SetupComponent](UNiagaraComponent* TrailComponent) mutable
	{
		TrailComponent->SetVariablePosition(FName(TEXT("User.TrailPosition")), Params.Location);

		if (SetupComponent)
		{
			SetupComponent(TrailComponent);
		}
	});
}

void UBloodCombatVFXSubsystem::UpdateBloodTrail(UNiagaraComponent* TrailComponent, const FVector& WorldPosition)
{
	if (!IsValid(TrailComponent))
	{
		return;
	}

	TrailComponent->SetVariablePosition(FName(TEXT("User.TrailPosition")), WorldPosition);
}

void UBloodCombatVFXSubsystem::StopBloodTrail(UNiagaraComponent* TrailComponent)
{
	if (!IsValid(TrailComponent))
	{
		return;
	}

	ReleaseComponent(TrailComponent);
}

void UBloodCombatVFXSubsystem::StopSwordBloodTrail(UNiagaraComponent* TrailComponent)
{
	StopBloodTrail(TrailComponent);
}

UProceduralMeshComponent* UBloodCombatVFXSubsystem::StartBloodBrush(
	const FBloodCombatVFXParams& Params,
	UPrimitiveComponent* IgnoreSwordComponent)
{
	UWorld* World = GetWorld();
	if (!World || !ConfigDataAsset)
	{
		return nullptr;
	}

	UMaterialInterface* BrushMaterial = ConfigDataAsset->GetBrushMaterial();
	if (!BrushMaterial)
	{
		// Keep the brush visible even when the config asset has no explicit material assigned.
		static const TCHAR* FallbackMaterialPath = TEXT("/VFX_BloodCombat_System/Materials/SwordBloodTrail_Mat.SwordBloodTrail_Mat");
		BrushMaterial = LoadObject<UMaterialInterface>(nullptr, FallbackMaterialPath);
	}

	if (!BrushMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("BloodCombatVFX: no hay material configurado para el pincel de sangre"));
		return nullptr;
	}

	UProceduralMeshComponent* Brush = EnsureBrushMesh(BrushMaterial);
	if (!Brush)
	{
		return nullptr;
	}

	// Exclude the attacker, the hit component and the sword collision from projection traces.
	BrushState.Instigator = Params.Instigator;
	BrushState.IgnoreComponent = Params.HitComponent;
	BrushState.IgnoreSwordComponent = IgnoreSwordComponent;
	BrushState.bHasLastBladePosition = false;
	BrushState.ProjectionDirection = FVector::ForwardVector;
	if (const FVector* Direction = Params.VectorParameters.Find(FName(TEXT("User.Direction"))))
	{
		BrushState.ProjectionDirection = Direction->GetSafeNormal();
		if (BrushState.ProjectionDirection.IsNearlyZero())
		{
			BrushState.ProjectionDirection = FVector::ForwardVector;
		}
	}

	// Copy sizing into the state once so later updates do not depend on asset loads.
	BrushState.Width = ConfigDataAsset->GetBrushWidth();
	BrushState.UVScaleU = ConfigDataAsset->GetBrushUVScaleU();
	BrushState.UVScaleV = ConfigDataAsset->GetBrushUVScaleV();
	// Clamp sample spacing to avoid dense, overlapping quads on narrow brushes.
	BrushState.MinPointDistance = FMath::Max(2.0f, BrushState.Width * 0.25f);
	BrushState.AccumulatedDistance = 0.0f;
	BrushState.bNeedsNewStrip = true;

	UE_LOG(LogTemp, Log, TEXT("BloodBrush: inicio material=%s ancho=%.1f UV=%.2f/%.2f dir=%s"),
		*BrushMaterial->GetName(), BrushState.Width, BrushState.UVScaleU, BrushState.UVScaleV,
		*BrushState.ProjectionDirection.ToString());

	return Brush;
}

UProceduralMeshComponent* UBloodCombatVFXSubsystem::EnsureBrushMesh(UMaterialInterface* Material)
{
	UWorld* World = GetWorld();
	if (!World || !Material)
	{
		return nullptr;
	}

	// One mesh is shared by every stroke; its vertices are already in world space, so the component stays at the origin.
	if (UProceduralMeshComponent* Existing = PersistentBrushMesh.Get())
	{
		Existing->SetMaterial(0, Material);
		return Existing;
	}

	UProceduralMeshComponent* Brush = NewObject<UProceduralMeshComponent>(World->GetWorldSettings());
	if (!Brush)
	{
		return nullptr;
	}

	// The brush is visual-only: no collision, no shadows, and a fixed identity transform.
	Brush->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Brush->SetCastShadow(false);
	Brush->RegisterComponentWithWorld(World);
	Brush->SetMaterial(0, Material);
	Brush->SetVisibility(true);
	Brush->SetWorldLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);

	// Reserve enough space for a few hundred samples before the first reallocation.
	BrushState.Points.Reserve(2048);
	BrushState.Normals.Reserve(2048);
	BrushState.UVs.Reserve(2048);
	BrushState.Triangles.Reserve(3072);

	PersistentBrushMesh = Brush;
	return Brush;
}

void UBloodCombatVFXSubsystem::AddStrokeConnection()
{
	// A new stroke intentionally skips bridge triangles, so it reads as a separate strip instead of a long smear.
	BrushState.bNeedsNewStrip = false;
	BrushState.bHasLastBladePosition = false;
}

void UBloodCombatVFXSubsystem::UpdateBloodBrush(UProceduralMeshComponent* Brush, const FVector& WorldPosition)
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(Brush))
	{
		return;
	}

	// The whole section is rebuilt per sample, so cap the buffer to keep that cost bounded.
	constexpr int32 MaxBrushPoints = 4096;
	if (BrushState.PointCount >= MaxBrushPoints)
	{
		return;
	}

	const bool bStartingNewStroke = BrushState.bNeedsNewStrip;

	// Prefer the real blade travel direction; fall back to the impact direction when the blade barely moved.
	FVector Direction = BrushState.ProjectionDirection;
	if (BrushState.bHasLastBladePosition)
	{
		const FVector BladeDelta = WorldPosition - BrushState.LastBladePosition;
		if (BladeDelta.SizeSquared() > FMath::Square(0.5f))
		{
			Direction = BladeDelta.GetSafeNormal();
		}
	}
	BrushState.LastBladePosition = WorldPosition;
	BrushState.bHasLastBladePosition = true;
	if (Direction.IsNearlyZero())
	{
		Direction = BrushState.ProjectionDirection;
	}

	FCollisionQueryParams QueryParams(FName(TEXT("BloodBrush")), false, BrushState.Instigator.Get());
	if (UPrimitiveComponent* IgnoredComponent = BrushState.IgnoreComponent.Get())
	{
		QueryParams.AddIgnoredComponent(IgnoredComponent);
		if (AActor* IgnoredActor = IgnoredComponent->GetOwner())
		{
			QueryParams.AddIgnoredActor(IgnoredActor);
		}
	}

	if (UPrimitiveComponent* IgnoredSword = BrushState.IgnoreSwordComponent.Get())
	{
		QueryParams.AddIgnoredComponent(IgnoredSword);
		if (AActor* SwordOwner = IgnoredSword->GetOwner())
		{
			QueryParams.AddIgnoredActor(SwordOwner);
		}
	}

	// Only static geometry receives brush strokes; dynamic actors and the attacker are ignored.
	FCollisionObjectQueryParams ObjectParams(ECC_WorldStatic);
	FHitResult Hit;
	// First try to project along the swing; this handles walls and vertical surfaces.
	bool bHit = World->LineTraceSingleByObjectType(
		Hit,
		WorldPosition - Direction * 50.0f,
		WorldPosition + Direction * 600.0f,
		ObjectParams,
		QueryParams);

	// Fall back to a downward trace for floors when the swing direction misses.
	if (!bHit)
	{
		bHit = World->LineTraceSingleByObjectType(
			Hit,
			WorldPosition + FVector(0.0f, 0.0f, 100.0f),
			WorldPosition - FVector(0.0f, 0.0f, 400.0f),
			ObjectParams,
			QueryParams);
	}

	if (bDebugBrush)
	{
		DrawDebugLine(
			World,
			WorldPosition - Direction * 50.0f,
			WorldPosition + Direction * 600.0f,
			bHit ? FColor::Green : FColor::Yellow,
			false,
			5.0f,
			0,
			1.0f);
	}

	if (!bHit)
	{
		return;
	}

	// Lift the sample slightly off the surface to avoid z-fighting with the hit geometry.
	const FVector SurfacePoint = Hit.ImpactPoint + Hit.ImpactNormal * 1.5f;
	// Skip samples that are too close together; otherwise the ribbon overlaps itself and wastes triangles.
	if (!bStartingNewStroke && BrushState.PointCount > 0 &&
		FVector::DistSquared(BrushState.Points[(BrushState.PointCount - 1) * 2], SurfacePoint) < FMath::Square(BrushState.MinPointDistance))
	{
		return;
	}

	FVector StrokeDirection = FVector::VectorPlaneProject(Direction, Hit.ImpactNormal).GetSafeNormal();
	// For impacts head-on to the swing, use world up projected onto the surface as the ribbon axis.
	if (StrokeDirection.IsNearlyZero())
	{
		StrokeDirection = FVector::VectorPlaneProject(FVector::UpVector, Hit.ImpactNormal).GetSafeNormal();
	}

	// The ribbon half-width extends along the cross product of normal and stroke direction.
	const FVector Side = FVector::CrossProduct(Hit.ImpactNormal, StrokeDirection).GetSafeNormal();
	if (Side.IsNearlyZero())
	{
		return;
	}

	if (bDebugBrush)
	{
		DrawDebugPoint(World, SurfacePoint, 12.0f, FColor::Red, false, 5.0f);
		DrawDebugDirectionalArrow(World, SurfacePoint, SurfacePoint + Side * 40.0f, 6.0f, FColor::Blue, false, 5.0f, 0, 2.0f);
	}

	// Each sample contributes two vertices; triangles are only emitted between samples of the same strip.
	const int32 VertexIndex = BrushState.PointCount * 2;
	const int32 RequiredNum = VertexIndex + 2;
	if (BrushState.Points.Num() < RequiredNum)
	{
		BrushState.Points.SetNum(RequiredNum, EAllowShrinking::No);
		BrushState.Normals.SetNum(RequiredNum, EAllowShrinking::No);
		BrushState.UVs.SetNum(RequiredNum, EAllowShrinking::No);
	}

	const float HalfWidth = BrushState.Width * 0.5f;
	BrushState.Points[VertexIndex] = SurfacePoint - Side * HalfWidth;
	BrushState.Points[VertexIndex + 1] = SurfacePoint + Side * HalfWidth;
	BrushState.Normals[VertexIndex] = Hit.ImpactNormal;
	BrushState.Normals[VertexIndex + 1] = Hit.ImpactNormal;

	if (bStartingNewStroke)
	{
		AddStrokeConnection();
		BrushState.AccumulatedDistance = 0.0f;
	}
	else
	{
		const int32 A = VertexIndex - 2;
		const int32 B = A + 1;
		const int32 C = VertexIndex;
		const int32 D = C + 1;
		BrushState.Triangles.Add(A);
		BrushState.Triangles.Add(B);
		BrushState.Triangles.Add(C);
		BrushState.Triangles.Add(C);
		BrushState.Triangles.Add(B);
		BrushState.Triangles.Add(D);
		BrushState.AccumulatedDistance += FVector::Distance(BrushState.Points[A], SurfacePoint);
	}

	// U advances with traveled distance so the texture does not stretch on fast swings.
	const float U = 0.5f + (BrushState.AccumulatedDistance / FMath::Max(BrushState.Width, 1.0f)) * BrushState.UVScaleU;
	BrushState.UVs[VertexIndex] = FVector2D(U, 0.0f);
	BrushState.UVs[VertexIndex + 1] = FVector2D(U, BrushState.UVScaleV);
	++BrushState.PointCount;

	TArray<FColor> Colors;
	Colors.Init(FColor::White, BrushState.Points.Num());
	TArray<FProcMeshTangent> Tangents;

	// UpdateMeshSection rejects a changed vertex count, so rebuild the section after each new sample.
	Brush->ClearMeshSection(0);
	Brush->CreateMeshSection(0, BrushState.Points, BrushState.Triangles, BrushState.Normals, BrushState.UVs, Colors, Tangents, false);

	if (bStartingNewStroke)
	{
		UE_LOG(LogTemp, Log, TEXT("BloodBrush: punto %d en %s superficie=%s"),
			BrushState.PointCount,
			*SurfacePoint.ToString(),
			Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("None"));
	}
}

void UBloodCombatVFXSubsystem::StopBloodBrush(UProceduralMeshComponent* Brush)
{
	if (IsValid(Brush))
	{
		// Geometry is intentionally persistent; only the next stroke boundary is marked.
		BrushState.bNeedsNewStrip = true;
	}
}

UNiagaraSystem* UBloodCombatVFXSubsystem::ResolveImpactSystem(ECombatImpactType Type, EBloodSurfaceType SurfaceType) const
{
 return ConfigDataAsset ? ConfigDataAsset->GetImpactSystem(Type, SurfaceType) : nullptr;
}

UNiagaraSystem* UBloodCombatVFXSubsystem::ResolveBloodSystem(EBloodEffectType Type) const
{
 return ConfigDataAsset ? ConfigDataAsset->GetBloodSystem(Type) : nullptr;
}

void UBloodCombatVFXSubsystem::ApplyParams(UNiagaraComponent* Component, const FBloodCombatVFXParams& Params) const
{
	if (!Component)
	{
		return;
	}

	Component->SetWorldScale3D(FVector(Params.Scale));

	for (const TPair<FName, float>& Pair : Params.FloatParameters)
	{
		Component->SetVariableFloat(Pair.Key, Pair.Value);
	}

	for (const TPair<FName, FVector>& Pair : Params.VectorParameters)
	{
		Component->SetVariableVec3(Pair.Key, Pair.Value);
	}

	for (const TPair<FName, FLinearColor>& Pair : Params.ColorParameters)
	{
		Component->SetVariableLinearColor(Pair.Key, Pair.Value);
	}
}

UNiagaraComponent* UBloodCombatVFXSubsystem::AcquireComponent(UNiagaraSystem* System)
{
	if (!System || !GetWorld())
	{
		return nullptr;
	}

	// Pools are keyed by Niagara system so a component never has to swap assets on reuse.
	TArray<TWeakObjectPtr<UNiagaraComponent>>& Pool = AvailableComponents.FindOrAdd(System);
	while (Pool.Num() > 0)
	{
		UNiagaraComponent* Component = Pool.Pop(EAllowShrinking::No).Get();
		if (IsValid(Component))
		{
			ActiveComponentSystems.Add(Component, System);
			return Component;
		}
	}

	// No reusable component: create one, wire the finish delegate and keep it alive for the world lifetime.
	UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(GetWorld());
	if (!NewComponent)
	{
		return nullptr;
	}

	NewComponent->SetAsset(System);
	NewComponent->bAutoActivate = false;
	NewComponent->RegisterComponentWithWorld(GetWorld());
	NewComponent->OnSystemFinished.AddDynamic(this, &UBloodCombatVFXSubsystem::OnPooledSystemFinished);
	AllComponents.Add(NewComponent);

	ActiveComponentSystems.Add(NewComponent, System);
	return NewComponent;
}

void UBloodCombatVFXSubsystem::ReleaseComponent(UNiagaraComponent* Component)
{
	if (!IsValid(Component))
	{
		ActiveComponentSystems.Remove(Component);
		return;
	}

	TObjectPtr<UNiagaraSystem>* SystemPtr = ActiveComponentSystems.Find(Component);
	if (!SystemPtr || !SystemPtr->Get())
	{
		ActiveComponentSystems.Remove(Component);
		return;
	}

	// Always detach the decal handler before deactivation so it can be reused by the next burst.
	ReleaseDecalHandler(Component);
	Component->DeactivateImmediate();
	ActiveComponentSystems.Remove(Component);
	AvailableComponents.FindOrAdd(SystemPtr->Get()).Add(Component);
}

void UBloodCombatVFXSubsystem::OnPooledSystemFinished(UNiagaraComponent* FinishedComponent)
{
	ReleaseComponent(FinishedComponent);
}
