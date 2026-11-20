#include "Data/VfxBloodCombatConfigDataAsset.h"

#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"

namespace
{
	UNiagaraSystem* GetImpactMappedSystem(const TArray<FVfxImpactMapping>& Mappings, ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType, TSoftObjectPtr<UNiagaraSystem> FVfxImpactMapping::* Member)
	{
		for (const FVfxImpactMapping& Mapping : Mappings)
		{
			if (Mapping.ImpactType == ImpactType && Mapping.SurfaceType == SurfaceType)
			{
				return (Mapping.*Member).LoadSynchronous();
			}
		}

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
