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
class UVfxBloodCombatConfigDataAsset;
class AActor;

UCLASS()
class VFX_BLOODCOMBAT_SYSTEM_API UBloodCombatVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
   virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnSystem(UNiagaraSystem* System, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnImpact(ECombatImpactType Type, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void SpawnImpactEffects(ECombatImpactType Type, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnBlood(EBloodEffectType Type, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UDecalComponent* SpawnBloodDecal(const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void SetConfigDataAsset(UVfxBloodCombatConfigDataAsset* NewConfigDataAsset);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatExecute")
	void SpawnBloodBurst(const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatExecute")
	void SpawnBloodSpray(const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatExecute")
	void SpawnBloodTrail(const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* StartBloodTrail(const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void UpdateBloodTrail(UNiagaraComponent* TrailComponent, const FVector& WorldPosition);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void StopBloodTrail(UNiagaraComponent* TrailComponent);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* StartSwordBloodTrail(
		const FBloodCombatVFXParams& Params,
		USkeletalMeshComponent* SwordMesh,
		const TArray<FName>& FilteredSockets);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	void StopSwordBloodTrail(UNiagaraComponent* TrailComponent);

protected:
	virtual UNiagaraSystem* ResolveImpactSystem(ECombatImpactType Type, EBloodSurfaceType SurfaceType) const;
 virtual UNiagaraSystem* ResolveBloodSystem(EBloodEffectType Type) const;

	void ApplyParams(UNiagaraComponent* Component, const FBloodCombatVFXParams& Params) const;

private:
	UNiagaraComponent* SpawnSystemInternal(
		UNiagaraSystem* System,
		const FBloodCombatVFXParams& Params,
		TFunction<void(UNiagaraComponent*)> SetupComponent);

	UNiagaraComponent* StartTrail(
		EBloodEffectType Type,
		const FBloodCombatVFXParams& Params,
		TFunction<void(UNiagaraComponent*)> SetupComponent = nullptr);

	UNiagaraComponent* AcquireComponent(UNiagaraSystem* System);
	void ReleaseComponent(UNiagaraComponent* Component);

	AActor* AcquireDecalHandler();
	void SetupDecalHandler(UNiagaraComponent* Component, EBloodEffectType Type, const FBloodCombatVFXParams& Params);
	void ReleaseDecalHandler(UNiagaraComponent* Component);
	void SetObjectProperty(AActor* Actor, FName PropertyName, UObject* Value);
	void SetVectorProperty(AActor* Actor, FName PropertyName, const FVector& Value);

	UFUNCTION()
	void OnPooledSystemFinished(UNiagaraComponent* FinishedComponent);

	UPROPERTY()
	TObjectPtr<UVfxBloodCombatConfigDataAsset> ConfigDataAsset;

	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> AllComponents;

	TMap<TObjectPtr<UNiagaraSystem>, TArray<TWeakObjectPtr<UNiagaraComponent>>> AvailableComponents;
	TMap<TWeakObjectPtr<UNiagaraComponent>, TObjectPtr<UNiagaraSystem>> ActiveComponentSystems;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> AllDecalHandlers;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> AvailableDecalHandlers;

	TMap<UNiagaraComponent*, AActor*> ActiveDecalHandlers;
};
