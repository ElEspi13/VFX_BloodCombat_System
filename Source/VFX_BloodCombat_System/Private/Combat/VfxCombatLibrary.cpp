#include "Combat/VfxCombatLibrary.h"
#include "Core/BloodCombatVFXSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UNiagaraComponent* UVfxCombatLibrary::SpawnImpactEffect(const UObject* WorldContextObject, ECombatImpactType ImpactType, const FBloodCombatVFXParams& Params)
{
	if (!WorldContextObject || !GEngine)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UBloodCombatVFXSubsystem* Subsystem = World ? World->GetSubsystem<UBloodCombatVFXSubsystem>() : nullptr;
	return Subsystem ? Subsystem->SpawnImpact(ImpactType, Params) : nullptr;
}

void UVfxCombatLibrary::SpawnImpactEffects(const UObject* WorldContextObject, ECombatImpactType ImpactType, const FBloodCombatVFXParams& Params)
{
	if (!WorldContextObject || !GEngine)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	UBloodCombatVFXSubsystem* Subsystem = World ? World->GetSubsystem<UBloodCombatVFXSubsystem>() : nullptr;
	if (Subsystem)
	{
		Subsystem->SpawnImpactEffects(ImpactType, Params);
	}
}
