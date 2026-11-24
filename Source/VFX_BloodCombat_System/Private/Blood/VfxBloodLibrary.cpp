#include "Blood/VfxBloodLibrary.h"
#include "Core/BloodCombatVFXSubsystem.h"
#include "Components/DecalComponent.h"
#include "NiagaraComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UActorComponent* UVfxBloodLibrary::SpawnBloodEffect(const UObject* WorldContextObject, EBloodEffectType EffectType, const FBloodCombatVFXParams& Params)
{
    if (!WorldContextObject || !GEngine)
    {
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    UBloodCombatVFXSubsystem* Subsystem = World ? World->GetSubsystem<UBloodCombatVFXSubsystem>() : nullptr;
    if (!Subsystem)
    {
        return nullptr;
    }

    // Decals are components but not Niagara systems, so they take a separate subsystem path.
    if (EffectType == EBloodEffectType::Decal)
    {
        return Subsystem->SpawnBloodDecal(Params);
    }

    return Subsystem->SpawnBlood(EffectType, Params);
}

void UVfxBloodLibrary::SpawnBloodEffects(const UObject* WorldContextObject, const TArray<EBloodEffectType>& EffectTypes, const FBloodCombatVFXParams& Params)
{
    for (EBloodEffectType EffectType : EffectTypes)
    {
        SpawnBloodEffect(WorldContextObject, EffectType, Params);
    }
}
