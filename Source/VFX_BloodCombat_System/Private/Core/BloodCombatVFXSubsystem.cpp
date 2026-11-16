
#include "Core/BloodCombatVFXSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

void UBloodCombatVFXSubsystem::Deinitialize()
{
	for (auto& Pair : AvailableComponents)
	{
		for (UNiagaraComponent* Component : Pair.Value)
		{
			if (Component)
			{
				Component->OnSystemFinished.RemoveAll(this);
				Component->DeactivateImmediate();
				Component->DestroyComponent();
			}
		}
	}
	AvailableComponents.Empty();

	for (auto& Pair : ActiveComponentSystems)
	{
		if (UNiagaraComponent* Component = Pair.Key)
		{
			Component->OnSystemFinished.RemoveAll(this);
		}
	}
	ActiveComponentSystems.Empty();

	Super::Deinitialize();
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnSystem(UNiagaraSystem* System, const FBloodCombatVFXParams& Params)
{
	if (!System || !GetWorld())
	{
		return nullptr;
	}

	UNiagaraComponent* Component = AcquireComponent(System);
	if (!Component)
	{
		return nullptr;
	}

	const FRotator SpawnRotation = Params.Rotation.IsNearlyZero() ? Params.Normal.Rotation() : Params.Rotation;
	Component->SetWorldLocationAndRotation(Params.Location, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);
	Component->SetWorldScale3D(FVector(Params.Scale));

	ApplyParams(Component, Params);
	Component->Activate(true);

	return Component;
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnImpact(ECombatImpactType Type, const FBloodCombatVFXParams& Params)
{
	UNiagaraSystem* System = ResolveImpactSystem(Type, Params.SurfaceType);
	if (!System)
	{
		return nullptr;
	}

	return SpawnSystem(System, Params);
}

UNiagaraComponent* UBloodCombatVFXSubsystem::SpawnBlood(EBloodEffectType Type, const FBloodCombatVFXParams& Params)
{
	UNiagaraSystem* System = ResolveBloodSystem(Type, Params.SurfaceType);
	if (!System)
	{
		return nullptr;
	}

	return SpawnSystem(System, Params);
}

UNiagaraSystem* UBloodCombatVFXSubsystem::ResolveImpactSystem(ECombatImpactType Type, EBloodSurfaceType SurfaceType) const
{
	return nullptr;
}

UNiagaraSystem* UBloodCombatVFXSubsystem::ResolveBloodSystem(EBloodEffectType Type, EBloodSurfaceType SurfaceType) const
{
	return nullptr;
}

void UBloodCombatVFXSubsystem::ApplyParams(UNiagaraComponent* Component, const FBloodCombatVFXParams& Params) const
{
	if (!Component)
	{
		return;
	}

	Component->SetWorldScale3D(FVector(Params.Scale));

	for (const TPair<FName, float>& Pair : Params.FloatParameters)
	{
		Component->SetVariableFloat(Pair.Key, Pair.Value);
	}

	for (const TPair<FName, FVector>& Pair : Params.VectorParameters)
	{
		Component->SetVariableVec3(Pair.Key, Pair.Value);
	}

	for (const TPair<FName, FLinearColor>& Pair : Params.ColorParameters)
	{
		Component->SetVariableLinearColor(Pair.Key, Pair.Value);
	}
}

UNiagaraComponent* UBloodCombatVFXSubsystem::AcquireComponent(UNiagaraSystem* System)
{
	if (!System || !GetWorld())
	{
		return nullptr;
	}

	TArray<TObjectPtr<UNiagaraComponent>>& Pool = AvailableComponents.FindOrAdd(System);
	while (Pool.Num() > 0)
	{
		UNiagaraComponent* Component = Pool.Pop(EAllowShrinking::No);
		if (IsValid(Component))
		{
			ActiveComponentSystems.Add(Component, System);
			return Component;
		}
	}

	UNiagaraComponent* NewComponent = NewObject<UNiagaraComponent>(GetWorld());
	if (!NewComponent)
	{
		return nullptr;
	}

	NewComponent->SetAsset(System);
	NewComponent->bAutoActivate = false;
	NewComponent->RegisterComponentWithWorld(GetWorld());
	NewComponent->OnSystemFinished.AddDynamic(this, &UBloodCombatVFXSubsystem::OnPooledSystemFinished);

	ActiveComponentSystems.Add(NewComponent, System);
	return NewComponent;
}

void UBloodCombatVFXSubsystem::ReleaseComponent(UNiagaraComponent* Component)
{
	if (!IsValid(Component))
	{
		ActiveComponentSystems.Remove(Component);
		return;
	}

	TObjectPtr<UNiagaraSystem>* SystemPtr = ActiveComponentSystems.Find(Component);
	if (!SystemPtr || !SystemPtr->Get())
	{
		ActiveComponentSystems.Remove(Component);
		return;
	}

	Component->DeactivateImmediate();
	ActiveComponentSystems.Remove(Component);
	AvailableComponents.FindOrAdd(SystemPtr->Get()).Add(Component);
}

void UBloodCombatVFXSubsystem::OnPooledSystemFinished(UNiagaraComponent* FinishedComponent)
{
	ReleaseComponent(FinishedComponent);
}
