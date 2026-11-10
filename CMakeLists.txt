cmake_minimum_required(VERSION 3.20)

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum OS X deployment version")
endif()

project(Zelda64Recompiled LANGUAGES C CXX)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything /W4")
endif()

# Opt out of constexpr mutex constructor on windows to prevent vcredist issues
if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
endif()

if (APPLE)
    enable_language(OBJC OBJCXX)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(RECOMP_FLATPAK "Configure the build for Flatpak compatibility." OFF)
endif()

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (WIN32)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/lib/")
endif()

set(RT64_STATIC TRUE)
set(RT64_SDL_WINDOW_VULKAN TRUE)
add_compile_definitions(HLSL_CPU)

if (RECOMP_FLATPAK)
    add_compile_definitions(RECOMP_FLATPAK)
endif()

# VR support through OpenVR (SteamVR). SteamVR isn't available on macOS, so it's disabled there by default. It's also
# disabled by default for Flatpak builds, as those are built without network access and OpenVR is downloaded during configuration.
if (APPLE OR RECOMP_FLATPAK)
    set(ZELDA64_VR_DEFAULT OFF)
else()
    set(ZELDA64_VR_DEFAULT ON)
endif()

option(ZELDA64_VR "Build with support for SteamVR headsets through OpenVR." ${ZELDA64_VR_DEFAULT})

