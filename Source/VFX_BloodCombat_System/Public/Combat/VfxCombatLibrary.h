#pragma once

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Combat/CombatImpactTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "VfxCombatLibrary.generated.h"

class UNiagaraComponent;

UCLASS()
class VFX_BLOODCOMBAT_SYSTEM_API UVfxCombatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static UNiagaraComponent* SpawnImpactEffect(const UObject* WorldContextObject, ECombatImpactType ImpactType, const FBloodCombatVFXParams& Params);

	UFUNCTION(BlueprintCallable, Category = "BloodCombatVFX", meta = (WorldContext = "WorldContextObject"))
	static void SpawnImpactEffects(const UObject* WorldContextObject, ECombatImpactType ImpactType, const FBloodCombatVFXParams& Params);
};
