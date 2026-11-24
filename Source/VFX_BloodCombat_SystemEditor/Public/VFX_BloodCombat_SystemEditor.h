// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/**
 * Editor-only entry point for the blood combat VFX plugin.
 * Reserved for editor tooling; runtime behaviour must stay in the runtime module.
 */
class FVFX_BloodCombat_SystemEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation. */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
