#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blood/VfxBloodTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "VfxBloodLibrary.generated.h"

class UActorComponent;

UCLASS()
class VFX_BLOODCOMBAT_SYSTEM_API UVfxBloodLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static UActorComponent* SpawnBloodEffect(const UObject* WorldContextObject, EBloodEffectType EffectType, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static void SpawnBloodEffects(const UObject* WorldContextObject, const TArray<EBloodEffectType>& EffectTypes, const FBloodCombatVFXParams& Params);
};
