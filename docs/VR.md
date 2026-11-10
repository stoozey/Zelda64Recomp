# VR Edition (SteamVR)

This fork renders Majora's Mask in stereoscopic 3D on SteamVR headsets. It is **not** a first-person mode: the game's own
camera stays where it normally is (behind Link, in cutscenes, etc.) and acts as the anchor for your head. On top of that
view, your real head movement is applied, so you can look around, lean in and see the world from any angle in full 3D.

## Requirements

* Windows (D3D12 or Vulkan) or Linux (Vulkan) with SteamVR installed. SteamVR is not available on macOS, so VR support
  is not built there.
* A headset that works with SteamVR (Index, Vive, Quest through Link/Air Link/Virtual Desktop, WMR, Pimax, ...).

## Building

Follow [BUILDING.md](../BUILDING.md) as usual. VR support is enabled by default on Windows and Linux (`ZELDA64_VR`).
During the CMake configure step, the few source files needed to build the OpenVR loader are downloaded from
[ValveSoftware/openvr](https://github.com/ValveSoftware/openvr) (release v2.5.1, each file verified against a SHA-256
hash) and linked statically, so there is no extra DLL to ship.

* To build without VR (e.g. offline builds), configure with `-DZELDA64_VR=OFF`. Flatpak builds default to `OFF`.
* The VR renderer lives in the RT64 submodule (`lib/rt64`). Those changes are local to the submodule's working tree:
  commit them to a fork of RT64 and point `.gitmodules` at it, otherwise they'll be lost if the submodule is reset.

## Playing

1. Start SteamVR (or just start the game; it launches SteamVR when a headset is detected).
2. Launch the game. Use the desktop window for the launcher and the settings menu, as those are only drawn there.
3. Once the game starts, look at the headset. The view is centered on your head position when the game starts.

| Key | Action |
| --- | --- |
| F9 | Recenter: makes your current head position and heading the new anchor of the game camera. |

The desktop window keeps working as a mirror of the right eye (stretched, as the eye's field of view is mapped onto a
4:3 image).

## Settings (`vr.json`)

The file is created next to the other config files (`general.json`, `graphics.json`, ...) on first launch. Edit it while
the game is closed.

| Setting | Default | Description |
| --- | --- | --- |
| `enabled` | `true` | Use the headset when SteamVR and a headset are available. The game falls back to the regular window otherwise. |
| `world_scale` | `1.0` | How big the world feels. `1.0` is roughly life size. Higher values make the world feel smaller (like a diorama) and increase the stereo depth and how far the camera moves with your head. |
| `hud_distance` | `1.5` | Distance in meters of the floating panel that shows the HUD, text boxes and menus. |
| `hud_size` | `55.0` | Horizontal size of that panel in degrees. |
| `level_horizon` | `false` | Ignore the game camera's pitch and roll so the horizon is always level. Only your head tilts the view. Recommended if the camera's motion makes you uncomfortable. |
| `hide_letterbox` | `true` | Hide the black bars shown during cutscenes. |
| `render_scale` | `1.0` | Multiplier for the per-eye resolution recommended by SteamVR. Lower it if your GPU can't keep up. |

Other notes:

* The aspect ratio, resolution and framerate settings from the graphics menu are ignored while in VR: the image is
  rendered at the headset's resolution and refresh rate, using frame interpolation to go beyond the game's 20 FPS.
* High precision framebuffers are disabled in VR, as SteamVR doesn't accept that format.

## How it works

* **Game patches** (`patches/vr_patches.c` and friends) tag the main gameplay camera's projection with a dedicated matrix
  group ID (`CAMERA_WORLD_TRANSFORM_ID`). In VR they also stop the game from culling actors, grass and room geometry
  that are behind the camera, hide the letterbox, and disable motion blur (it blends in the previous frame, which would
  belong to the other eye).
* **RT64** (`lib/rt64/src/hle/rt64_vr.*`) renders every displayed frame twice, once per eye. For projections tagged as
  the world camera, the eye's view is built as `game view * inverse(head pose) * inverse(eye pose)` and the projection's
  horizontal and vertical terms are replaced with the headset's per-eye frustum, while the game's depth terms (near/far
  planes, fog) are kept. Frame interpolation still applies, so motion is smooth at 90 Hz and above.
* Everything else drawn to the screen (HUD, text boxes, pause menu, file select) is remapped onto a head-locked virtual
  panel by adjusting its viewport and scissor. Full-screen draws such as clears, fades, screen transitions and captured
  frames keep covering the whole eye.
* **OpenVR provider** (`lib/rt64/src/vr/rt64_vr_openvr.cpp`) handles pacing (`WaitGetPoses`), poses, recentering and
  submitting the eye images to the SteamVR compositor for D3D12 and Vulkan.

## Known limitations

* The game only renders what its own camera would normally render. Some objects that are spawned or updated based on
  what the camera sees (e.g. bushes that only spawn on screen) may be missing when looking far away from the camera's
  direction.
* Sprites and billboards (particles, some effects) face the game camera, not your eyes.
* Large game camera movements (cuts, fast pans) move your whole view. Enable `level_horizon` and keep `world_scale`
  near `1.0` if this is uncomfortable.
* The pause screen shows the captured frame across your whole view instead of in 3D.
* The launcher and settings menus are only shown on the desktop window.
* If SteamVR stops, the game keeps running on the desktop window. Restart the game to go back into VR.
