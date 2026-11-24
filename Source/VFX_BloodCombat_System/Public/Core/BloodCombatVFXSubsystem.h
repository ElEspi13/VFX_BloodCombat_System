#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Combat/CombatImpactTypes.h"
#include "Blood/VfxBloodTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "BloodCombatVFXSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USkeletalMeshComponent;
class UDecalComponent;
class UMaterialInterface;
class UProceduralMeshComponent;
class UPrimitiveComponent;
class UVfxBloodCombatConfigDataAsset;
class AActor;

/** State of the single persistent procedural blood ribbon owned by the world subsystem. */
struct FBloodBrushState
{
	/** Paired ribbon vertices in world space: left and right edge per sampled point. */
	TArray<FVector> Points;
	/** Surface normal duplicated for both vertices of each sample. */
	TArray<FVector> Normals;
	/** UV pairs; U advances along the stroke and V spans the ribbon width. */
	TArray<FVector2D> UVs;
	/** Two triangles per segment between consecutive samples. */
	TArray<int32> Triangles;
	/** Attacking actor excluded from projection traces. */
	TWeakObjectPtr<AActor> Instigator;
	/** Hit component excluded from projection traces. */
	TWeakObjectPtr<UPrimitiveComponent> IgnoreComponent;
	/** Sword collision component excluded from projection traces. */
	TWeakObjectPtr<UPrimitiveComponent> IgnoreSwordComponent;
	/** Fallback projection direction when the blade delta is too small. */
	FVector ProjectionDirection = FVector::ForwardVector;
	/** Previous blade position used to derive the sweep direction. */
	FVector LastBladePosition = FVector::ZeroVector;
	/** Ribbon width copied from the config data asset. */
	float Width = 20.0f;
	/** Longitudinal texture repeat copied from the config data asset. */
	float UVScaleU = 1.0f;
	/** Transverse texture span copied from the config data asset. */
	float UVScaleV = 1.0f;
	/** Minimum distance between consecutive surface samples. */
	float MinPointDistance = 5.0f;
	/** Traveled distance used to advance the U coordinate. */
	float AccumulatedDistance = 0.0f;
	/** Number of sampled points; each point owns two vertices. */
	int32 PointCount = 0;
	/** Whether a valid previous blade position exists for direction calculation. */
	bool bHasLastBladePosition = false;
	/** True when the next sample must start a disconnected strip. */
	bool bNeedsNewStrip = true;
};

/** World subsystem that owns Niagara pooling, blood decals and the persistent procedural brush. */
UCLASS()
class VFX_BLOODCOMBAT_SYSTEM_API UBloodCombatVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Loads the shared config data asset used by every resolver. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/** Destroys pooled components, decal handlers and the persistent brush. */
	virtual void Deinitialize() override;

	/** Spawns an arbitrary Niagara system through the component pool. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnSystem(UNiagaraSystem* System, const FBloodCombatVFXParams& Params);

	/** Spawns the main impact system for a combat context and surface. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnImpact(ECombatImpactType Type, const FBloodCombatVFXParams& Params);

	/** Spawns the optional spark and flash layers for a combat context. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void SpawnImpactEffects(ECombatImpactType Type, const FBloodCombatVFXParams& Params);

	/** Spawns a blood Niagara system for the requested effect type. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnBlood(EBloodEffectType Type, const FBloodCombatVFXParams& Params);

	/** Spawns a decal attached to the hit component, or at the world impact point. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UDecalComponent* SpawnBloodDecal(const FBloodCombatVFXParams& Params);

	/** Overrides the config data asset used by this world instance. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void SetConfigDataAsset(UVfxBloodCombatConfigDataAsset* NewConfigDataAsset);

	/** Blueprint shortcut for a one-shot blood burst. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatExecute")
	void SpawnBloodBurst(const FBloodCombatVFXParams& Params);

	/** Blueprint shortcut for a one-shot directional spray. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatExecute")
	void SpawnBloodSpray(const FBloodCombatVFXParams& Params);

	/** Blueprint shortcut for a one-shot trail effect. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatExecute")
	void SpawnBloodTrail(const FBloodCombatVFXParams& Params);

	/** Starts a continuous trail driven by User.TrailPosition. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* StartBloodTrail(const FBloodCombatVFXParams& Params);

	/** Moves the active trail position to a new world location. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void UpdateBloodTrail(UNiagaraComponent* TrailComponent, const FVector& WorldPosition);

	/** Releases a trail component back into the pool. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void StopBloodTrail(UNiagaraComponent* TrailComponent);

	/** Starts a ribbon trail bound to filtered sockets on the sword mesh. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* StartSwordBloodTrail(
		const FBloodCombatVFXParams& Params,
		USkeletalMeshComponent* SwordMesh,
		const TArray<FName>& FilteredSockets);

	/** Releases a sword ribbon trail back into the pool. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void StopSwordBloodTrail(UNiagaraComponent* TrailComponent);

	/** Starts a persistent procedural brush stroke; geometry is kept until Deinitialize. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UProceduralMeshComponent* StartBloodBrush(
		const FBloodCombatVFXParams& Params,
		UPrimitiveComponent* IgnoreSwordComponent = nullptr);

	/** Projects the blade position onto a surface and appends a ribbon sample. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void UpdateBloodBrush(UProceduralMeshComponent* Brush, const FVector& WorldPosition);

	/** Marks the next brush sample as the start of a new disconnected strip. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void StopBloodBrush(UProceduralMeshComponent* Brush);

	/** Draws brush traces, sampled points and ribbon side vectors in the world. */
	UPROPERTY(EditAnywhere, Category = "BloodCombatVFX|Debug")
	bool bDebugBrush = true;

