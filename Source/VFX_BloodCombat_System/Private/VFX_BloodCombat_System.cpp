// Copyright Epic Games, Inc. All Rights Reserved.

#include "VFX_BloodCombat_System.h"

#define LOCTEXT_NAMESPACE "FVFX_BloodCombat_SystemModule"

void FVFX_BloodCombat_SystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FVFX_BloodCombat_SystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVFX_BloodCombat_SystemModule, VFX_BloodCombat_System)