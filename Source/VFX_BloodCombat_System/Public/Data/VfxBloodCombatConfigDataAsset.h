#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Blood/VfxBloodTypes.h"
#include "Combat/CombatImpactTypes.h"
#include "Combat/VfxCombatTypes.h"
#include "VfxBloodCombatConfigDataAsset.generated.h"

class UNiagaraSystem;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct FVfxImpactMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	ECombatImpactType ImpactType = ECombatImpactType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	EBloodSurfaceType SurfaceType = EBloodSurfaceType::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	TSoftObjectPtr<UNiagaraSystem> SparkSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	TSoftObjectPtr<UNiagaraSystem> FlashSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	TSoftObjectPtr<UMaterialInterface> DecalMaterial;
};

USTRUCT(BlueprintType)
struct FVfxBloodMapping
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	EBloodEffectType BloodType = EBloodEffectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Decal")
	TSoftObjectPtr<UMaterialInterface> DecalMaterial;
};

UCLASS(BlueprintType)
class VFX_BLOODCOMBAT_SYSTEM_API UVfxBloodCombatConfigDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Impacts")
	TArray<FVfxImpactMapping> ImpactMappings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blood")
	TArray<FVfxBloodMapping> BloodMappings;

	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UNiagaraSystem* GetImpactSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const;

	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UNiagaraSystem* GetImpactSparkSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const;

	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UNiagaraSystem* GetImpactFlashSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const;

	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
    UNiagaraSystem* GetBloodSystem(EBloodEffectType BloodType) const;

	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
  UMaterialInterface* GetBloodDecalMaterial() const;

	UFUNCTION(BlueprintCallable, Category = "VFX|Config")
	UMaterialInterface* GetBloodDecalMaterialForEffect(EBloodEffectType BloodType) const;
};
