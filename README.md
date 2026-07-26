# LeviVision

Plugin nativo (`.so` / `.levipack`) para **LeviLaunchroid** com:

- Night Vision (liga/desliga + persistência)
- X-Ray (transparência de blocos + outline)
- Glow Ores (destaque emissivo)
- Mod Menu completo (toggles + sliders)
- Botão flutuante **LV**
- Resource pack companion (X-Ray + Glow)

Compatível com:

- LeviLaunchroid + preloader-android SDK (`pl::Mod`, `pl::Config`, `pl::ModMenu`)
- Minecraft Bedrock `1.21.100*`
- Android **ARM64** (`arm64-v8a`)

---

## Estrutura

```text
LeviVision/
├── manifest.json / manifest.json.in
├── CMakeLists.txt
├── README.md
├── assets/                  # icon do mod
├── config/                  # config.json + schema
├── include/                 # headers públicos do mod
├── src/                     # implementação
├── resource_pack/
│   └── LeviVision_RP/       # pack de texturas X-Ray / Glow
└── scripts/
    └── package.ps1
```

---

## Build (Android NDK + CMake)

Requisitos:

- Android SDK
- NDK 28.x (ou compatível)
- CMake 3.22+
- Ninja
- PowerShell (script de package) **ou** comandos CMake manuais

Exemplo ARM64:

```powershell
$env:ANDROID_HOME = "C:/Users/<you>/AppData/Local/Android/Sdk"
$ndk = "$env:ANDROID_HOME/ndk/28.2.13676358"

cmake -S . -B build-arm64-v8a `
  -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE="$ndk/build/cmake/android.toolchain.cmake" `
  -DANDROID_ABI=arm64-v8a `
  -DANDROID_PLATFORM=android-28 `
  -DANDROID_STL=c++_shared `
  -DMOD_ID=levivision `
  -DMOD_NAME="LeviVision" `
  -DMOD_AUTHOR="Say" `
  -DMOD_VERSION=1.0.0 `
  -DMOD_LIBRARY_NAME=levivision `
  -DMOD_MINECRAFT_VERSIONS='["1.21.100*"]' `
  -DMOD_ICON="assets/icon.png"

cmake --build build-arm64-v8a --target levi_package
```

O artefato `.levipack` sai em `build-arm64-v8a/`.

Importe o `.levipack` no LeviLaunchroid.

---

## Mod Menu

Módulo: **LeviVision**

| Config           | Tipo       | Descrição                          |
|------------------|------------|------------------------------------|
| Night Vision     | Toggle     | Brilho / visão noturna             |
| X-Ray            | Toggle     | Transparente em não-minérios       |
| Glow Ores        | Toggle     | Destaque emissivo nos minérios     |
| Outline          | Toggle     | Contorno nos minérios              |
| Transparency     | Slider 0–100 | Opacidade do X-Ray              |
| Glow Strength    | Slider 0–100 | Intensidade do glow             |
| Render Distance  | Slider 16–512 | Raio lógico de efeito          |

Botão flutuante **LV**: ciclo rápido NV → X-Ray → Glow → off.

---

## Resource pack (X-Ray / Glow)

Pasta: `resource_pack/LeviVision_RP/`

1. Adicione PNGs em `textures/blocks/` (veja `README_TEXTURES.txt`).
2. Empacote a pasta como `.mcpack` ou copie para resource packs do mundo.
3. Ative o pack **junto** com o plugin nativo.

### RenderDragon / MaterialBinLoader

O pack inclui:

- `terrain_texture.json` apontando stone transparente + ores “glow”
- `blocks.json` para ores principais
- stub de `materials/` para extensão futura

**Emissivo real no RenderDragon** (glow sem depender só de textura brilhante) costuma exigir:

- MaterialBinLoader (ou equivalente no launcher)
- bins/materials específicos da versão do jogo

O código nativo em `Hooks.cpp` está preparado para receber signatures de `libminecraftpe.so` quando você as tiver para o build exato.

---

## Lifecycle

| Fase      | Quando                         |
|-----------|--------------------------------|
| `load()`  | Carrega config tipada          |
| `enable()`| Menu, botão, hooks, aplica CFG |
| `disable()` | Desliga efeitos, salva CFG  |
| `unload()`| Libera config file             |

---

## APIs usadas (somente públicas)

```cpp
#include <pl/Mod.hpp>
#include <pl/ModMenu.hpp>
#include <pl/Config.hpp>
#include <pl/memory/Hook.hpp>
#include <pl/memory/Signature.hpp>
```

---

## Notas importantes

1. **Night Vision nativo** precisa de hook de brilho/fog (signature por versão).
2. **X-Ray / Glow** funcionam de forma prática via resource pack; o nativo controla toggles e parâmetros.
3. Não use headers internos do preloader — só `include/pl/*`.
4. Compile sempre para `arm64-v8a` (filtro principal do launcher).
