#pragma once

#include "CoreMinimal.h"
#include "VfxBloodTypes.generated.h"

/** Identifies a blood effect entry in the configuration data asset. */
UENUM(BlueprintType)
enum class EBloodEffectType : uint8
{
	None UMETA(DisplayName = "None"),
	/** Directional spray emitted from the impact point. */
	Spray UMETA(DisplayName = "Spray"),
	/** Short radial emission at the impact point. */
	Burst UMETA(DisplayName = "Burst"),
	/** Falling droplets spawned above the impact point. */
	Droplets UMETA(DisplayName = "Droplets"),
	/** Decal spawned on the hit surface. */
	Decal UMETA(DisplayName = "Decal"),
	/** Continuous trail driven by User.TrailPosition. */
	Trail UMETA(DisplayName = "Trail"),
	/** Ribbon trail bound to filtered sockets on a skeletal mesh. */
	SwordBloodTrail UMETA(DisplayName = "Sword Blood Trail")
};

/** Decal tuning values used by the BP_DecalComponent handler. */
USTRUCT(BlueprintType)
struct FBloodDecalParams
{
	GENERATED_BODY()

	/** Projected decal dimensions in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	FVector DecalSize = FVector(32.0f, 64.0f, 64.0f);

	/** Total time before the decal component is destroyed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float LifeSpan = 10.0f;

	/** Screen size at which the decal starts fading out. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float FadeScreenSize = 0.01f;

	/** Delay before fade-out begins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float FadeStartDelay = 5.0f;

	/** Duration of the fade-out once FadeStartDelay has elapsed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Decal")
	float FadeDuration = 5.0f;
};
