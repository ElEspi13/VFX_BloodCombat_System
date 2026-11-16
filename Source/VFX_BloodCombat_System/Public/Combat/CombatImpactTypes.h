#pragma once

#include "CoreMinimal.h"
#include "CombatImpactTypes.generated.h"

UENUM(BlueprintType)
enum class ECombatImpactType : uint8
{
	None UMETA(DisplayName = "None"),
	Sword UMETA(DisplayName = "Sword"),
	HeavySword UMETA(DisplayName = "Heavy Sword"),
	Critical UMETA(DisplayName = "Critical"),
	Blunt UMETA(DisplayName = "Blunt")
};
