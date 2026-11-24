#pragma once

#include "CoreMinimal.h"
#include "CombatImpactTypes.generated.h"

/** Combat context used to resolve impact Niagara systems from the configuration data asset. */
UENUM(BlueprintType)
enum class ECombatImpactType : uint8
{
	None UMETA(DisplayName = "None"),
	/** Light blade contact; default fallback for sword hits. */
	Sword UMETA(DisplayName = "Sword"),
	/** Slow, high-mass blade contact. */
	HeavySword UMETA(DisplayName = "Heavy Sword"),
	/** Damage multiplier case that should read as a stronger hit. */
	Critical UMETA(DisplayName = "Critical"),
	/** Non-bladed impact such as a shield or blunt weapon. */
	Blunt UMETA(DisplayName = "Blunt")
};