protected:
	/** Virtual so projects can override system selection without editing the config asset. */
	virtual UNiagaraSystem* ResolveImpactSystem(ECombatImpactType Type, EBloodSurfaceType SurfaceType) const;
	/** Virtual so projects can override blood system selection without editing the config asset. */
	virtual UNiagaraSystem* ResolveBloodSystem(EBloodEffectType Type) const;

	/** Forwards scale and generic user parameters to a pooled Niagara component. */
	void ApplyParams(UNiagaraComponent* Component, const FBloodCombatVFXParams& Params) const;

private:
	/** Common spawn path with an optional setup callback for trail and decal wiring. */
	UNiagaraComponent* SpawnSystemInternal(
		UNiagaraSystem* System,
		const FBloodCombatVFXParams& Params,
		TFunction<void(UNiagaraComponent*)> SetupComponent);

	/** Resolves a blood system and seeds User.TrailPosition before activation. */
	UNiagaraComponent* StartTrail(
		EBloodEffectType Type,
		const FBloodCombatVFXParams& Params,
		TFunction<void(UNiagaraComponent*)> SetupComponent = nullptr);

	/** Reuses an idle component for the given system, or creates and registers a new one. */
	UNiagaraComponent* AcquireComponent(UNiagaraSystem* System);
	/** Deactivates a component and returns it to the pool for its original system. */
	void ReleaseComponent(UNiagaraComponent* Component);

	/** Reuses a BP_DecalComponent handler, spawning one on demand. */
	AActor* AcquireDecalHandler();
	/** Pushes decal settings into the handler and links it to the Niagara callback variable. */
	void SetupDecalHandler(UNiagaraComponent* Component, EBloodEffectType Type, const FBloodCombatVFXParams& Params);
	/** Clears handler references and returns it to the available list. */
	void ReleaseDecalHandler(UNiagaraComponent* Component);
	/** Sets a reflected UObject property by name without a hard C++ dependency on the Blueprint class. */
	void SetObjectProperty(AActor* Actor, FName PropertyName, UObject* Value);
	/** Sets a reflected FVector property by name. */
	void SetVectorProperty(AActor* Actor, FName PropertyName, const FVector& Value);

	/** Lazily creates the single persistent brush mesh in world space. */
	UProceduralMeshComponent* EnsureBrushMesh(UMaterialInterface* Material);
	/** Prepares the brush state to start a new disconnected strip. */
	void AddStrokeConnection();

	/** Returns a component to its pool when its Niagara system finishes naturally. */
	UFUNCTION()
	void OnPooledSystemFinished(UNiagaraComponent* FinishedComponent);

	/** Config asset loaded at Initialize, or replaced through SetConfigDataAsset. */
	UPROPERTY()
	TObjectPtr<UVfxBloodCombatConfigDataAsset> ConfigDataAsset;

	/** Every pooled component, kept alive for the lifetime of the world. */
	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> AllComponents;

	/** Idle components grouped by the system they were created for. */
	TMap<TObjectPtr<UNiagaraSystem>, TArray<TWeakObjectPtr<UNiagaraComponent>>> AvailableComponents;
	/** Active component to system mapping used when returning a component to its pool. */
	TMap<TWeakObjectPtr<UNiagaraComponent>, TObjectPtr<UNiagaraSystem>> ActiveComponentSystems;

	/** Every spawned BP_DecalComponent handler. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AllDecalHandlers;

	/** Handlers currently free for reuse. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AvailableDecalHandlers;

	/** Niagara component currently driving each checked-out decal handler. */
	TMap<UNiagaraComponent*, AActor*> ActiveDecalHandlers;

	/** Single world-space mesh reused by every brush stroke. */
	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> PersistentBrushMesh;

	/** Vertex buffers and projection state for the persistent brush. */
	FBloodBrushState BrushState;
};
