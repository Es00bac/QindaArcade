# Reclaimed-world asset notes

## Reference

`references/qinda-bliss.png` is a reference copy of the locally installed
`/usr/share/qindaqt/wallpapers/qinda-bliss.png`. It was inspected for the rolling
meadow, discarded monitors/electronics, distant city, sky and character-world
relationship. It is not used as a flat in-game backdrop. Its original ownership
and licensing remain unchanged.

## Image-generated texture

Saved asset: `textures/reclaimed-atlas.png`.
The tool returned a 1254 × 1254 PNG (four 627 × 627 material tiles); the prompt
requested 2048 × 2048. The saved file has not been upscaled.
Generated with the **built-in image-generation tool**, not the API/CLI fallback.
The image-generation skill guided a diffuse-only four-material atlas with no
logos or baked perspective. The renderer uploads the four quadrants separately
with independent repeating mipmaps to prevent cross-material bleeding. Grass
uses multi-scale sampling; metal uses world-space triplanar projection to keep
rust at a consistent scale on beams and small props.

Generation prompt/specification:

> Use case: stylized-concept. Production base-color texture atlas for a polished
> stylized 3D racing game in a sunny e-waste junkyard reclaimed by meadow. One
> square 2048 × 2048, exactly four equal edge-to-edge quadrants, no gutters.
> Top left: seamless dense mossy meadow grass, clover and tiny yellow flowers,
> olive and spring green with earthy pockets, no large objects. Top right:
> seamless worn pale cream painted machinery metal, flaking paint, copper-brown
> rust, scratches, exposed brushed steel and subtle moss, no objects. Bottom
> left: aged dark green PCB, copper traces, small chips, silver solder and dirt,
> no lettering. Bottom right: seamless weathered gray asphalt/compacted gravel,
> small cracks and fine stone aggregate, muted warm gray, no markings.
> Orthographic top-down flat scans, diffuse lighting, no baked shadows,
> perspective, horizon, labels, text, logos or watermark. Tactile realistic
> detail with restrained painterly AA stylization; tile each quadrant
> independently.

The saved PNG is the selected generated artifact; there is no runtime image API.

## Blender MCP assets

`models/reclaimed-world.blend` is an editable assembled prop library with packed
textures; `models/reclaimed-world.glb` is its interchange export.
`models/manifest.json` records triangle counts. Both games load the actual
exported `.qmesh` vertex/normal/UV data at runtime.

The kit contains CRT monitors with inset glass and grille details, server racks
with exposed boards, cable coils, a braced salvage gantry with hanging hook,
individual-leaflet ferns and wind-powered reclamation pumps. It was authored in
Blender 4.5.5 LTS through the existing Blender MCP 1.5 local socket protocol,
inside a separate factory-startup session. No existing Blender document or
preferences were overwritten.

Authoring sources: `../tools/build_environment_assets.py`,
`../tools/blender_bootstrap.py`, and `../tools/blender_client.py`. The bootstrap
uses the existing addon at
`/home/cabewse/work_SPaC3/D3scent/tools/blender_mcp/blender_mcp.py` and listens
on loopback port 9877. The builder clears its authoring scene, so only run it in
the isolated factory-startup session, never inside an unrelated open document.

## Controller data and icons

`gamecontrollerdb.txt` was retrieved from the primary
[SDL_GameControllerDB repository](https://github.com/mdqinc/SDL_GameControllerDB)
on 2026-09-19. Its license is included as `SDL_GameControllerDB-LICENSE.txt`.
Device handling uses [SDL2 GameController](https://wiki.libsdl.org/SDL2/CategoryGameController).
The application icons are the original local Prism Brawl and Prism Circuit SVGs.
