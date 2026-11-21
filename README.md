# VFX Blood Combat System

[Español](README_ES.md)

A data-driven combat VFX plugin for Unreal Engine 5 built on top of Niagara. It provides pooled Niagara effects, surface-aware impact mapping, blood decals, and weapon blood trails through a single world subsystem and a small set of Blueprint function libraries.

---

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Configuration](#configuration)
- [API Reference](#api-reference)
- [Parameters](#parameters)
- [Niagara Interface](#niagara-interface)
- [Architecture](#architecture)
- [Content Layout](#content-layout)
- [Troubleshooting](#troubleshooting)
- [Notes and Limitations](#notes-and-limitations)
- [License](#license)

---

## Features

- **Centralized world subsystem** — `UBloodCombatVFXSubsystem` owns spawning, configuration, and pooling.
- **Pooled Niagara components** — components are reused per `UNiagaraSystem` and released automatically when the system finishes.
- **Surface-aware impacts** — map an impact type to a different effect per surface (`Flesh`, `Metal`, `Stone`, `Wood`, `Fabric`), with automatic fallback to `Default`.
- **Layered impact effects** — main system plus optional spark and flash systems.
- **Blood effects** — spray, burst, droplets, decals, body trails, and sword trails.
- **Decal pooling** — a reusable `BP_DecalComponent` actor handles decal spawn requests emitted from Niagara.
- **Blueprint friendly** — every public function is `BlueprintCallable`; no C++ is required to use the plugin.
- **Runtime reconfiguration** — swap the config data asset at any time with `SetConfigDataAsset`.

---

## Requirements

| Requirement | Value |
| --- | --- |
| Unreal Engine | 5.x (developed against 5.8) |
| Plugins | **Niagara** (enabled by the `.uplugin`) |
| Modules | `VFX_BloodCombat_System` (Runtime), `VFX_BloodCombat_SystemEditor` (Editor) |
| Rendering | Deferred renderer recommended for decals |

---

## Installation

1. Copy the `VFX_BloodCombat_System` folder into your project's `Plugins/` directory (or into `<Engine>/Plugins/` for an engine-level plugin).
2. Make sure the **Niagara** plugin is enabled in your project.
3. Enable **VFX_BloodCombat_System** in `Edit > Plugins`, or confirm it is listed in your `.uproject`:
   ```json
   {
     "Name": "VFX_BloodCombat_System",
     "Enabled": true
   }
   ```
4. Regenerate project files and compile the project (the runtime module has C++ sources).
5. Launch the editor. The subsystem is created automatically per world; nothing needs to be placed in the level.

> The default config asset is loaded from `/VFX_BloodCombat_System/Data/VFX_DataAsset`. Keep the plugin's `Content` folder intact or supply your own asset at runtime.

---

## Quick Start

### Blueprint

1. Drag off any object with a world context (an Actor works) and search for **Spawn Impact Effect** or **Spawn Blood Effect**.
2. Fill in `FBloodCombatVFXParams` — at minimum `Location`, `Normal`, and `SurfaceType`.
3. For a hit event, pass `Hit Component` from the hit result so the decal can be attached to the hit surface.

Common calls:

| Node | Purpose |
| --- | --- |
| `Spawn Impact Effect` | Spawns the main Niagara system for an impact type |
| `Spawn Impact Effects` | Spawns the configured spark + flash systems |
| `Spawn Blood Effect` | Spawns a blood effect, or a decal when `Decal` is selected |
| `Spawn Blood Effects` | Spawns a list of blood effects with the same parameters |
| `Get Blood Combat VFX Subsystem` | Access the subsystem for trail and direct-spawn calls |

### C++

Add the module to your target's dependencies:

```csharp
PrivateDependencyModuleNames.AddRange(new[] { "VFX_BloodCombat_System" });
```

Then spawn an impact:

```cpp
#include "Core/BloodCombatVFXSubsystem.h"
#include "Combat/VfxCombatTypes.h"
#include "Combat/CombatImpactTypes.h"
#include "Blood/VfxBloodTypes.h"

if (UBloodCombatVFXSubsystem* VFX = GetWorld()->GetSubsystem<UBloodCombatVFXSubsystem>())
{
    FBloodCombatVFXParams Params;
    Params.Location = Hit.ImpactPoint;
    Params.Normal = Hit.ImpactNormal;
    Params.SurfaceType = EBloodSurfaceType::Flesh;
    Params.HitComponent = Hit.GetComponent();
    Params.DecalSize = FVector(16.f, 32.f, 32.f);

    VFX->SpawnImpact(ECombatImpactType::Sword, Params);
    VFX->SpawnImpactEffects(ECombatImpactType::Sword, Params);
    VFX->SpawnBlood(EBloodEffectType::Burst, Params);
}
```

Continuous trails:

```cpp
UNiagaraComponent* Trail = VFX->StartBloodTrail(Params);

// Each tick while the effect should follow a target:
VFX->UpdateBloodTrail(Trail, CharacterMesh->GetSocketLocation(SocketName));

// When finished:
VFX->StopBloodTrail(Trail);
```

Sword trails require a skeletal mesh and at least two filtered sockets:

```cpp
TArray<FName> Sockets = { TEXT("Blade_Start"), TEXT("Blade_End") };
UNiagaraComponent* SwordTrail = VFX->StartSwordBloodTrail(Params, SwordMesh, Sockets);

// Later:
VFX->StopSwordBloodTrail(SwordTrail);
```

---

## Configuration

All effects are resolved through `UVfxBloodCombatConfigDataAsset` (`Content/Data/VFX_DataAsset`).

**Impact mappings** are keyed by `(ImpactType, SurfaceType)` and define:

| Field | Description |
| --- | --- |
| `ImpactType` | `None`, `Sword`, `HeavySword`, `Critical`, `Blunt` |
| `SurfaceType` | `Default`, `Flesh`, `Metal`, `Stone`, `Wood`, `Fabric` |
| `NiagaraSystem` | Main impact effect |
| `SparkSystem` | Optional spark layer |
| `FlashSystem` | Optional flash layer |
| `DecalMaterial` | Optional decal material |

Resolution order: exact `(ImpactType, SurfaceType)` match first, then `(ImpactType, Default)`. If neither exists, nothing spawns.

**Blood mappings** are keyed by `EBloodEffectType` (`None`, `Spray`, `Burst`, `Droplets`, `Decal`, `Trail`, `SwordBloodTrail`) and define a `NiagaraSystem` and a `DecalMaterial`.

To use a different asset:

```cpp
Subsystem->SetConfigDataAsset(MyConfigAsset);
```

---

## API Reference

### `UBloodCombatVFXSubsystem`

| Function | Returns | Description |
| --- | --- | --- |
| `SpawnSystem` | `UNiagaraComponent*` | Spawns an explicit Niagara system with the given params |
| `SpawnImpact` | `UNiagaraComponent*` | Resolves and spawns the main impact system |
| `SpawnImpactEffects` | `void` | Spawns the configured spark and flash systems |
| `SpawnBlood` | `UNiagaraComponent*` | Resolves and spawns a blood effect |
| `SpawnBloodBurst` / `SpawnBloodSpray` / `SpawnBloodTrail` | `void` | Convenience wrappers |
| `SpawnBloodDecal` | `UDecalComponent*` | Spawns an attached or world-space decal |
| `StartBloodTrail` | `UNiagaraComponent*` | Starts a position-driven trail |
| `UpdateBloodTrail` | `void` | Feeds a new world position into the trail |
| `StopBloodTrail` | `void` | Stops and returns the trail component to the pool |
| `StartSwordBloodTrail` | `UNiagaraComponent*` | Starts a trail bound to a skeletal mesh socket range |
| `StopSwordBloodTrail` | `void` | Stops and pools a sword trail |
| `SetConfigDataAsset` | `void` | Replaces the active configuration |

### Blueprint function libraries

- `UVfxCombatLibrary` — `SpawnImpactEffect`, `SpawnImpactEffects`
- `UVfxBloodLibrary` — `SpawnBloodEffect`, `SpawnBloodEffects`
- `UVfxBloodCombatConfigDataAsset` — `GetImpactSystem`, `GetImpactSparkSystem`, `GetImpactFlashSystem`, `GetBloodSystem`, `GetBloodDecalMaterial`, `GetBloodDecalMaterialForEffect`

---

## Parameters

`FBloodCombatVFXParams` is the shared parameter struct.

| Group | Fields |
| --- | --- |
| Transform | `Location`, `Rotation`, `Normal`, `Scale` |
| VFX | `Intensity`, `SurfaceType`, `Instigator`, `RandomSeed` |
| Decal | `HitComponent`, `DecalSize`, `DecalLifeSpan` |
| User parameters | `FloatParameters`, `VectorParameters`, `ColorParameters` |
| Trail shaping | `StartPosition`, `Direction`, `Length`, `Duration`, `Width` |

Notes:

- If `Rotation` is near zero, the rotation is derived from `Normal`.
- `Scale` is applied as world scale on the Niagara component.
- Entries in the three user-parameter maps are forwarded to Niagara as `SetVariableFloat`, `SetVariableVec3`, and `SetVariableLinearColor`. Use full names such as `User.MyParameter`.
- `HitComponent` enables attached decals; otherwise the decal is placed in world space slightly offset along `Normal`.
- `Instigator`, `Intensity`, `RandomSeed`, and the trail-shaping fields are carried in the struct but are not yet forwarded to Niagara by the subsystem.

---

## Niagara Interface

Custom Niagara user parameters and data interfaces used by the bundled systems:

| Name | Type | Used by |
| --- | --- | --- |
| `User.TrailPosition` | Position | Blood and sword trails; updated by `UpdateBloodTrail` |
| `User.SwordMesh` | Skeletal Mesh data interface | Sword trails; filtered to the socket list passed to `StartSwordBloodTrail` |
| `User.CallbackHandler` | Object | Burst systems; set to a pooled `BP_DecalComponent` actor that creates the decal |

The decal handler expects these properties on `BP_DecalComponent`:

| Property | Type |
| --- | --- |
| `MaterialDecal` | Object (material interface) |
| `NiagaraSystemBurst` | Object (Niagara system) |
| `TamanoDecal` | Vector (decal size) |

---

## Architecture

```
Game code / Blueprint
        |
        v
UVfxCombatLibrary / UVfxBloodLibrary  --->  UBloodCombatVFXSubsystem (UWorldSubsystem)
                                                   |
                     +-----------------------------+---------------------+
                     |                             |                     |
          Config data asset (soft refs)   Niagara component pool   Decal handler pool
                     |                             |                     |
          Impact / blood mappings          OnSystemFinished          BP_DecalComponent
```

- The subsystem is a `UWorldSubsystem`, so each world gets its own pools and configuration.
- Niagara components are created with `bAutoActivate = false`, activated on spawn, and returned to a per-system pool when `OnSystemFinished` fires or when a trail is stopped.
- Decal handler actors are acquired and recycled alongside Niagara components that request decals.
- All pooled objects are destroyed in `Deinitialize()` when the world shuts down.

---

## Content Layout

```
Content/
├── Blueprints/   BP_DecalComponent        # pooled decal spawner actor
├── Data/         VFX_DataAsset            # default mapping configuration
├── Niagara/      NS_Blood_Spray, NS_Blood_Burst, NS_Blood_Trail,
│                 NS_SwordBlood_Trail, NS_Blood_Trail1, Blood_Trail
├── Materials/    BloodDecal, BloodBurst, BloodTrail, Blood_Spray,
│                 SwordBloodTrail_Mat, Test
└── Textures/     blood-Texture, Blood-Trail, HM_BloodTrail,
                  SwordBloodTrail, T_BloodTrail_Normal
```

---

## Troubleshooting

| Symptom | Likely cause |
| --- | --- |
| `BloodCombatVFX: no se pudo cargar el Data Asset...` | The plugin content was stripped, renamed, or not cooked; verify `/VFX_BloodCombat_System/Data/VFX_DataAsset`. |
| No impact effect appears | No mapping for the requested `(ImpactType, SurfaceType)`, or the soft object reference is invalid. Add an exact mapping or a `Default` fallback. |
| No decals appear | `BP_DecalComponent` is missing or its `MaterialDecal` / `NiagaraSystemBurst` / `TamanoDecal` properties were renamed; the burst system may not be setting `User.CallbackHandler`. |
| Sword trail warning about mesh or sockets | `SwordMesh` is null or fewer than two filtered socket names were supplied. |
| Effects are invisible in packaged builds | Niagara systems or materials are not referenced by a cooked asset; check the data asset soft references and packaging settings. |

---

## Notes and Limitations

- `Content/Data/VFX_DataAsset` is the only asset loaded automatically; overrides must be set explicitly.
- Material and Niagara lookups use synchronous soft-object loading, so first use of a new effect may cause a hitch. Pre-warm assets if that matters for your project.
- Log messages currently use Spanish text; search for `BloodCombatVFX:` in the Output Log.
- The editor module is currently an empty shell; all functionality lives in the runtime module.

---

## License

Licensed under the [MIT License](LICENSE).

Copyright (c) 2026 Espi.

This license applies to the entire plugin, including the C++ source code and all assets under `Content/` (Blueprints, Niagara systems, materials, and textures).

Created by **Espi**.
