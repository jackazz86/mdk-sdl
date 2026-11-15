# MDK for the Nintendo Switch (unofficial fork, work in progress)

This is a fork of **mdk-sdl**, the C# + SDL3 port of *MDK* (Shiny Entertainment, 1997) by Marek Draskaba
([nautilusbits/mdk-sdl](https://github.com/nautilusbits/mdk-sdl)). The `switch` branch adds what is
needed to run it on Nintendo Switch homebrew (Atmosphere): on the console's GPU through Vulkan (Mesa NVK),
with Mono compiled ahead of time (full AOT), using
[NroForge](https://github.com/BennettNotFound/NroForge) as the build chain. The game itself is unchanged
apart from the small edits listed below; `main` follows the original repository.

> **You need the original game.** This repository contains no data from MDK: no levels, textures,
> models, sounds, music or videos. Copy `TRAVERSE`, `MISC`, `FALL3D` and `STREAM` from your own copy
> of the game (for example the GOG one) to the console's SD card. Not affiliated with, or endorsed by,
> Shiny Entertainment or the current rights holders of MDK. No binaries or releases are published from
> this repository.

## What is where, and under which licence

| Path | What | Licence |
| --- | --- | --- |
| everything except `switch/` | **mdk-sdl 1.2.6** by Marek Draskaba, with the Switch changes listed below | **GPL-3.0** (`LICENSE`), copyright of the original author for his part; the modifications are marked and published under the same licence |
| `switch/` | NroForge (runner, SDK patches, `nroforge.py`, ...), derived from mono-nx and a Switch port of another game; `switch/runner/native/` holds the pad and gyro code and the SDL3 symbol table of this port | MIT (`switch/LICENSE`), with the components in `switch/THIRD_PARTY_NOTICES.md` and `switch/licenses/` |
| `switch/icon.jpg` | The NRO's icon: "MDK (1997)" pixel-art icon by swagmaster9000 from [SteamGridDB](https://www.steamgriddb.com/) | Not covered by any licence here; belongs to its author and the rights holders of the game |

The game's SDL3 binding is `ppy.SDL3-CS` (MIT); SDL3 is zlib licensed; Mono, .NET and Mesa keep their own
licences (see `switch/licenses/`).

## Changes made to mdk-sdl for the Switch

Marked as modifications of the original, as the GPL requires (`git diff v1.2.6 switch -- src`):

- `src/Mdk.Switch/` (new): the program that starts the game on the console.
- `src/Mdk.Engine/Platform/SwitchBridge.cs` (new), `Window.cs`: the pad bridge hook; no on-screen
  keyboard and no window icon on the console (its SDL 3.2 lacks `SDL_LoadPNG_IO`).
- `src/Mdk.Engine/Render/Renderer.cs`: no GPU properties query on the console (`SDL_GetGPUDeviceProperties` is SDL 3.4+).
- `src/Mdk.Formats/MdkData.cs`, `src/Mdk.Game/DevTools/TechInfo.cs`, `OverlayText.cs`: Mono on the
  Switch has no base directory, no `JitInfo` and no working set.
- `src/Mdk.Game/Viewer.cs`: the quick save does not ask for a name on the Switch (no keyboard).
- `src/Mdk.Game/Flow/Game.cs`: no `mdk.log` on the Switch (it flushes every line to the SD card, which
  stutters explosions; the output already goes to the SD log of the runner).
- `.github/workflows/build.yml`: runs only when asked (the upstream build is five platforms);
  `.github/workflows/switch.yml` (new) builds the NRO.

## Controls

| Pad | Action |
| --- | --- |
| Left stick | Move |
| Right stick / gyro | Look |
| A / B | Jump / run |
| X / Y | Use the item / next item |
| L / R | Previous / next item or ammo |
| ZR / ZL | Fire / sniper mode |
| D-pad up / down | Zoom in / out in the scope |
| `+` | Pause menu |
| `-` (tap) / `-` (hold 2 s) | Quick save / quick load |
| R3 | Gyro on / off |

The gyro reads the six-axis sensor (all fresh samples per frame, turning in player space) and is tuned
in `/switch/mdk/gyro.cfg` (`gyro`, `gyro_sweep`, `gyro_tilt`, `gyro_pitch`, `gyro_deadzone`, `gyro_debug`);
a negative gain flips that direction.

## Building

The GitHub Actions workflow `Build for Switch` (run manually) publishes the managed program as .NET 9
(the Switch runs Mono; the workflow switches `Directory.Build.props` from net10.0 to net9.0 in its own checkout), compiles the shaders with `dxc`, and runs NroForge,
which produces `mdk.nro`. The SD layout is `/switch/mdk/mdk.nro`, `/switch/mdk/MDK/` (the game's data) and
`/mono/etc/icudt77l.dat` (ICU data, from the NroForge output).

Status: boots and plays on hardware; found problems are listed in the issues.
