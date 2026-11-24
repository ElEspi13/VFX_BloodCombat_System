#include "Data/VfxBloodCombatConfigDataAsset.h"

#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"

namespace
{
	UNiagaraSystem* GetImpactMappedSystem(const TArray<FVfxImpactMapping>& Mappings, ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType, TSoftObjectPtr<UNiagaraSystem> FVfxImpactMapping::* Member)
	{
		// Prefer the exact surface row; the member pointer lets main, spark and flash share one lookup.
		for (const FVfxImpactMapping& Mapping : Mappings)
		{
			if (Mapping.ImpactType == ImpactType && Mapping.SurfaceType == SurfaceType)
			{
				return (Mapping.*Member).LoadSynchronous();
			}
		}

		// Fall back to a default-surface row for the same combat context.
		for (const FVfxImpactMapping& Mapping : Mappings)
		{
			if (Mapping.ImpactType == ImpactType && Mapping.SurfaceType == EBloodSurfaceType::Default)
			{
				return (Mapping.*Member).LoadSynchronous();
			}
		}

		return nullptr;
	}
}

UNiagaraSystem* UVfxBloodCombatConfigDataAsset::GetImpactSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const
{
 return GetImpactMappedSystem(ImpactMappings, ImpactType, SurfaceType, &FVfxImpactMapping::NiagaraSystem);
}

UNiagaraSystem* UVfxBloodCombatConfigDataAsset::GetImpactSparkSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const
{
	return GetImpactMappedSystem(ImpactMappings, ImpactType, SurfaceType, &FVfxImpactMapping::SparkSystem);
}

UNiagaraSystem* UVfxBloodCombatConfigDataAsset::GetImpactFlashSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const
{
	return GetImpactMappedSystem(ImpactMappings, ImpactType, SurfaceType, &FVfxImpactMapping::FlashSystem);
}

UMaterialInterface* UVfxBloodCombatConfigDataAsset::GetBloodDecalMaterial() const
{
	for (const FVfxBloodMapping& Mapping : BloodMappings)
	{
     if (Mapping.BloodType == EBloodEffectType::Decal)
		{
			return Mapping.DecalMaterial.LoadSynchronous();
		}
	}

	return nullptr;
}

UMaterialInterface* UVfxBloodCombatConfigDataAsset::GetBloodDecalMaterialForEffect(EBloodEffectType BloodType) const
{
	for (const FVfxBloodMapping& Mapping : BloodMappings)
	{
		if (Mapping.BloodType == BloodType)
		{
			return Mapping.DecalMaterial.LoadSynchronous();
		}
	}

	return nullptr;
}

UMaterialInterface* UVfxBloodCombatConfigDataAsset::GetBrushMaterial() const
{
	return BrushMaterial.LoadSynchronous();
}

float UVfxBloodCombatConfigDataAsset::GetBrushWidth() const
{
	// Clamp at the getter as well as in metadata so data migrated from older assets stays valid.
	return FMath::Max(BrushWidth, 1.0f);
}

float UVfxBloodCombatConfigDataAsset::GetBrushUVScaleU() const
{
	return FMath::Max(BrushUVScaleU, 0.01f);
}

float UVfxBloodCombatConfigDataAsset::GetBrushUVScaleV() const
{
	return FMath::Max(BrushUVScaleV, 0.01f);
}

UNiagaraSystem* UVfxBloodCombatConfigDataAsset::GetBloodSystem(EBloodEffectType BloodType) const
{
	for (const FVfxBloodMapping& Mapping : BloodMappings)
	{
       if (Mapping.BloodType == BloodType)
		{
			return Mapping.NiagaraSystem.LoadSynchronous();
		}
	}

	return nullptr;
}
