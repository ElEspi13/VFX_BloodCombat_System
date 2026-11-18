
#include "Core/BloodCombatVFXSubsystem.h"
#include "Data/VfxBloodCombatConfigDataAsset.h"
#include "Components/DecalComponent.h"
#include "Components/PrimitiveComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"

void UBloodCombatVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	static const TCHAR* ConfigAssetPath = TEXT("/VFX_BloodCombat_System/Data/VFX_DataAsset.VFX_DataAsset");
	ConfigDataAsset = LoadObject<UVfxBloodCombatConfigDataAsset>(nullptr, ConfigAssetPath);

	if (!ConfigDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("BloodCombatVFX: no se pudo cargar el Data Asset en %s"), ConfigAssetPath);
	}
}

void UBloodCombatVFXSubsystem::Deinitialize()
{
	for (auto& Pair : AvailableComponents)
	{
		for (UNiagaraComponent* Component : Pair.Value)
		{
			if (Component)
			{
				Component->OnSystemFinished.RemoveAll(this);
				Component->DeactivateImmediate();
				Component->DestroyComponent();
			}
		}
	}
	AvailableComponents.Empty();

	for (auto& Pair : ActiveComponentSystems)
	{
		if (UNiagaraComponent* Component = Pair.Key)
		{
			Component->OnSystemFinished.RemoveAll(this);
		}
	}
	ActiveComponentSystems.Empty();

	Super::Deinitialize();
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnSystem(UNiagaraSystem* System, const FBloodCombatVFXParams& Params)
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

	const FRotator SpawnRotation = Params.Rotation.IsNearlyZero() ? Params.Normal.Rotation() : Params.Rotation;
	Component->SetWorldLocationAndRotation(Params.Location, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);
	Component->SetWorldScale3D(FVector(Params.Scale));

	ApplyParams(Component, Params);
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

	return SpawnSystem(System, Params);
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

	TArray<TObjectPtr<UNiagaraComponent>>& Pool = AvailableComponents.FindOrAdd(System);
	while (Pool.Num() > 0)
	{
		UNiagaraComponent* Component = Pool.Pop(EAllowShrinking::No);
		if (IsValid(Component))
		{
			ActiveComponentSystems.Add(Component, System);
			return Component;
		}
	}

	UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(GetWorld());
	if (!NewComponent)
	{
		return nullptr;
	}

	NewComponent->SetAsset(System);
	NewComponent->bAutoActivate = false;
	NewComponent->RegisterComponentWithWorld(GetWorld());
	NewComponent->OnSystemFinished.AddDynamic(this, &UBloodCombatVFXSubsystem::OnPooledSystemFinished);

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

	Component->DeactivateImmediate();
	ActiveComponentSystems.Remove(Component);
	AvailableComponents.FindOrAdd(SystemPtr->Get()).Add(Component);
}

void UBloodCombatVFXSubsystem::OnPooledSystemFinished(UNiagaraComponent* FinishedComponent)
{
	ReleaseComponent(FinishedComponent);
}
