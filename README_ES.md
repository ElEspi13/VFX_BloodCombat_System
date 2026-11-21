# VFX Blood Combat System

[English](README.md)

Un plugin de efectos de combate para Unreal Engine 5 basado en Niagara. Centraliza los efectos de impactos y sangre en un *world subsystem*, con agrupación de componentes (*pooling*), mapeo de impactos según el tipo de superficie, decos de sangre y estelas de armas.

---

## Índice

- [Características](#características)
- [Requisitos](#requisitos)
- [Instalación](#instalación)
- [Inicio rápido](#inicio-rápido)
- [Configuración](#configuración)
- [Referencia de la API](#referencia-de-la-api)
- [Parámetros](#parámetros)
- [Interfaz con Niagara](#interfaz-con-niagara)
- [Arquitectura](#arquitectura)
- [Estructura del contenido](#estructura-del-contenido)
- [Solución de problemas](#solución-de-problemas)
- [Notas y limitaciones](#notas-y-limitaciones)
- [Licencia](#licencia)

---

## Características

- **Subsistema central** — `UBloodCombatVFXSubsystem` gestiona la creación, la configuración y el *pooling* de efectos.
- **Componentes Niagara reutilizables** — se agrupan por `UNiagaraSystem` y se devuelven al pool cuando el sistema termina.
- **Impactos según superficie** — un mismo tipo de impacto puede tener un efecto distinto para `Flesh`, `Metal`, `Stone`, `Wood` o `Fabric`, con retroceso automático a `Default`.
- **Impactos por capas** — sistema principal más capas opcionales de chispas y destello.
- **Efectos de sangre** — rociado, explosión, gotas, decos, estela corporal y estela de espada.
- **Decos agrupados** — un actor `BP_DecalComponent` reutilizable crea los decos solicitados desde Niagara.
- **Preparado para Blueprint** — todas las funciones públicas son `BlueprintCallable`; no hace falta C++ para usarlo.
- **Reconfiguración en ejecución** — se puede cambiar el data asset con `SetConfigDataAsset`.

---

## Requisitos

| Requisito | Valor |
| --- | --- |
| Unreal Engine | 5.x (desarrollado sobre 5.8) |
| Plugins | **Niagara** (activado por el `.uplugin`) |
| Módulos | `VFX_BloodCombat_System` (Runtime), `VFX_BloodCombat_SystemEditor` (Editor) |
| Renderizado | Se recomienda el renderizador diferido para los decos |

---

## Instalación

1. Copia la carpeta `VFX_BloodCombat_System` en el directorio `Plugins/` de tu proyecto (o en `<Engine>/Plugins/` si lo quieres a nivel de motor).
2. Asegúrate de que el plugin **Niagara** esté activado en el proyecto.
3. Activa **VFX_BloodCombat_System** en `Edit > Plugins`, o comprueba que aparece en el `.uproject`:
   ```json
   {
     "Name": "VFX_BloodCombat_System",
     "Enabled": true
   }
   ```
4. Regenera los archivos del proyecto y compila (el módulo runtime tiene código C++).
5. Abre el editor. El subsistema se crea automáticamente para cada mundo; no hay que colocar nada en el nivel.

> El data asset por defecto se carga desde `/VFX_BloodCombat_System/Data/VFX_DataAsset`. Mantén intacta la carpeta `Content` del plugin o proporciona tu propio asset en ejecución.

---

## Inicio rápido

### Blueprint

1. Arrastra desde cualquier objeto con contexto de mundo (un Actor sirve) y busca **Spawn Impact Effect** o **Spawn Blood Effect**.
2. Rellena `FBloodCombatVFXParams` — como mínimo `Location`, `Normal` y `SurfaceType`.
3. En un evento de impacto, pasa `Hit Component` del resultado para que el deco se adhiera a la superficie golpeada.

Nodos habituales:

| Nodo | Uso |
| --- | --- |
| `Spawn Impact Effect` | Crea el sistema Niagara principal de un impacto |
| `Spawn Impact Effects` | Crea las capas de chispas y destello configuradas |
| `Spawn Blood Effect` | Crea un efecto de sangre, o un deco si se selecciona `Decal` |
| `Spawn Blood Effects` | Crea una lista de efectos con los mismos parámetros |
| `Get Blood Combat VFX Subsystem` | Accede al subsistema para estelas y creación directa |

### C++

Añade el módulo a las dependencias de tu *target*:

```csharp
PrivateDependencyModuleNames.AddRange(new[] { "VFX_BloodCombat_System" });
```

Crear un impacto:

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

Estelas continuas:

```cpp
UNiagaraComponent* Trail = VFX->StartBloodTrail(Params);

// En cada tick mientras la estela deba seguir al objetivo:
VFX->UpdateBloodTrail(Trail, CharacterMesh->GetSocketLocation(SocketName));

// Al terminar:
VFX->StopBloodTrail(Trail);
```

La estela de espada necesita una malla esquelética y al menos dos sockets filtrados:

```cpp
TArray<FName> Sockets = { TEXT("Blade_Start"), TEXT("Blade_End") };
UNiagaraComponent* SwordTrail = VFX->StartSwordBloodTrail(Params, SwordMesh, Sockets);

// Después:
VFX->StopSwordBloodTrail(SwordTrail);
```

---

## Configuración

Todos los efectos se resuelven mediante `UVfxBloodCombatConfigDataAsset` (`Content/Data/VFX_DataAsset`).

**Los mapeos de impacto** se indexan por `(ImpactType, SurfaceType)` y definen:

| Campo | Descripción |
| --- | --- |
| `ImpactType` | `None`, `Sword`, `HeavySword`, `Critical`, `Blunt` |
| `SurfaceType` | `Default`, `Flesh`, `Metal`, `Stone`, `Wood`, `Fabric` |
| `NiagaraSystem` | Efecto principal del impacto |
| `SparkSystem` | Capa opcional de chispas |
| `FlashSystem` | Capa opcional de destello |
| `DecalMaterial` | Material de deco opcional |

Orden de resolución: primero la coincidencia exacta `(ImpactType, SurfaceType)`, después `(ImpactType, Default)`. Si no existe ninguna, no se crea nada.

**Los mapeos de sangre** se indexan por `EBloodEffectType` (`None`, `Spray`, `Burst`, `Droplets`, `Decal`, `Trail`, `SwordBloodTrail`) y definen un `NiagaraSystem` y un `DecalMaterial`.

Para usar otro asset:

```cpp
Subsystem->SetConfigDataAsset(MyConfigAsset);
```

---

## Referencia de la API

### `UBloodCombatVFXSubsystem`

| Función | Devuelve | Descripción |
| --- | --- | --- |
| `SpawnSystem` | `UNiagaraComponent*` | Crea un sistema Niagara explícito con los parámetros dados |
| `SpawnImpact` | `UNiagaraComponent*` | Resuelve y crea el sistema principal del impacto |
| `SpawnImpactEffects` | `void` | Crea las capas de chispas y destello configuradas |
| `SpawnBlood` | `UNiagaraComponent*` | Resuelve y crea un efecto de sangre |
| `SpawnBloodBurst` / `SpawnBloodSpray` / `SpawnBloodTrail` | `void` | Atajos de conveniencia |
| `SpawnBloodDecal` | `UDecalComponent*` | Crea un deco adjunto o en el mundo |
| `StartBloodTrail` | `UNiagaraComponent*` | Inicia una estela controlada por posición |
| `UpdateBloodTrail` | `void` | Envía una nueva posición mundial a la estela |
| `StopBloodTrail` | `void` | Detiene la estela y devuelve el componente al pool |
| `StartSwordBloodTrail` | `UNiagaraComponent*` | Inicia una estela ligada a un rango de sockets de una malla esquelética |
| `StopSwordBloodTrail` | `void` | Detiene y agrupa una estela de espada |
| `SetConfigDataAsset` | `void` | Reemplaza la configuración activa |

### Bibliotecas de funciones Blueprint

- `UVfxCombatLibrary` — `SpawnImpactEffect`, `SpawnImpactEffects`
- `UVfxBloodLibrary` — `SpawnBloodEffect`, `SpawnBloodEffects`
- `UVfxBloodCombatConfigDataAsset` — `GetImpactSystem`, `GetImpactSparkSystem`, `GetImpactFlashSystem`, `GetBloodSystem`, `GetBloodDecalMaterial`, `GetBloodDecalMaterialForEffect`

---

## Parámetros

`FBloodCombatVFXParams` es la estructura de parámetros compartida.

| Grupo | Campos |
| --- | --- |
| Transformación | `Location`, `Rotation`, `Normal`, `Scale` |
| VFX | `Intensity`, `SurfaceType`, `Instigator`, `RandomSeed` |
| Deco | `HitComponent`, `DecalSize`, `DecalLifeSpan` |
| Parámetros de usuario | `FloatParameters`, `VectorParameters`, `ColorParameters` |
| Configuración de estela | `StartPosition`, `Direction`, `Length`, `Duration`, `Width` |

Notas:

- Si `Rotation` es casi cero, la rotación se calcula a partir de `Normal`.
- `Scale` se aplica como escala mundial del componente Niagara.
- Las entradas de los tres mapas de parámetros se envían a Niagara mediante `SetVariableFloat`, `SetVariableVec3` y `SetVariableLinearColor`. Usa nombres completos como `User.MiParametro`.
- `HitComponent` permite crear decos adjuntos; si no hay componente, el deco se coloca en el mundo ligeramente desplazado a lo largo de `Normal`.
- `Instigator`, `Intensity`, `RandomSeed` y los campos de configuración de estela están en la estructura, pero todavía no se envían a Niagara desde el subsistema.

---

## Interfaz con Niagara

Parámetros y interfaces de usuario personalizados que usan los sistemas incluidos:

| Nombre | Tipo | Uso |
| --- | --- | --- |
| `User.TrailPosition` | Position | Estelas corporales y de espada; lo actualiza `UpdateBloodTrail` |
| `User.SwordMesh` | Skeletal Mesh data interface | Estelas de espada; se filtra con la lista de sockets pasada a `StartSwordBloodTrail` |
| `User.CallbackHandler` | Object | Sistemas de explosión; apunta a un actor `BP_DecalComponent` del pool que crea el deco |

El actor de decos espera estas propiedades en `BP_DecalComponent`:

| Propiedad | Tipo |
| --- | --- |
| `MaterialDecal` | Object (material interface) |
| `NiagaraSystemBurst` | Object (Niagara system) |
| `TamanoDecal` | Vector (tamaño del deco) |

---

## Arquitectura

```
Código de juego / Blueprint
        |
        v
UVfxCombatLibrary / UVfxBloodLibrary  --->  UBloodCombatVFXSubsystem (UWorldSubsystem)
                                                   |
                     +-----------------------------+---------------------+
                     |                             |                     |
        Data asset de configuración      Pool de componentes      Pool de actors de deco
          (referencias suspendidas)          Niagara                   |
                     |                             |             BP_DecalComponent
          Mapeos de impacto / sangre       OnSystemFinished
```

- El subsistema es un `UWorldSubsystem`, por lo que cada mundo tiene sus propios pools y configuración.
- Los componentes Niagara se crean con `bAutoActivate = false`, se activan al crearlos y vuelven a un pool por sistema cuando salta `OnSystemFinished` o se detiene una estela.
- Los actors de decos se adquieren y reciclan junto con los componentes Niagara que solicitan decos.
- Todos los objetos agrupados se destruyen en `Deinitialize()` al cerrar el mundo.

---

## Estructura del contenido

```
Content/
├── Blueprints/   BP_DecalComponent        # actor creador de decos reutilizable
├── Data/         VFX_DataAsset            # configuración de mapeos por defecto
├── Niagara/      NS_Blood_Spray, NS_Blood_Burst, NS_Blood_Trail,
│                 NS_SwordBlood_Trail, NS_Blood_Trail1, Blood_Trail
├── Materials/    BloodDecal, BloodBurst, BloodTrail, Blood_Spray,
│                 SwordBloodTrail_Mat, Test
└── Textures/     blood-Texture, Blood-Trail, HM_BloodTrail,
                  SwordBloodTrail, T_BloodTrail_Normal
```

---

## Solución de problemas

| Síntoma | Causa probable |
| --- | --- |
| `BloodCombatVFX: no se pudo cargar el Data Asset...` | El contenido del plugin se eliminó, se renombró o no se cocinó; comprueba `/VFX_BloodCombat_System/Data/VFX_DataAsset`. |
| No aparece el impacto | Falta un mapeo para el par `(ImpactType, SurfaceType)` o la referencia suspendida no es válida. Añade un mapeo exacto o uno `Default`. |
| No aparecen decos | Falta `BP_DecalComponent` o se renombraron sus propiedades `MaterialDecal` / `NiagaraSystemBurst` / `TamanoDecal`; puede que el sistema de explosión no esté asignando `User.CallbackHandler`. |
| Aviso de estela de espada sobre malla o sockets | `SwordMesh` es nulo o se pasaron menos de dos nombres de sockets filtrados. |
| Los efectos no se ven en builds empaquetados | Los sistemas Niagara o los materiales no están referenciados por un asset cocinado; revisa el data asset y la configuración de empaquetado. |

---

## Notas y limitaciones

- Solo se carga automáticamente `Content/Data/VFX_DataAsset`; cualquier sustitución debe configurarse explícitamente.
- Los materiales y sistemas Niagara se cargan de forma síncrona, por lo que el primer uso de un efecto puede causar un tirón. Precarga los assets si es importante para tu proyecto.
- Los mensajes de log están actualmente en español; busca `BloodCombatVFX:` en el Output Log.
- El módulo editor es una carcasa vacía; toda la funcionalidad está en el módulo runtime.

---

## Licencia

Licenciado bajo la [licencia MIT](LICENSE).

Copyright (c) 2026 Espi.

Esta licencia aplica a todo el plugin, incluido el código C++ y todos los recursos de `Content/` (Blueprints, sistemas Niagara, materiales y texturas).

Creado por **Espi**.
