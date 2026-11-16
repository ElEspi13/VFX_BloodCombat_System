#include "Data/VfxBloodCombatConfigDataAsset.h"

#include "NiagaraSystem.h"

UNiagaraSystem* UVfxBloodCombatConfigDataAsset::GetImpactSystem(ECombatImpactType ImpactType, EBloodSurfaceType SurfaceType) const
{
	for (const FVfxImpactMapping& Mapping : ImpactMappings)
	{
		if (Mapping.ImpactType == ImpactType && Mapping.SurfaceType == SurfaceType)
		{
			return Mapping.NiagaraSystem.LoadSynchronous();
		}
	}

	for (const FVfxImpactMapping& Mapping : ImpactMappings)
	{
		if (Mapping.ImpactType == ImpactType && Mapping.SurfaceType == EBloodSurfaceType::Default)
		{
			return Mapping.NiagaraSystem.LoadSynchronous();
		}
	}

	return nullptr;
}

UNiagaraSystem* UVfxBloodCombatConfigDataAsset::GetBloodSystem(EBloodEffectType BloodType, EBloodSurfaceType SurfaceType) const
{
	for (const FVfxBloodMapping& Mapping : BloodMappings)
	{
		if (Mapping.BloodType == BloodType && Mapping.SurfaceType == SurfaceType)
		{
			return Mapping.NiagaraSystem.LoadSynchronous();
		}
	}

	for (const FVfxBloodMapping& Mapping : BloodMappings)
	{
		if (Mapping.BloodType == BloodType && Mapping.SurfaceType == EBloodSurfaceType::Default)
		{
			return Mapping.NiagaraSystem.LoadSynchronous();
		}
	}

	return nullptr;
}
