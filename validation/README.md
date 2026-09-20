# Playable-game verification — 2026-09-19

These are native executable checks, not UI mockups. Screenshot files in this
directory were captured from the OpenGL framebuffer. Earlier `*-first` and
`*-second` files are retained as iteration evidence; use the links below for
the current level pass.

## Automated checks

Release and AddressSanitizer/UndefinedBehaviorSanitizer builds pass all seven
CTest targets. [Release log](ctest-release.log) · [Sanitizer log](ctest-sanitizers.log).

The tests cover simulation determinism, geometry/index bounds, all four arenas
and courses, combat damage/shields/specials, stocks and respawns, kart collision
separation, human control without AI takeover, button edges, double jump,
recovery, attacks/heavy/special/dodge/grab, recovery-camera widening, steering,
braking, drift-release boosts, manual item use and persistent results.
The racing input test drives three complete laps on every course through the
public human-input interface, then verifies that all opponents can finish and
that stopped finishers do not block them.

Controller tests use actual SDL virtual joystick devices with Xbox and Sony
identifiers and a deliberately unmapped generic USB joystick. They verify raw
fallback and its upgrade to a mapped controller, discovery, hot-plugging, deadzones, press/hold edges,
PlayStation-specific prompts, vibration dispatch, stable slots after removal
and reconnect, focus-loss key clearing, short press/release events during frame
stalls, multiple rapid menu taps, and all 12 mapping-wizard steps followed
by saving/applying the mapping. No physical controller was attached: USB and
Bluetooth hardware behavior, actual vibration and unusual adapters remain
unverified. The broad mapping database and raw-device wizard are implemented;
universal compatibility with every peripheral is not promised.

## Native input replays

`games/common/replay.hpp` injects real SDL input events. Each `expect` checks the
application's actual page, and incomplete replays fail. Benchmark-mode replays
advance by rendered simulation frames, allowing long races to run uncapped.

| Replay | Checked flow |
|---|---|
| [kart-input.replay](kart-input.replay) | Home → setup → racing; throttle/steer/drift; pause/resume; controller removal pauses |
| [brawl-input.replay](brawl-input.replay) | Home → setup → fighting; movement/jump/attack/special; pause/restart |
| [brawl-results.replay](brawl-results.replay) | Finished match → classification → rematch |
| [grand-prix.replay](grand-prix.replay) | Four complete races driven with throttle; course transitions; accumulated points; cup classification; new cup |
| [split-screen.replay](split-screen.replay) | Four virtual pads; four viewports at 1080p and 720p; resize; disconnect pause |
| [three-player.replay](three-player.replay) | Keyboard + Xbox + PlayStation; three-player setup and layout; device-specific prompts; pause |
| [fullscreen.replay](fullscreen.replay) | Native desktop F11 transition to 3840×2160 and back to a window |
| [rapid-input.replay](rapid-input.replay) | Multiple complete navigation/select taps inside one 10 FPS frame still start the game |

The Grand Prix replay deliberately holds throttle without an expert racing line;
it verifies completion and UI flow, not racing balance. Separate simulation
tests exercise heading-based steering and drift. A sound-enabled native brawl
smoke test opened the audio device without an SFX error and exited normally.

Example reproduction (from the project root):

```sh
SDL_VIDEODRIVER=offscreen QINDA_CONFIG=/tmp/qinda-qa \
  ./play-kart --replay validation/grand-prix.replay \
  --fps 10 --benchmark 14500 --size 1440x810 --mute
SDL_VIDEODRIVER=offscreen QINDA_CONFIG=/tmp/qinda-qa \
  ./play-brawl --play --start 94 --replay validation/brawl-results.replay --mute
```

## Visual review

- Kart: [Bliss Reclamation](kart-course-3.png), [Afterglow Viaduct](kart-course-0.png),
  [Reactor Gardens](kart-course-1.png), [Aurora Pass](kart-course-2.png).
- Brawl: [Salvage Yard](brawl-arena-3.png), [Prism Terminal](brawl-arena-0.png),
  [Reactor Garden](brawl-arena-1.png), [Afterglow Rooftop](brawl-arena-2.png).
- UI: [race setup](kart-setup.png), [live brawl](brawl-playing.png),
  [match classification](brawl-results.png), [cup standings](cup-final.png),
  [three players](kart-three-player.png), [four players](kart-four-player.png).

Review fixes included material-atlas bleeding, stretched beam textures, coarse
asphalt scale, noisy road banking, terrain/road clearance, camera framing,
opaque item shields, floating crane feet, support piers through the old knot
road, view-dependent scrap placement, finish-line blockers, dropped short input
taps during preview loads, rematch audio
serials, character uniqueness in local racing, and per-player controller prompts.

## Performance sample

Machine: AMD Radeon RX 5700 XT, Mesa 26.1.8, OpenGL 4.6, Linux
6.18.48-gentoo-dist-bin; SDL 2.32.8, Cairo 1.18.4, GCC 15.3.0. Release build,
1920×1080, 600 uncapped frames, default MSAA/bloom, effects muted,
`SDL_VIDEODRIVER=offscreen`:

| Scene | Observed average |
|---|---:|
| Bliss Kart exhibition, starting at 30 seconds | 301 FPS |
| Bliss Brawl exhibition, starting at 30 seconds | 537 FPS |
| Four-player Bliss Kart, starting grid and countdown | 97 FPS |

These are short throughput samples, not guaranteed minimum frame rates or
long-session frame-pacing measurements. The four-player case renders four
cameras and four shadow passes. Native 4K fullscreen was checked functionally,
not claimed to have the same performance as 1080p. The normal game caps at
60 FPS unless changed with `--fps`.

## Scope

Playable local modes, results/rematches, controller handling and the new asset
pipeline are implemented. Physical-controller playtesting, accessibility work,
music, deeper combat/racing balance, bespoke character animation and further
environment art remain production tasks. The work should not be represented
as finished commercial AA/AAA quality. There is no online multiplayer.

All direct builds inherited the actual Portage job/load configuration unchanged
(`portageq envvar MAKEOPTS`, which returned `-j24 -l24`). No build limits, system
package configuration or existing screensaver binaries were changed. Only
user-level desktop entries were added. Blender work used a separate temporary
authoring process, which was closed after saving the editable library.
