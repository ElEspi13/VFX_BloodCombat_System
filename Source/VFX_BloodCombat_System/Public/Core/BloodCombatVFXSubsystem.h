#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Combat/CombatImpactTypes.h"
#include "Blood/VfxBloodTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "BloodCombatVFXSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class VFX_BLOODCOMBAT_SYSTEM_API UBloodCombatVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnSystem(UNiagaraSystem* System, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnImpact(ECombatImpactType Type, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX")
	UNiagaraComponent* SpawnBlood(EBloodEffectType Type, const FBloodCombatVFXParams& Params);

protected:
	virtual UNiagaraSystem* ResolveImpactSystem(ECombatImpactType Type, EBloodSurfaceType SurfaceType) const;
	virtual UNiagaraSystem* ResolveBloodSystem(EBloodEffectType Type, EBloodSurfaceType SurfaceType) const;

	void ApplyParams(UNiagaraComponent* Component, const FBloodCombatVFXParams& Params) const;

private:
	UNiagaraComponent* AcquireComponent(UNiagaraSystem* System);
	void ReleaseComponent(UNiagaraComponent* Component);

	UFUNCTION()
	void OnPooledSystemFinished(UNiagaraComponent* FinishedComponent);

	TMap<TObjectPtr<UNiagaraSystem>, TArray<TObjectPtr<UNiagaraComponent>>> AvailableComponents;
	TMap<TObjectPtr<UNiagaraComponent>, TObjectPtr<UNiagaraSystem>> ActiveComponentSystems;
};
