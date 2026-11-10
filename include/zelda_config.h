#ifndef __ZELDA_CONFIG_H__
#define __ZELDA_CONFIG_H__

#include <filesystem>
#include <string_view>
#include "ultramodern/config.hpp"
#include "recomp_input.h"

namespace zelda64 {
    constexpr std::u8string_view program_id = u8"Zelda64Recompiled";
    constexpr std::string_view program_name = "Zelda 64: Recompiled";

    // TODO: Move loading configs to the runtime once we have a way to allow per-project customization.
    void load_config();
    void save_config();
    
    void reset_input_bindings();
    void reset_cont_input_bindings();
    void reset_kb_input_bindings();
    void reset_single_input_binding(recomp::InputDevice device, recomp::GameInput input);

    std::filesystem::path get_app_folder_path();
    
    bool get_debug_mode_enabled();
    void set_debug_mode_enabled(bool enabled);
    
    enum class AutosaveMode {
        On,
        Off,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::AutosaveMode, {
        {zelda64::AutosaveMode::On, "On"},
        {zelda64::AutosaveMode::Off, "Off"}
    });

    enum class TargetingMode {
        Switch,
        Hold,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::TargetingMode, {
        {zelda64::TargetingMode::Switch, "Switch"},
        {zelda64::TargetingMode::Hold, "Hold"}
    });

    TargetingMode get_targeting_mode();
    void set_targeting_mode(TargetingMode mode);

    enum class CameraInvertMode {
        InvertNone,
        InvertX,
        InvertY,
        InvertBoth,
        OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::CameraInvertMode, {
        {zelda64::CameraInvertMode::InvertNone, "InvertNone"},
        {zelda64::CameraInvertMode::InvertX, "InvertX"},
        {zelda64::CameraInvertMode::InvertY, "InvertY"},
        {zelda64::CameraInvertMode::InvertBoth, "InvertBoth"}
    });

    CameraInvertMode get_camera_invert_mode();
    void set_camera_invert_mode(CameraInvertMode mode);

    CameraInvertMode get_analog_camera_invert_mode();
    void set_analog_camera_invert_mode(CameraInvertMode mode);

    enum class AnalogCamMode {
        On,
        Off,
		OptionCount
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(zelda64::AnalogCamMode, {
        {zelda64::AnalogCamMode::On, "On"},
        {zelda64::AnalogCamMode::Off, "Off"}
    });

    AutosaveMode get_autosave_mode();
    void set_autosave_mode(AutosaveMode mode);

    AnalogCamMode get_analog_cam_mode();
    void set_analog_cam_mode(AnalogCamMode mode);

    void open_quit_game_prompt();

    // Settings for rendering to a SteamVR headset. Stored in vr.json and read when the renderer starts, so changes
    // to them require restarting the game.
    struct VRSettings {
        // Attempt to render to a SteamVR headset. The game falls back to the regular window if no headset is found.
        bool enabled = true;
        // How big the world feels. 1.0 is roughly life size, higher values make the world feel smaller (like a diorama)
        // and increase both the stereo depth and how far the camera moves when moving your head.
        float world_scale = 1.0f;
        // Distance in meters and horizontal size in degrees of the floating panel that shows the HUD, text and menus.
        float hud_distance = 1.5f;
        float hud_size = 55.0f;
        // Remove the game camera's pitch and roll so the horizon always stays level. Can be more comfortable.
        bool level_horizon = false;
        // Hide the black bars shown at the top and bottom of the screen during cutscenes.
        bool hide_letterbox = true;
        // Multiplier for the resolution recommended by SteamVR.
        float render_scale = 1.0f;
    };

    const VRSettings& get_vr_settings();
    void set_vr_settings(const VRSettings& settings);
};

#endif
