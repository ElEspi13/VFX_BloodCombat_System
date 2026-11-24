#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Combat/CombatImpactTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "VfxCombatLibrary.generated.h"

class UNiagaraComponent;

/** Blueprint-friendly entry points for combat impact VFX. */
UCLASS()
class VFX_BLOODCOMBAT_SYSTEM_API UVfxCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Spawns the main impact Niagara system for the given combat context.
	 * Returns the spawned component, or nullptr when the subsystem or mapped system is unavailable.
	 */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static UNiagaraComponent* SpawnImpactEffect(const UObject* WorldContextObject, ECombatImpactType ImpactType, const FBloodCombatVFXParams& Params);

	/** Spawns the optional spark and flash systems associated with the impact context. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static void SpawnImpactEffects(const UObject* WorldContextObject, ECombatImpactType ImpactType, const FBloodCombatVFXParams& Params);
};
