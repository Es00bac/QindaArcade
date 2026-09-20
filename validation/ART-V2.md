# Art revision 2 — native build verification

This pass changes the playable games in `Games/QindaMegaBrawlSmash`, not the
original screensavers. All screenshots below are native OpenGL captures. The
Blender libraries, generated textures and editable graphics are saved in the
project; nothing needs to be generated or downloaded at game launch.

## Eight machines, not eight paint swaps

The [racer bible](../docs/RACERS.md) gives every character an origin, moral role,
motivation, sponsor and construction story. The selected kart and a short
version of that story appear in the [race setup](kart-v2-1-menu.png).

| Driver | Machine / construction | Native views |
|---|---|---|
| CyberPengu | Breakwater — folded rescue-tug armor, batteries, recovery eye | [Front](kart-v2-0-front.png) · [Rear](kart-v2-0-rear.png) |
| Ducké | Pressure Drop — riveted tub, copper accumulator, mismatched hubcaps | [Front](kart-v2-1-front.png) · [Rear](kart-v2-1-rear.png) |
| Vix | Redline Riot — rally nose, donor fender, intercooler, tall wing | [Front](kart-v2-2-front.png) · [Rear](kart-v2-2-rear.png) |
| Cache | Lost & Found — road-sign bonnet, flatbed, cargo cage, spare tire | [Front](kart-v2-3-front.png) · [Rear](kart-v2-3-rear.png) |
| Mochi | Moonshot — narrow hillclimb nose, exposed triangulation, electric drive | [Front](kart-v2-4-front.png) · [Rear](kart-v2-4-rear.png) |
| Hex | Hostile Takeover — forked prototype, closed fairings, disc wheels | [Front](kart-v2-5-front.png) · [Rear](kart-v2-5-rear.png) |
| Patches | Common Ground — service buggy, timber rails, radiator, canvas tool roll | [Front](kart-v2-6-front.png) · [Rear](kart-v2-6-rear.png) |
| Axi | Little Current — research skiff, cooling pods, reservoir, instrument mast | [Front](kart-v2-7-front.png) · [Rear](kart-v2-7-rear.png) |

All eight use the same tire radius and four wheel contact positions. Ducké,
Mochi and Axi are smaller drivers with adjusted seats and steering placement,
not smaller karts. Sponsor plates, numbers, personal bumper stickers and stencil
graphics have independent padded UV islands. Wear differs by construction:
scraped salvage panels and repairs are more visible than Hex's restrained
prototype damage. These are authored wear details, not a dynamic damage system.

## Track-specific scenery

Twenty new prop types are shared selectively between four sets of working
districts. Counts below come from the full-course overview composition and
exclude the earlier CRT/server/fern/gantry kit and other procedural geometry.
Gameplay views cull distant clusters. Each course has forty stable clusters,
organized into four zones, with varied arrangements and material colors.

| Course | New types / instances | Districts | Native views |
|---|---:|---|---|
| Afterglow Viaduct | 7 / 110 | Transit, roof plant, freight, communications | [View 1](course-v2-0.png) · [View 2](course-v2-0-sector.png) |
| Reactor Gardens | 8 / 134 | Cultivation, irrigation, solar power, pump service | [View 1](course-v2-1.png) · [View 2](course-v2-1-sector.png) |
| Aurora Pass | 8 / 260 | Survey, snow control, communications, rescue | [View 1](course-v2-2.png) · [View 2](course-v2-2-sector.png) |
| Bliss Reclamation | 11 / 161 | Intake, electronic windrows, cable recovery, reclamation | [View 1](course-v2-3.png) · [View 2](course-v2-3-sector.png) |

Bliss was compared again with the installed wallpaper: discarded electronics
belong in a sunny reclaimed meadow, with working machinery and breaks of
vegetation. Its road shoulders now expose larger equipment terraces, more
monitor/cable clusters and worn soil. City equipment sits on occupied buildings
rather than unsupported platform-like structures. Large equipment uses grounded
foundations and track-clearance checks.

