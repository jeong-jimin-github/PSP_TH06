#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <cstdio>

#ifdef __PSP__
#include <psppower.h>
#endif

#include "AnmManager.hpp"
#include "Chain.hpp"
#include "FileSystem.hpp"
#include "GameErrorContext.hpp"
#include "GameWindow.hpp"
#include "PspDiagnostics.hpp"
#include "SoundPlayer.hpp"
#include "Stage.hpp"
#include "Supervisor.hpp"
#include "ZunResult.hpp"
#include "i18n.hpp"
#include "utils.hpp"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

#ifdef __PSP__
    // TH06's fixed 60 Hz simulation and software-side vertex preparation need
    // the PSP's full user-mode clock, especially on dense bullet patterns.
    scePowerSetClockFrequency(333, 333, 166);
    PspDiagnostics::ResetLoadTrace();
#endif

    i32 renderResult = 0;
    //    MSG msg;
    //    i32 waste1, waste2, waste3, waste4, waste5, waste6;

    //    if (utils::CheckForRunningGameInstance())
    //    {
    //        g_GameErrorContext.Flush();
    //
    //        return 1;
    //    }

    //    g_Supervisor.hInstance = hInstance;

    if (g_Supervisor.LoadConfig(TH_CONFIG_FILE) != ZUN_SUCCESS)
    {
        g_GameErrorContext.Flush();
        return -1;
    }
    PspDiagnostics::TraceStageLoad("config_complete");

    //    if (GameWindow::InitD3dInterface())
    //    {
    //        g_GameErrorContext.Flush();
    //        return 1;
    //    }

    //    SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &g_GameWindow.screenSaveActive, 0);
    //    SystemParametersInfo(SPI_GETLOWPOWERACTIVE, 0, &g_GameWindow.lowPowerActive, 0);
    //    SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &g_GameWindow.powerOffActive, 0);
    //    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0, NULL, SPIF_SENDCHANGE);

restart:
    GameWindow::CreateGameWindow();
    PspDiagnostics::TraceStageLoad("window_complete");

    g_AnmManager = new AnmManager();
    PspDiagnostics::TraceStageLoad("anm_manager_complete");

    if (GameWindow::InitD3dRendering() != ZUN_SUCCESS)
    {
        g_GameErrorContext.Flush();
        return 1;
    }
    PspDiagnostics::TraceStageLoad("renderer_complete");

    g_SoundPlayer.InitializeDSound();
    PspDiagnostics::TraceStageLoad("sound_device_complete");
    Controller::GetJoystickCaps();
    Controller::ResetKeyboard();
    PspDiagnostics::TraceStageLoad("input_complete");

    if (Supervisor::RegisterChain() != ZUN_SUCCESS)
    {
        goto stop;
    }
    PspDiagnostics::TraceStageLoad("supervisor_complete");
    if (!g_Supervisor.cfg.windowed)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }

    g_GameWindow.curFrame = 0;

    while (true)
    {
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                goto stop;
            }
        }

        renderResult = g_GameWindow.Render();
        if (renderResult != 0)
        {
            break;
        }

#ifdef TH06_AUTOTEST_FRAMES
        if (g_AutotestPresentedFrames >= TH06_AUTOTEST_FRAMES)
        {
            break;
        }
#endif

        //        SDL_Delay(1000.0f / 60.0f);

        //        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        //        {
        //            TranslateMessage(&msg);
        //            DispatchMessage(&msg);
        //        }
        //        else
        //        {
        //            testCoopLevelRes = g_Supervisor.d3dDevice->TestCooperativeLevel();
        //            if (testCoopLevelRes == D3D_OK)
        //            {
        //                renderResult = g_GameWindow.Render();
        //                if (renderResult != 0)
        //                {
        //                    goto stop;
        //                }
        //            }
        //            else if (testCoopLevelRes == D3DERR_DEVICENOTRESET)
        //            {
        //                g_AnmManager->ReleaseSurfaces();
        //                testResetRes = g_Supervisor.d3dDevice->Reset(&g_Supervisor.presentParameters);
        //                if (testResetRes != 0)
        //                {
        //                    goto stop;
        //                }
        //                GameWindow::InitD3dDevice();
        //                g_Supervisor.unk198 = 3;
        //            }
        //        }
    }

stop:
    g_Chain.Release();
    g_SoundPlayer.Release();

    delete g_AnmManager;
    g_AnmManager = NULL;

    if (g_GfxBackend != NULL)
        delete g_GfxBackend;
    SDL_Quit();

    if (renderResult == 2)
    {
        g_GameErrorContext.ResetContext();

        g_GameErrorContext.Log(TH_ERR_OPTION_CHANGED_RESTART);

        if (!g_Supervisor.cfg.windowed)
        {
            SDL_ShowCursor(SDL_ENABLE);
        }

        goto restart;
    }

    FileSystem::WriteDataToFile(TH_CONFIG_FILE, &g_Supervisor.cfg, sizeof(g_Supervisor.cfg));
    //    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, g_GameWindow.screenSaveActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, g_GameWindow.lowPowerActive, NULL, SPIF_SENDCHANGE);
    //    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, g_GameWindow.powerOffActive, NULL, SPIF_SENDCHANGE);

    SDL_ShowCursor(SDL_ENABLE);
    g_GameErrorContext.Flush();
    return 0;
}
