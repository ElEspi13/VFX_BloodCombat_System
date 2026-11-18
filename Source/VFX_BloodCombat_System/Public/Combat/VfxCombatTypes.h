#pragma once

#include "CoreMinimal.h"
#include "VfxCombatTypes.generated.h"

class UPrimitiveComponent;

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

USTRUCT(BlueprintType)
struct FBloodCombatVFXParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Normal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Intensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	EBloodSurfaceType SurfaceType = EBloodSurfaceType::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TWeakObjectPtr<AActor> Instigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	FVector DecalSize = FVector(16.0f, 16.0f, 16.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal",
		meta = (ClampMin = 0.0, Units = "s"))
	float DecalLifeSpan = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|UserParameters")
	TMap<FName, float> FloatParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|UserParameters")
	TMap<FName, FVector> VectorParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|UserParameters")
	TMap<FName, FLinearColor> ColorParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector StartPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Direction = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Length = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Duration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Width = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	int32 RandomSeed = INDEX_NONE;
};
