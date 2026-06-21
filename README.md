# Touhou 6: The Embodiment of Scarlet Devil — PSP port

An experimental PSP homebrew port of **Touhou Koumakyou ~ The Embodiment of
Scarlet Devil** (TH06), based on the `portable` branch of
[GensokyoClub/th06](https://github.com/GensokyoClub/th06).

This repository contains source code only. It does **not** contain the original
game executable, DAT archives, music, or other copyrighted game assets. You
must supply data from a legally obtained copy of the Japanese PC release.

## Current status

- Boots as a standard PSP homebrew `EBOOT.PBP`.
- Title screen, menus, gameplay, enemies, player shots, sound effects, and WAV
  music are implemented.
- The original 640x480 scene is rendered at an exact 1/2 scale inside a
  centered 320x240 viewport with black borders on the PSP's 480x272 display.
- Textures and streaming are tuned for the 32 MiB PSP-1000.
- Frame presentation is synchronized to VBlank to prevent screen tearing.
- Tested with PPSSPP's PSP-1000 memory model. Real-hardware testing is still
  recommended for every release.

This is still an experimental port. Keep a backup of save and replay files.

## Requirements

- Windows 10 or 11 with PowerShell and WSL2 (Ubuntu is the default)
- [PSPDEV](https://github.com/pspdev/pspdev) with `psp-cmake`
- PSP builds of SDL2, SDL2_image, SDL2_ttf, and PSPGL
- A legally obtained TH06 installation containing:
  - `CM.DAT`, `ED.DAT`, `IN.DAT`, `MD.DAT`, `ST.DAT`, and `TL.DAT`
  - `bgm/th06_01.wav` through `bgm/th06_17.wav`

The original Windows executable is never copied or executed by the packaging
script. It is only hashed, when present, to warn about unexpected game versions.

## Build and package

Clone the repository and run the packaging script from PowerShell:

```powershell
git clone https://github.com/jeong-jimin-github/PSP_TH06.git
cd PSP_TH06
./scripts/build_psp.ps1 `
  -SourceDir "D:\Games\TH06" `
  -Pspdev "/home/your-wsl-user/pspdev"
```

If PSPDEV is installed at `/usr/local/pspdev`, the `-Pspdev` argument can be
omitted. Use `-WslDistro` if the WSL distribution is not named `Ubuntu`.

The completed Memory Stick layout is written to:

```text
build/psp-package/PSP/GAME/TH06/
```

Copy the whole `TH06` directory to `ms0:/PSP/GAME/TH06/`, or launch its
`EBOOT.PBP` with PPSSPP. The generated package is about 340 MiB because it
contains the original DAT files, WAV soundtrack, and extracted sidecar assets
that are read directly by the PSP build to avoid memory-heavy DAT decoding.

After the first successful build, packaging can be repeated without compiling:

```powershell
./scripts/build_psp.ps1 -SourceDir "D:\Games\TH06" -SkipBuild
```

## Controls

| PSP control | Action |
| --- | --- |
| D-pad / analog nub | Move / menu navigation |
| Cross | Shoot / confirm |
| Circle | Bomb / back |
| Square | Focus / slow movement |
| Start | Pause |
| Right trigger | Skip dialogue |

## PSP-specific behavior

- The CPU and bus clocks are set to 333/166 MHz.
- Static opaque textures use RGB565; masked sprites retain full alpha where
  needed in RGBA4444. Temporary decoded texture data is released after GPU
  upload.
- Release builds discard unused code and unwind metadata. The text font cache
  is temporarily released during synchronous stage setup, avoiding the memory
  allocation spike that can terminate a PSP-1000 at `Now Loading`.
- The PSP renderer uses a bounded, auto-flushing sprite batch instead of the
  desktop-sized 2.25 MiB vertex buffer. Dialogue portraits are loaded on first
  use rather than during the stage-transition allocation peak.
- BGM uses streamed PCM WAV data and reusable mixer buffers. On PSP, audio is
  queued from the frame loop to avoid real-hardware SDL/thread deadlocks.
- MIDI, window mode, and color-depth options are hidden because they are not
  meaningful on PSP. Music can be set to WAV or Off.
- Configuration, score data, saves, and replays are written beside the EBOOT.
  The generated directory must remain writable.
- `loadtrace.txt` is recreated beside the EBOOT on each launch and records the
  last completed stage-loading step for diagnosing real-hardware shutdowns.

## Development build

The PSP target can also be configured directly inside WSL:

```bash
export PSPDEV=/usr/local/pspdev
export PATH="$PSPDEV/bin:$PATH"
psp-cmake -S . -B build/psp -DCMAKE_BUILD_TYPE=Release
cmake --build build/psp -j2
```

`TH06_AUTOTEST_FRAMES`, `TH06_AUTOTEST_INPUT`, and
`TH06_FORCE_SIDECAR_ASSETS` are test-only CMake options and should remain off
for release builds.

## Repository policy

Do not open issues or pull requests containing original TH06 assets, game
executables, extracted data, music, or links to unauthorized downloads. Build
outputs and locally supplied game data are excluded by `.gitignore`.

## Credits and license

- Original game: ZUN / Team Shanghai Alice
- Portable reimplementation: contributors to
  [GensokyoClub/th06](https://github.com/GensokyoClub/th06)
- PSP port: this repository's contributors

The source code is distributed under the [GNU GPL v3](LICENSE), following the
upstream project. Touhou Project, Touhou Koumakyou, and all original game assets
belong to their respective rights holders. This project is unofficial and is
not affiliated with or endorsed by Team Shanghai Alice.
