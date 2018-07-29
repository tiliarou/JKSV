#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <switch.h>

#include "gfx.h"
#include "data.h"
#include "ui.h"
#include "file.h"
#include "util.h"

extern "C"
{
    void userAppInit(void)
    {
        romfsInit();
        hidInitialize();
        nsInitialize();
        setsysInitialize();
        accountInitialize();
    }

    void userAppExit(void)
    {
        romfsExit();
        hidExit();
        nsExit();
        setsysExit();
        accountExit();
    }
}

int main(int argc, const char *argv[])
{
    fs::init();
    graphicsInit(1280, 720, MAX_GFXCMD_DEFAULT);
    data::loadDataInfo();
    ui::init();

    //built with 'make debug CFLAGS:=-D__debug__'
    #ifdef __debug__
    socketInitializeDefault();
    nxlinkStdio();
    #endif

    bool run = true;
    while(appletMainLoop() && run)
    {
        hidScanInput();

        uint64_t down = hidKeysDown(CONTROLLER_P1_AUTO);
        uint64_t held = hidKeysHeld(CONTROLLER_P1_AUTO);

        touchPosition p;
        hidTouchRead(&p, 0);

        if((held & KEY_L) && (held & KEY_R) && (held & KEY_ZL) && (held & KEY_ZR))
        {
            if(ui::confirm("You are about to enable system save dumping and remove checks. Are you sure you want to continue?"))
            {
                data::sysSave = true;
                data::loadDataInfo();

                //Just to be sure
                fsdevUnmountDevice("sv");

                //Kick back to user
                ui::mstate = USR_SEL;
            }
        }
        else if(down & KEY_PLUS)
            break;
        ui::runApp(down, held, p);

        gfxProcQueue();
        gfxHandleBuffs();
    }
    #ifdef __debug__
    socketExit();
    #endif

    graphicsExit();
    ui::exit();
    data::exit();
}
