#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blood/VfxBloodTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "VfxBloodLibrary.generated.h"

class UActorComponent;

/** Blueprint-friendly entry points for spawning blood effects through the world subsystem. */
UCLASS()
class VFX_BLOODCOMBAT_SYSTEM_API UVfxBloodLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Spawns one blood effect. Decal effects are routed through the decal path;
	 * all other types resolve to a Niagara system in the config data asset.
	 * Returns the spawned component, or nullptr when the world/subsystem/asset is unavailable.
	 */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static UActorComponent* SpawnBloodEffect(const UObject* WorldContextObject, EBloodEffectType EffectType, const FBloodCombatVFXParams& Params);

	/** Convenience wrapper that spawns every requested effect with the same parameters. */
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static void SpawnBloodEffects(const UObject* WorldContextObject, const TArray<EBloodEffectType>& EffectTypes, const FBloodCombatVFXParams& Params);
};
