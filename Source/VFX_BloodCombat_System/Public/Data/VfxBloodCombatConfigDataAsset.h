#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Blood/VfxBloodTypes.h"
#include "Combat/CombatImpactTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "VfxBloodCombatConfigDataAsset.generated.h"

class UNiagaraSystem;
class UMaterialInterface;

/** Maps a combat context and surface to the Niagara and decal assets used for an impact. */
USTRUCT(BlueprintType)
struct FVfxImpactMapping
{
	GENERATED_BODY()

	/** Combat context that selects this row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	ECombatImpactType ImpactType = ECombatImpactType::None;

	/** Surface category that selects this row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	EBloodSurfaceType SurfaceType = EBloodSurfaceType::Default;

	/** Main impact system spawned by SpawnImpact. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

	/** Optional spark layer spawned by SpawnImpactEffects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	TSoftObjectPtr<UNiagaraSystem> SparkSystem;

	/** Optional flash layer spawned by SpawnImpactEffects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	TSoftObjectPtr<UNiagaraSystem> FlashSystem;

	/** Decal material associated with this impact context. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	TSoftObjectPtr<UMaterialInterface> DecalMaterial;
};

/** Maps a blood effect type to its Niagara system and optional decal material. */
USTRUCT(BlueprintType)
struct FVfxBloodMapping
{
	GENERATED_BODY()

	/** Blood effect that selects this row. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	EBloodEffectType BloodType = EBloodEffectType::None;

	/** Niagara system spawned for this blood effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

	/** Decal material used when the effect needs a blood stain. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	TSoftObjectPtr<UMaterialInterface> DecalMaterial;
};

/** Central asset that maps combat contexts to VFX and stores procedural brush tuning. */
UCLASS(BlueprintType)
class VFX_BLOODCOMBAT_SYSTEM_API UVfxBloodCombatConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Rows consulted by the impact system resolvers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacts")
	TArray<FVfxImpactMapping> ImpactMappings;

	/** Rows consulted by the blood system resolvers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood")
	TArray<FVfxBloodMapping> BloodMappings;

	/** Material used by the procedural blood brush stroke. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood Brush")
	TSoftObjectPtr<UMaterialInterface> BrushMaterial;

	/** Full width of each brush ribbon in world units. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood Brush",
		meta = (ClampMin = "1.0", Units = "cm"))
	float BrushWidth = 20.0f;

	/** Number of texture repeats along the stroke direction. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood Brush",
		meta = (ClampMin = "0.01"))
	float BrushUVScaleU = 1.0f;

	/** Texture coverage across the ribbon width. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood Brush",
		meta = (ClampMin = "0.01"))
	float BrushUVScaleV = 1.0f;

	/** Resolves the main impact Niagara system for an impact type and surface. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UNiagaraSystem* GetImpactSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const;

	/** Resolves the optional spark layer for an impact type and surface. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UNiagaraSystem* GetImpactSparkSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const;

	/** Resolves the optional flash layer for an impact type and surface. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UNiagaraSystem* GetImpactFlashSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const;

	/** Resolves the Niagara system for a blood effect type. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
    UNiagaraSystem* GetBloodSystem(EBloodEffectType BloodType) const;

	/** Resolves the default blood decal material. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
  UMaterialInterface* GetBloodDecalMaterial() const;

	/** Resolves the decal material associated with a specific blood effect. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UMaterialInterface* GetBloodDecalMaterialForEffect(EBloodEffectType BloodType) const;

	/** Loads the material used by the persistent brush mesh. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UMaterialInterface* GetBrushMaterial() const;

	/** Returns the configured ribbon width. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	float GetBrushWidth() const;

	/** Returns the configured longitudinal UV scale. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	float GetBrushUVScaleU() const;

	/** Returns the configured transverse UV scale. */
	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	float GetBrushUVScaleV() const;
};