Brawl also receives the shared material fixes and authored scenery:
[Prism Terminal](brawl-v2-0.png), [Reactor Garden](brawl-v2-1.png),
[Afterglow Rooftop](brawl-v2-2.png), [Bliss Salvage Yard](brawl-v2-3.png).
Garden glasshouses, rooftop ventilation and salvage-processing equipment sit
behind the combat plane. Platform collision heights are unchanged.

## Texture and geometry corrections

- New image-generated neutral enamel, quieter meadow ground and reclaimed
  timber replace the overly rusty/bright all-purpose treatment. Exact prompts,
  output paths and built-in generation mode are in [asset notes](../assets/ART-V2.md).
- Physical-scale face UVs keep scratches from stretching. Each plank's long axis
  controls its wood grain; legacy PCB UVs repeat within their own atlas tile.
- Ducké and Axi's nose decals conform to the curved body, and sloped hoods use
  slope-matched decal geometry. Alpha stencils no longer render opaque backing
  plates or cast rectangular shadows in Kart.
- Material classes distinguish aged ABS, enamel, rubber, wood, bare metal and
  prototype paint. Paint bump amplitude was reduced after close-up inspection.
- Eight editable livery UV guides accompany the SVG/PNG atlas. Bumper lettering
  compensates for the long physical plate aspect ratio.
- Blender export filters degenerate faces and preserves normalized normals;
  the art regression test caught malformed thin bevels before the final export.
- Brawl's deck core and tread formerly had coplanar top faces. Separating them
  removes striped depth-buffer flicker without changing the landing surfaces;
  a regression assertion now covers every platform in every arena.

## Verification

**8/8 release and 8/8 AddressSanitizer/UndefinedBehaviorSanitizer CTest targets
pass.** [Release log](ctest-art-v2.log) · [Sanitizer log](ctest-art-v2-sanitizers.log).
The existing simulation suites execute 1,055,242 Brawl assertions and 1,122,900
Kart assertions. Additional targets cover player input, controllers and art.

The new art target checks all 28 authored meshes, finite geometry/UVs, normalized
normals, material bounds, padded livery coordinates, separate body meshes,
identical wheel geometry, smaller drivers, profiles and per-course prop variety.
Visual inspection covers all eight front/rear combinations and both views of
each course; image review is not replaced by triangle-count checks.

Native event replays passed:

- [Four-race Cup](replay-art-v2-cup.log): race completion, accumulated standings,
  next courses, final classification and a new Cup.
- [Four-player](replay-art-v2-split.log): 1080p, resize to 720p and disconnect pause.
- [Three-player](replay-art-v2-three-player.log): keyboard + virtual Xbox + virtual
  PlayStation; setup, gameplay and pause.
- Rapid complete taps at 10 FPS in [Kart](replay-art-v2-rapid-qinda-prism-kart.log)
  and [Brawl](replay-art-v2-rapid-qinda-mega-brawl.log).
- [Brawl flow](replay-art-v2-brawl.log): setup, movement/attacks, pause and restart.

Controller checks use SDL virtual devices; no physical Xbox, PlayStation or
third-party controller was attached. Hardware vibration, Bluetooth behavior and
unusual adapters still require physical testing. Replay timing is functional
evidence, not a performance benchmark.

All builds used the actual `portageq envvar MAKEOPTS` value (`-j24 -l24`) without
overriding it. The isolated Blender MCP process was closed after both editable
libraries and interchange exports had been saved. Existing Blender documents,
system package settings and original screensaver binaries were not changed.

## Reproduce a view

```sh
./play-kart --garage --character 1
SDL_VIDEODRIVER=offscreen ./play-kart --garage --character 7 \
  --garage-angle 3.14159 --no-hud --size 1440x900 --mute --snapshot /tmp/axi.png
SDL_VIDEODRIVER=offscreen ./play-kart --demo --course bliss --start 38 \
  --size 1600x900 --mute --snapshot /tmp/bliss.png
ctest --test-dir build --output-on-failure
```

This is an implemented art and asset-pipeline revision, not finished commercial
AA/AAA production. Bespoke character rigs/animation, richer vegetation and
landmark architecture, further lighting/art polish, physical-pad playtesting
and deeper game balance remain production work. No online multiplayer was added.