if (ZELDA64_VR)
    # Only the few source files needed to build the OpenVR loader are downloaded, as the full repository is very large
    # (it includes prebuilt binaries and samples). Every file is pinned to a release and verified with its hash.
    # OpenVR's own CMake project modifies global compiler flags, so the 'openvr_api' library is defined here instead.
    set(OPENVR_VERSION "v2.5.1")
    set(OPENVR_URL "https://raw.githubusercontent.com/ValveSoftware/openvr/${OPENVR_VERSION}")
    set(openvr_SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/openvr-${OPENVR_VERSION}")
    set(OPENVR_FILES
        "LICENSE" f56ff606104d4ef18e617921a75c73ad73b5a1a1d70c69590c29de16919e04ad
        "headers/openvr.h" 94e5545370159c85f87cd6e15dd3739f7c919fc7a6e869f5e4ed463533a07ed0
        "src/ivrclientcore.h" 07c8ce981a59fb7cd1dc30572f0c9972d98e6275e95527f0f7a74a18bc0cf846
        "src/json/json.h" dc678ddb2f21936f7e778a90b4579150dfb198a7d9adff5da0de789297757c71
        "src/json/json-forwards.h" 74a06407028e0d79735d4199f194f4b83df123153610e521a0305ce25a84e664
        "src/jsoncpp.cpp" bd8df7509444c8ac9bcc0559e1ae28dfb20f614a14155cdd3807987be224f091
        "src/openvr_api_public.cpp" fcad852f6b8caba8e4285067ea3f0430f0e5be22ffc42133a8d6bb6b73d9bede
        "src/vrcore/dirtools_public.cpp" ee39fa2e8739e916edd71566e22b64108c78d6841b09190cd03593960120adca
        "src/vrcore/dirtools_public.h" d15d3c6e6c7b7fc62f7b46e93fb5214bc8fe7ad6a0dfdbc42cda9890e1735188
        "src/vrcore/envvartools_public.cpp" 732c976825872e951117c4cea746b553c6455eca924efe47b3de5437b56c7516
        "src/vrcore/envvartools_public.h" 09fb43f6a56a89ea90c5f48dff971e9326b7e13280d35a3f6a7e9a545aa6e14c
        "src/vrcore/hmderrors_public.cpp" a3c4f5e23d5c665a41e3d63a8d9ad76949a1f33e203d66eca8129ad8e19e399c
        "src/vrcore/hmderrors_public.h" 84c01d425383973210978452d0f8873f3eb181bdf04a34a5b26c09bc7821d7ae
        "src/vrcore/pathtools_public.cpp" 5ef435287b1f4462e13a69e21c654944eeecaeacea9b97cd84f908d4c02598a1
        "src/vrcore/pathtools_public.h" cc5866a4006efe7b216fd6163a8c1910db0d34747bde8937dd09d15f0ad24d66
        "src/vrcore/sharedlibtools_public.cpp" 55c1b734352bdd5c81bb24b0550582f352394d95790b78d863fba9aa18fd1ac4
        "src/vrcore/sharedlibtools_public.h" 28854bffca6a8621c8be2059d206a38f888d5195b7600c5ef1b1266dda73e5ca
        "src/vrcore/strtools_public.cpp" eb114b4f41b8bc8dfe52a01f74762a71e3ffbb11a8ffd56efdfbcdc2b6f5ad5d
        "src/vrcore/strtools_public.h" 34a2b9445522008df991cb57c7e9a987f5c0d4ea853cbe745a43842bb71100b5
        "src/vrcore/vrpathregistry_public.cpp" d7ce9a3ed06176c50df46295d8a09dc8758583dc53f351439bdc7c4b0d9ca839
        "src/vrcore/vrpathregistry_public.h" 9742a132b1a1287abd8157ef19ad8e022fe0899f6921a25b388ba2ae1e8656b3
    )

    list(LENGTH OPENVR_FILES OPENVR_FILES_LENGTH)
    math(EXPR OPENVR_FILES_LAST "${OPENVR_FILES_LENGTH} - 1")
    foreach(OPENVR_FILE_INDEX RANGE 0 ${OPENVR_FILES_LAST} 2)
        math(EXPR OPENVR_HASH_INDEX "${OPENVR_FILE_INDEX} + 1")
        list(GET OPENVR_FILES ${OPENVR_FILE_INDEX} OPENVR_FILE)
        list(GET OPENVR_FILES ${OPENVR_HASH_INDEX} OPENVR_HASH)

        # The download is skipped if the file already exists and its hash matches.
        file(DOWNLOAD "${OPENVR_URL}/${OPENVR_FILE}" "${openvr_SOURCE_DIR}/${OPENVR_FILE}"
            EXPECTED_HASH SHA256=${OPENVR_HASH}
            TLS_VERIFY ON
            STATUS OPENVR_DOWNLOAD_STATUS
        )

        list(GET OPENVR_DOWNLOAD_STATUS 0 OPENVR_DOWNLOAD_CODE)
        if (NOT OPENVR_DOWNLOAD_CODE EQUAL 0)
            message(FATAL_ERROR "Failed to download OpenVR file ${OPENVR_FILE}: ${OPENVR_DOWNLOAD_STATUS}. Configure with -DZELDA64_VR=OFF to build without VR support.")
        endif()
    endforeach()

    add_library(openvr_api STATIC
        ${openvr_SOURCE_DIR}/src/openvr_api_public.cpp
        ${openvr_SOURCE_DIR}/src/jsoncpp.cpp
        ${openvr_SOURCE_DIR}/src/vrcore/dirtools_public.cpp
        ${openvr_SOURCE_DIR}/src/vrcore/envvartools_public.cpp
        ${openvr_SOURCE_DIR}/src/vrcore/pathtools_public.cpp
        ${openvr_SOURCE_DIR}/src/vrcore/sharedlibtools_public.cpp
        ${openvr_SOURCE_DIR}/src/vrcore/hmderrors_public.cpp
        ${openvr_SOURCE_DIR}/src/vrcore/vrpathregistry_public.cpp
        ${openvr_SOURCE_DIR}/src/vrcore/strtools_public.cpp
    )

    target_include_directories(openvr_api
        PUBLIC ${openvr_SOURCE_DIR}/headers
        PRIVATE ${openvr_SOURCE_DIR}/src ${openvr_SOURCE_DIR}/src/vrcore
    )

    target_compile_definitions(openvr_api
        PUBLIC OPENVR_BUILD_STATIC
        PRIVATE VR_API_PUBLIC VRCORE_NO_PLATFORM
    )

    # OpenVR's sources rely on <codecvt>, which is deprecated in C++17 and later.
    set_target_properties(openvr_api PROPERTIES CXX_STANDARD 17)

    if (WIN32)
        target_compile_definitions(openvr_api PRIVATE WIN64 _CRT_SECURE_NO_WARNINGS _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_compile_definitions(openvr_api PRIVATE LINUX POSIX)
        if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            target_compile_definitions(openvr_api PRIVATE LINUXARM64)
        else()
            target_compile_definitions(openvr_api PRIVATE LINUX64)
        endif()
    elseif (APPLE)
        target_compile_definitions(openvr_api PRIVATE OSX POSIX)
        set_source_files_properties(
            ${openvr_SOURCE_DIR}/src/vrcore/pathtools_public.cpp
            ${openvr_SOURCE_DIR}/src/vrcore/vrpathregistry_public.cpp
            PROPERTIES COMPILE_FLAGS "-x objective-c++"
        )
        target_link_libraries(openvr_api PRIVATE "-framework Foundation")
    endif()

    target_link_libraries(openvr_api PRIVATE ${CMAKE_DL_LIBS})
    set(RT64_OPENVR ON)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/rt64 ${CMAKE_BINARY_DIR}/rt64)

# set(BUILD_SHARED_LIBS_SAVED "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
SET(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/lunasvg)
# set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_SAVED}")
SET(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
SET(RMLUI_TESTS_ENABLED OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/RmlUi)
target_compile_definitions(rmlui_core PRIVATE LUNASVG_BUILD_STATIC)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime)

target_include_directories(rt64 PRIVATE ${CMAKE_BINARY_DIR}/rt64/src)

# RecompiledFuncs - Library containing the primary recompiler output
add_library(RecompiledFuncs STATIC)

target_compile_options(RecompiledFuncs PRIVATE
    # -Wno-unused-but-set-variable
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)

target_include_directories(RecompiledFuncs PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/ultramodern/include
    ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/librecomp/include
    ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/N64Recomp/include
)

file(GLOB FUNC_C_SOURCES ${CMAKE_SOURCE_DIR}/RecompiledFuncs/*.c)
file(GLOB FUNC_CXX_SOURCES ${CMAKE_SOURCE_DIR}/RecompiledFuncs/*.cpp)

target_sources(RecompiledFuncs PRIVATE ${FUNC_C_SOURCES} ${FUNC_CXX_SOURCES})

# PatchesLib - Library containing the recompiled output for any custom function patches
add_library(PatchesLib STATIC)

target_compile_options(PatchesLib PRIVATE
    # -Wno-unused-but-set-variable
    -fno-strict-aliasing
    -Wno-unused-variable
    -Wno-implicit-function-declaration
)

target_include_directories(PatchesLib PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/ultramodern/include
    ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/librecomp/include
    ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/N64Recomp/include
)

target_sources(PatchesLib PRIVATE
    ${CMAKE_SOURCE_DIR}/RecompiledPatches/patches.c
    ${CMAKE_SOURCE_DIR}/RecompiledPatches/patches_bin.c
)

set_source_files_properties(${CMAKE_SOURCE_DIR}/RecompiledPatches/patches.c PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)

# Build patches elf
if(NOT DEFINED PATCHES_C_COMPILER)
    set(PATCHES_C_COMPILER clang)
endif()

if(NOT DEFINED PATCHES_LD)
    set(PATCHES_LD ld.lld)
endif()

add_custom_target(PatchesBin
    COMMAND ${CMAKE_COMMAND} -E env CC=${PATCHES_C_COMPILER} LD=${PATCHES_LD} make
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/patches
    BYPRODUCTS ${CMAKE_SOURCE_DIR}/patches/patches.elf
)

# Generate patches_bin.c from patches.bin
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/RecompiledPatches/patches_bin.c
    COMMAND file_to_c ${CMAKE_SOURCE_DIR}/patches/patches.bin mm_patches_bin ${CMAKE_SOURCE_DIR}/RecompiledPatches/patches_bin.c ${CMAKE_SOURCE_DIR}/RecompiledPatches/patches_bin.h
    DEPENDS ${CMAKE_SOURCE_DIR}/patches/patches.bin
)

# Recompile patches elf into patches.c and patches.bin
add_custom_command(OUTPUT
                       ${CMAKE_SOURCE_DIR}/patches/patches.bin
                       ${CMAKE_SOURCE_DIR}/RecompiledPatches/patches.c
                       ${CMAKE_SOURCE_DIR}/RecompiledPatches/recomp_overlays.inl
                       ${CMAKE_SOURCE_DIR}/RecompiledPatches/funcs.h
                   # TODO: Look into why modifying patches requires two builds to take
                   COMMAND ./N64Recomp patches.toml
                   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                   DEPENDS ${CMAKE_SOURCE_DIR}/patches/patches.elf
)

# Main executable
add_executable(Zelda64Recompiled)

set (SOURCES
    ${CMAKE_SOURCE_DIR}/src/main/main.cpp
    ${CMAKE_SOURCE_DIR}/src/main/support.cpp
    ${CMAKE_SOURCE_DIR}/src/main/register_overlays.cpp
    ${CMAKE_SOURCE_DIR}/src/main/register_patches.cpp
    ${CMAKE_SOURCE_DIR}/src/main/rt64_render_context.cpp

    ${CMAKE_SOURCE_DIR}/src/game/input.cpp
    ${CMAKE_SOURCE_DIR}/src/game/controls.cpp
    ${CMAKE_SOURCE_DIR}/src/game/config.cpp
    ${CMAKE_SOURCE_DIR}/src/game/scene_table.cpp
    ${CMAKE_SOURCE_DIR}/src/game/debug.cpp
    ${CMAKE_SOURCE_DIR}/src/game/quicksaving.cpp
    ${CMAKE_SOURCE_DIR}/src/game/recomp_api.cpp
    ${CMAKE_SOURCE_DIR}/src/game/recomp_actor_api.cpp
    ${CMAKE_SOURCE_DIR}/src/game/recomp_data_api.cpp
    ${CMAKE_SOURCE_DIR}/src/game/rom_decompression.cpp

    ${CMAKE_SOURCE_DIR}/src/ui/ui_renderer.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_state.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_launcher.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_config.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_prompt.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_config_sub_menu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_color_hack.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_rml_hacks.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_elements.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_mod_details_panel.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_mod_installer.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_mod_menu.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_api.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_api_events.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_api_images.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/ui_utils.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/util/hsv.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/core/ui_context.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_button.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_clickable.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_container.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_element.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_image.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_label.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_radio.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_scroll_container.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_slider.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_span.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_style.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_text_input.cpp
    ${CMAKE_SOURCE_DIR}/src/ui/elements/ui_toggle.cpp

    ${CMAKE_SOURCE_DIR}/rsp/aspMain.cpp
    ${CMAKE_SOURCE_DIR}/rsp/njpgdspMain.cpp

    ${CMAKE_SOURCE_DIR}/lib/RmlUi/Backends/RmlUi_Platform_SDL.cpp
)

if (APPLE)
    list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/src/main/support_apple.mm)
endif()

target_include_directories(Zelda64Recompiled PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/N64ModernRuntime/N64Recomp/include
    ${CMAKE_SOURCE_DIR}/lib/concurrentqueue
    ${CMAKE_SOURCE_DIR}/lib/GamepadMotionHelpers
    ${CMAKE_SOURCE_DIR}/lib/RmlUi/Include
    ${CMAKE_SOURCE_DIR}/lib/RmlUi/Backends
    ${CMAKE_SOURCE_DIR}/lib/rt64/src/contrib
    ${CMAKE_SOURCE_DIR}/lib/rt64/src/contrib/hlslpp/include
    ${CMAKE_SOURCE_DIR}/lib/rt64/src/contrib/dxc/inc
    ${CMAKE_SOURCE_DIR}/lib/rt64/src
    ${CMAKE_SOURCE_DIR}/lib/rt64/src/rhi
    ${CMAKE_SOURCE_DIR}/lib/rt64/src/render
    ${CMAKE_SOURCE_DIR}/lib/freetype-windows-binaries/include
    ${CMAKE_SOURCE_DIR}/lib/rt64/src/contrib/nativefiledialog-extended/src/include
    ${CMAKE_SOURCE_DIR}/lib/SlotMap
    ${CMAKE_BINARY_DIR}/shaders
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
    target_compile_options(Zelda64Recompiled PRIVATE
        -march=nehalem
        -fno-strict-aliasing
        -fms-extensions
    )
else()
    target_compile_options(Zelda64Recompiled PRIVATE
        -fno-strict-aliasing
        -fms-extensions
    )
endif()

if (MSVC)
    # Disable identical code folding, since this breaks mod function patching as multiple functions can get merged into one.
    target_link_options(Zelda64Recompiled PRIVATE /OPT:NOICF)
elseif (APPLE)
    # Use a wrapper around ld64 that respects segprot's `max_prot` value in order
    # to make our executable memory writable (required for mod function patching)
    target_link_options(Zelda64Recompiled PRIVATE
        "-fuse-ld=${CMAKE_SOURCE_DIR}/.github/macos/ld64"
    )
endif()

if (WIN32)
    include(FetchContent)

    if (DEFINED ENV{SDL2_VERSION})
        set(SDL2_VERSION $ENV{SDL2_VERSION})
    else()
        set(SDL2_VERSION "2.30.3")
    endif()

    # Fetch SDL2 on windows
    FetchContent_Declare(
        sdl2
        URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-devel-${SDL2_VERSION}-VC.zip
    )
    FetchContent_MakeAvailable(sdl2)
    target_include_directories(Zelda64Recompiled PRIVATE
        ${sdl2_SOURCE_DIR}/include
    )
    target_link_directories(Zelda64Recompiled PRIVATE
        ${sdl2_SOURCE_DIR}/lib/x64
    )

    # Copy SDL2 and dxc DLLs to output folder as post build step
    add_custom_command(TARGET Zelda64Recompiled POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${sdl2_SOURCE_DIR}/lib/x64/SDL2.dll"
            "${CMAKE_SOURCE_DIR}/lib/rt64/src/contrib/dxc/bin/x64/dxil.dll"
            "${CMAKE_SOURCE_DIR}/lib/rt64/src/contrib/dxc/bin/x64/dxcompiler.dll"
            $<TARGET_FILE_DIR:Zelda64Recompiled>)

    set_target_properties(
        Zelda64Recompiled
        PROPERTIES
            LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
            LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
            LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
            LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )

    target_sources(Zelda64Recompiled PRIVATE ${CMAKE_SOURCE_DIR}/icons/app.rc)
    target_link_libraries(Zelda64Recompiled PRIVATE SDL2 Winmm.lib)
endif()

if (APPLE)
    find_package(SDL2 REQUIRED)
    target_include_directories(Zelda64Recompiled PRIVATE ${SDL2_INCLUDE_DIRS})

    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)

    target_link_libraries(Zelda64Recompiled PRIVATE ${CMAKE_DL_LIBS} Threads::Threads SDL2::SDL2)

    include(${CMAKE_SOURCE_DIR}/.github/macos/apple_bundle.cmake)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(SDL2 REQUIRED)
    add_compile_definitions("RT64_SDL_WINDOW_VULKAN")

    # Generate icon_bytes.c from the app icon PNG.
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon_bytes.c ${CMAKE_CURRENT_BINARY_DIR}/icon_bytes.h
        COMMAND file_to_c ${CMAKE_SOURCE_DIR}/icons/512.png icon_bytes ${CMAKE_CURRENT_BINARY_DIR}/icon_bytes.c ${CMAKE_CURRENT_BINARY_DIR}/icon_bytes.h
        DEPENDS ${CMAKE_SOURCE_DIR}/icons/512.png
    )
    target_sources(Zelda64Recompiled PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/icon_bytes.c)

    message(STATUS "SDL2_FOUND = ${SDL2_FOUND}")
    message(STATUS "SDL2_INCLUDE_DIRS = ${SDL2_INCLUDE_DIRS}")

    target_include_directories(Zelda64Recompiled PRIVATE ${SDL2_INCLUDE_DIRS})

    find_package(Freetype REQUIRED)

    message(STATUS "FREETYPE_FOUND = ${FREETYPE_FOUND}")
    message(STATUS "FREETYPE_INCLUDE_DIRS = ${FREETYPE_INCLUDE_DIRS}")
    message(STATUS "FREETYPE_LIBRARIES = ${FREETYPE_LIBRARIES}")

    include_directories(${FREETYPE_LIBRARIES})
    target_link_libraries(Zelda64Recompiled PRIVATE ${FREETYPE_LIBRARIES} SDL2::SDL2)

    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)

    target_link_libraries(Zelda64Recompiled PRIVATE "-latomic -static-libstdc++" ${CMAKE_DL_LIBS} Threads::Threads)
endif()

target_link_libraries(Zelda64Recompiled PRIVATE
    PatchesLib
    RecompiledFuncs
    librecomp
    ultramodern
    rt64
    RmlUi::Core
    RmlUi::Debugger
    nfd
    lunasvg
)

# TODO fix the rt64 CMake script so that this doesn't need to be duplicated here
# For DXC
set (DXC_COMMON_OPTS "-I${PROJECT_SOURCE_DIR}/src")
set (DXC_DXIL_OPTS "-Wno-ignored-attributes")
set (DXC_SPV_OPTS "-spirv" "-fspv-target-env=vulkan1.0" "-fvk-use-dx-layout")
set (DXC_PS_OPTS "${DXC_COMMON_OPTS}" "-E" "PSMain" "-T ps_6_0" "-D DYNAMIC_RENDER_PARAMS")
set (DXC_VS_OPTS "${DXC_COMMON_OPTS}" "-E" "VSMain" "-T vs_6_0" "-D DYNAMIC_RENDER_PARAMS" "-fvk-invert-y")
set (DXC_CS_OPTS "${DXC_COMMON_OPTS}" "-E" "CSMain" "-T cs_6_0")
set (DXC_GS_OPTS "${DXC_COMMON_OPTS}" "-E" "GSMain" "-T gs_6_0")
set (DXC_RT_OPTS "${DXC_COMMON_OPTS}" "-D" "RT_SHADER" "-T" "lib_6_3" "-fspv-target-env=vulkan1.1spirv1.4" "-fspv-extension=SPV_KHR_ray_tracing" "-fspv-extension=SPV_EXT_descriptor_indexing")

if (${WIN32})
    set (DXC "${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/dxc/bin/x64/dxc.exe")
    add_compile_definitions(NOMINMAX)
else()
    if (APPLE)
        # Apple's binary is universal, so it'll work on both x86_64 and arm64
        set (DXC "DYLD_LIBRARY_PATH=${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/dxc/lib/arm64" "${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/dxc/bin/arm64/dxc-macos")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
            set(SPIRVCROSS "DYLD_LIBRARY_PATH=${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/spirv-cross/lib/x64" "${PROJECT_SOURCE_DIR}/lib/rt64//src/contrib/spirv-cross/bin/x64/spirv-cross")
        else()
            set(SPIRVCROSS "DYLD_LIBRARY_PATH=${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/spirv-cross/lib/arm64" "${PROJECT_SOURCE_DIR}/lib/rt64//src/contrib/spirv-cross/bin/x64/spirv-cross")
        endif()
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
            set (DXC "LD_LIBRARY_PATH=${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/dxc/lib/x64" "${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/dxc/bin/x64/dxc-linux")
        else()
            set (DXC "LD_LIBRARY_PATH=${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/dxc/lib/arm64" "${PROJECT_SOURCE_DIR}/lib/rt64/src/contrib/dxc/bin/arm64/dxc-linux")
        endif()
    endif()
endif()

build_vertex_shader(Zelda64Recompiled "shaders/InterfaceVS.hlsl" "shaders/InterfaceVS.hlsl")
build_pixel_shader(Zelda64Recompiled "shaders/InterfacePS.hlsl" "shaders/InterfacePS.hlsl")

# Embed all .nrm files in the "mods" directory
file(GLOB NRM_FILES "${CMAKE_SOURCE_DIR}/mods/*.nrm")

set(GENERATED_NRM_SOURCES "")

foreach(NRM_FILE ${NRM_FILES})
    get_filename_component(NRM_NAME ${NRM_FILE} NAME_WE)
    set(OUT_C "${CMAKE_CURRENT_BINARY_DIR}/mods/${NRM_NAME}.c")
    set(OUT_H "${CMAKE_CURRENT_BINARY_DIR}/mods/${NRM_NAME}.h")

    add_custom_command(
        OUTPUT ${OUT_C} ${OUT_H}
        COMMAND file_to_c ${NRM_FILE} ${NRM_NAME} ${OUT_C} ${OUT_H}
        DEPENDS ${NRM_FILE}
    )

    list(APPEND GENERATED_NRM_SOURCES ${OUT_C})
endforeach()

target_sources(Zelda64Recompiled PRIVATE ${GENERATED_NRM_SOURCES})

target_sources(Zelda64Recompiled PRIVATE ${SOURCES})

set_property(TARGET Zelda64Recompiled PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
