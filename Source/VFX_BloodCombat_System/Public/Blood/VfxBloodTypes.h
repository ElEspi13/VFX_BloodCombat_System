#pragma once

#include "CoreMinimal.h"
#include "VfxBloodTypes.generated.h"

UENUM(BlueprintType)
enum class EBloodEffectType : uint8
{
	None UMETA(DisplayName = "None"),
	Spray UMETA(DisplayName = "Spray"),
	Burst UMETA(DisplayName = "Burst"),
	Droplets UMETA(DisplayName = "Droplets"),
	Decal UMETA(DisplayName = "Decal"),
	Trail UMETA(DisplayName = "Trail"),
	SwordBloodTrail UMETA(DisplayName = "Sword Blood Trail")
};

USTRUCT(BlueprintType)
struct FBloodDecalParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	FVector DecalSize = FVector(32.0f, 64.0f, 64.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float LifeSpan = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float FadeScreenSize = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float FadeStartDelay = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float FadeDuration = 5.0f;
};
