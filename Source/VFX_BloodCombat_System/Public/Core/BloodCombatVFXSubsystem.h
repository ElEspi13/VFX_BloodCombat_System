#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Combat/CombatImpactTypes.h"
#include "Blood/VfxBloodTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "BloodCombatVFXSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UDecalComponent;
class UMaterialInterface;
class UVfxBloodCombatConfigDataAsset;

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
	

protected:
	virtual UNiagaraSystem* ResolveImpactSystem(ECombatImpactType Type, EBloodSurfaceType SurfaceType) const;
 virtual UNiagaraSystem* ResolveBloodSystem(EBloodEffectType Type) const;

	void ApplyParams(UNiagaraComponent* Component, const FBloodCombatVFXParams& Params) const;

private:
	UNiagaraComponent* AcquireComponent(UNiagaraSystem* System);
	void ReleaseComponent(UNiagaraComponent* Component);

	UFUNCTION()
	void OnPooledSystemFinished(UNiagaraComponent* FinishedComponent);

	UPROPERTY()
	TObjectPtr<UVfxBloodCombatConfigDataAsset> ConfigDataAsset;

	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> AllComponents;

	TMap<TObjectPtr<UNiagaraSystem>, TArray<TWeakObjectPtr<UNiagaraComponent>>> AvailableComponents;
	TMap<TWeakObjectPtr<UNiagaraComponent>, TObjectPtr<UNiagaraSystem>> ActiveComponentSystems;
};
