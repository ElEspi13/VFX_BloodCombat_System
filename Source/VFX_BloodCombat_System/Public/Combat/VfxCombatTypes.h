#pragma once

#include "CoreMinimal.h"
#include "VfxCombatTypes.generated.h"

class UPrimitiveComponent;

/** Surface category used to select material-specific blood systems and decals. */
UENUM(BlueprintType)
enum class EBloodSurfaceType : uint8
{
	Default UMETA(DisplayName = "Default"),
	Flesh UMETA(DisplayName = "Flesh"),
	Metal UMETA(DisplayName = "Metal"),
	Stone UMETA(DisplayName = "Stone"),
	Wood UMETA(DisplayName = "Wood"),
	Fabric UMETA(DisplayName = "Fabric")
};

/** Shared parameter block for every blood and impact effect in the plugin. */
USTRUCT(BlueprintType)
struct FBloodCombatVFXParams
{
	GENERATED_BODY()

	/** World-space spawn location, or initial trail position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Location = FVector::ZeroVector;

	/** Explicit spawn rotation. When zero, Normal.Rotation() is used instead. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Surface normal at the impact point; also drives the fallback rotation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Normal = FVector::UpVector;

	/** Uniform world scale applied to the spawned Niagara component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Scale = 1.0f;

	/** Generic intensity hint forwarded to Niagara user parameters when wired in the asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Intensity = 1.0f;

	/** Surface category used for system resolution. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	EBloodSurfaceType SurfaceType = EBloodSurfaceType::Default;

	/** Actor that caused the effect; excluded from brush traces to avoid self-hits. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TWeakObjectPtr<AActor> Instigator;

	/** Component hit by the attack; enables attached decals and brush trace exclusions. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;

	/** Projected decal size in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	FVector DecalSize = FVector(16.0f, 16.0f, 16.0f);

	/** Lifetime of a spawned decal. The persistent brush ignores this value. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal",
		meta = (ClampMin = 0.0, Units = "s"))
	float DecalLifeSpan = 10.0f;

	/** Niagara user parameters forwarded as float, in addition to the built-in fields. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|UserParameters")
	TMap<FName, float> FloatParameters;

	/** Niagara user parameters forwarded as vector, e.g. User.Direction and User.StartPosition. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|UserParameters")
	TMap<FName, FVector> VectorParameters;

	/** Niagara user parameters forwarded as linear color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|UserParameters")
	TMap<FName, FLinearColor> ColorParameters;

	/** Start of a swept effect such as a spray or ribbon trail. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector StartPosition = FVector::ZeroVector;

	/** Direction of a swept effect; also used as the brush projection fallback. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Direction = FVector::ZeroVector;

	/** Length of a swept effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Length = 50.0f;

	/** Duration of a swept effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Duration = 0.35f;

	/** Width of a swept effect. Brush width is configured in UVfxBloodCombatConfigDataAsset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Width = 10.0f;

	/** Optional Niagara random seed; INDEX_NONE leaves the system default. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	int32 RandomSeed = INDEX_NONE;
};
