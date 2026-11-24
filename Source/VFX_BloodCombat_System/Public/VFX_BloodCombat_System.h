// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Runtime entry point for the blood combat VFX plugin.
 * Registers no gameplay logic itself: spawning and pooling are owned by UBloodCombatVFXSubsystem.
 */
class FVFX_BloodCombat_SystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation. */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
