cmake_minimum_required(VERSION 3.16)

project(th06_psp LANGUAGES C CXX)

set(TH06_AUTOTEST_FRAMES 0 CACHE STRING "Exit automatically after this many rendered frames (test builds only)")
option(TH06_AUTOTEST_INPUT "Inject deterministic menu/gameplay input (test builds only)" OFF)
option(TH06_FORCE_SIDECAR_ASSETS "Read pre-extracted PBG3 sidecar assets (test builds only)" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(TH06_SOURCES
    src/AnmManager.cpp
    src/AsciiManager.cpp
    src/BombData.cpp
    src/BulletData.cpp
    src/BulletManager.cpp
    src/Chain.cpp
    src/Controller.cpp
    src/EclManager.cpp
    src/EffectManager.cpp
    src/Ending.cpp
    src/EnemyEclInstr.cpp
    src/EnemyManager.cpp
    src/FileSystem.cpp
    src/GameErrorContext.cpp
    src/GameManager.cpp
    src/GameWindow.cpp
    src/Gui.cpp
    src/ItemManager.cpp
    src/MainMenu.cpp
    src/MidiOutput.cpp
    src/MusicRoom.cpp
    src/Player.cpp
    src/PspDiagnostics.cpp
    src/ReplayManager.cpp
    src/ResultScreen.cpp
    src/Rng.cpp
    src/ScreenEffect.cpp
    src/SoundPlayer.cpp
    src/Stage.cpp
    src/Supervisor.cpp
    src/TextHelper.cpp
    src/ZunMath.cpp
    src/ZunTimer.cpp
    src/graphics/FixedFunctionGL.cpp
    src/graphics/GLFunc.cpp
    src/main.cpp
    src/midi/MidiDefault.cpp
    src/pbg3/FileAbstraction.cpp
    src/pbg3/IPbg3Parser.cpp
    src/pbg3/Pbg3Archive.cpp
    src/pbg3/Pbg3Parser.cpp
    src/thirdparty/sjis_converter.cpp
    src/utils.cpp
)

add_executable(th06_psp ${TH06_SOURCES})
target_include_directories(th06_psp PRIVATE src)

if(PSP)
    include(FindPkgConfig)
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
    pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf)

    target_include_directories(th06_psp PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
    )
    target_compile_definitions(th06_psp PRIVATE
        TH06_PSP=1
        GAME_WINDOW_WIDTH_REAL=480
        GAME_WINDOW_HEIGHT_REAL=272
    )
    if(TH06_AUTOTEST_FRAMES GREATER 0)
        target_compile_definitions(th06_psp PRIVATE TH06_AUTOTEST_FRAMES=${TH06_AUTOTEST_FRAMES})
    endif()
    if(TH06_AUTOTEST_INPUT)
        target_compile_definitions(th06_psp PRIVATE TH06_AUTOTEST_INPUT=1)
    endif()
    if(TH06_FORCE_SIDECAR_ASSETS)
        target_compile_definitions(th06_psp PRIVATE TH06_FORCE_SIDECAR_ASSETS=1)
    endif()
    target_compile_options(th06_psp PRIVATE
        -Os
        -G0
        -ffunction-sections
        -fdata-sections
        -fno-exceptions
        -fno-rtti
        -fno-unwind-tables
        -fno-asynchronous-unwind-tables
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
    )
    target_link_options(th06_psp PRIVATE
        -Wl,--gc-sections
        -Wl,-T,${CMAKE_SOURCE_DIR}/cmake/psp-gc-sections.ld
        -Wl,-Map,${CMAKE_BINARY_DIR}/th06_psp.map
    )
    target_link_libraries(th06_psp PRIVATE
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_LIBRARIES}
    )
    create_pbp_file(
        TARGET th06_psp
        ICON_PATH ${CMAKE_SOURCE_DIR}/assets/psp/ICON0.PNG
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE "Touhou 6 - EoSD PSP"
        VERSION 01.00
    )
else()
    message(FATAL_ERROR "This CMake build currently targets PSPDEV. Configure with psp-cmake.")
endif()
