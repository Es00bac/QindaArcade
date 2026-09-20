# Qinda Arcade — playable builds

Two native Linux games developed from the Prism Brawl and Prism Circuit
screensavers. These are playable local games, not screensavers: input controls
your character, Escape pauses, matches finish at results, and rematches are
explicit. The original screensaver programs remain separate and unchanged.

## Play

Run `./play-brawl` or `./play-kart` from this directory, or open **Qinda Mega
Brawl** / **Qinda Prism Kart** in the desktop's Games menu. No network login or
download is required. Both open in a resizable window; F11 toggles fullscreen.

- **Mega Brawl:** 1–4 local humans, CPU opponents, eight characters, four arenas,
  three stocks, 90-second rounds, damage-based knockback, double jumps, recovery,
  attacks, grabs, shields, dodges, character specials, pickups and rematches.
- **Prism Kart:** eight racers, 1–4 local humans, split-screen, four courses,
  three laps, manual steering/throttle/braking, charged drift boosts, item boxes,
  single races and a four-race Grand Prix with cumulative standings.

CPU opponents fill unused seats. Brawl shares one camera; Kart uses two-, three-
or four-player layouts. One keyboard may accompany up to three controllers;
four controllers also work. Connect controllers before starting local play.
Player slots survive disconnect/reconnect. Disconnecting or leaving the window
pauses a live game. Pair Bluetooth devices through the desktop as usual.

## Controls

| Action | Keyboard | Xbox / standard | PlayStation |
|---|---|---|---|
| Menus | Arrows, Enter, Esc | D-pad, A, B | D-pad, Cross, Circle |
| Pause | Esc | Start/Menu | Options |
| Move / steer | WASD or arrows | Left stick / D-pad | Left stick / D-pad |
| Brawl jump | Space | A | Cross |
| Brawl attack / heavy | J / L | X / Y | Square / Triangle |
| Brawl special | K | B | Circle |
| Brawl recovery | Up + K | Up + B | Up + Circle |
| Brawl shield / dodge | Shift / E | LB or LT / RB | L1 or L2 / R1 |
| Brawl grab | R | RT | R2 |
| Kart accelerate / brake | W / S | RT or A / LT or B | R2 or Cross / L2 or Circle |
| Kart drift | E, while steering | RB, while steering | R1, while steering |
| Kart item | J or Shift | X or LB | Square or L1 |

Drop through an upper fighting platform with Down. Drift charge has three
thresholds; releasing drift spends it on a turbo. Items stay in your inventory
until you activate them. Human inputs are never replaced by the exhibition AI.
Kart handling is track-bounded arcade racing; reverse driving and free-roam
off-road physics are not implemented.

SDL's controller API and the bundled mapping database cover Xbox, PlayStation,
Nintendo and many third-party pads. **Controllers → Map this controller** adds
a persistent 12-step mapping for an unrecognized joystick; move the stick right
and down, then press the requested face/shoulder/trigger buttons. Logical face
positions are used on controllers with non-Xbox labeling. Vibration is sent only
through supported device APIs; special hardware features such as gyro, touchpad
and adaptive-trigger effects are not implemented. No physical controller was
connected during validation: Xbox/PlayStation testing used SDL virtual devices.

Settings include effects volume, stick deadzone, vibration and fullscreen.
Preferences and custom mappings are saved in `$XDG_CONFIG_HOME/qinda-arcade`
(normally `~/.config/qinda-arcade`). `QINDA_CONFIG` selects a separate profile.

## Levels and artwork

Bliss Reclamation is based on the installed Qinda Bliss wallpaper, not a clean
Windows XP hill: it has a depot straight, monitor windrows, cableworks bends,
wind-pump ridge, exposed circuit boards, rusted hardware and meadow breaks.
The road is banked into terrain, with guardrails, sector gantries, start grid,
route markers and boost pads following its actual geometry.

Afterglow Viaduct follows an elevated city perimeter; Reactor Gardens winds
through planted utility terraces; Aurora Pass is a mountain-road environment.
Brawl's terminal, utility garden, rooftop plant and salvage yard use supported
decks, coherent background structures and a clear foreground combat plane.

The [Blender source library](assets/models/reclaimed-world.blend), exported GLB
and runtime meshes are included. [Asset notes and image-generation prompt](assets/README.md)
describe the saved texture atlas and its provenance.

This is a substantial playable foundation with a stylized art pass, not a claim
of finished commercial AA/AAA production quality. Bespoke character animation,
deeper combat/racing balance, music, accessibility expansion and physical-pad
playtesting remain production work. Multiplayer is local; there is no online
netcode or campaign.

## Build and test

Requirements: C++20, CMake, Ninja, SDL2 ≥ 2.24, Cairo, OpenGL 3.3 and a working
graphics driver. Blender and the image-generation tool are authoring tools;
neither is needed to play.

```sh
./build-games
```

The script reads `portageq envvar MAKEOPTS` and passes the configured job and
load limits unchanged to Ninja. It never sets or replaces `MAKEOPTS`. At the
time of this build the system returned `-j24 -l24`. No Portage configuration or
system package was changed. Build from this root, not from `games/brawl` or
`games/circuit`; those directories contain the two game implementations.

```sh
./play-kart --play --course bliss
./play-brawl --play --stage bliss
./play-kart --help
ctest --test-dir build --output-on-failure
```

`--demo` runs an AI exhibition; `--snapshot FILE.png`, `--start SECONDS`,
`--overview`, `--benchmark FRAMES` and `--replay FILE` support repeatable QA.
Replay events use wall time normally, simulation-frame time with `--benchmark`.
`--mute` disables sound and `--no-msaa` lowers rendering cost. `QINDA_ASSETS`
overrides asset discovery. Assets are otherwise found in the source project or
beside an installed binary at `../share/qinda-arcade`.

[Validation notes and screenshots](validation/README.md) record the tests and
remaining limitations. Code derives from the GPL-3.0-or-later screensavers;
see [LICENSE](games/brawl/LICENSE). The controller database retains its separate
[license](assets/SDL_GameControllerDB-LICENSE.txt).
