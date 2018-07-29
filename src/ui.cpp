#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <switch.h>

#include "ui.h"
#include "gfx.h"
#include "util.h"
#include "file.h"

#define TITLE_TEXT "JKSV - 07/28/2018 - MultiThread GFX Test"

//Secret background that can be drawn from "/JKSV/back.jpg"
static tex *background = NULL;

namespace ui
{
    //Classic mode
    bool clsMode = false;

    //Current menu state
    int mstate = USR_SEL;

    //Info printed on folder menu
    std::string folderMenuInfo;

    //Touch button vector
    std::vector<ui::button> selButtons;

    //UI colors
    clr clearClr, mnuTxt, txtClr, rectLt, rectSh, tboxClr;

    //textbox pieces
    //I was going to flip them when I draw them, but then laziness kicked in.
    tex *cornerTopLeft, *cornerTopRight, *cornerBottomLeft, *cornerBottomRight;

    tex *buttonA, *buttonB, *buttonX, *buttonY, *buttonMin;

    tex *selBox;

    font *shared;

    void init()
    {
        ColorSetId gthm;
        setsysGetColorSetId(&gthm);

        switch(gthm)
        {
            case ColorSetId_Light:
                //Dark corners
                cornerTopLeft = texLoadPNGFile("romfs:/img/tboxDrk/tboxCornerTopLeft.png");
                cornerTopRight = texLoadPNGFile("romfs:/img/tboxDrk/tboxCornerTopRight.png");
                cornerBottomLeft = texLoadPNGFile("romfs:/img/tboxDrk/tboxCornerBotLeft.png");
                cornerBottomRight = texLoadPNGFile("romfs:/img/tboxDrk/tboxCornerBotRight.png");

                //Dark buttons
                buttonA = texLoadPNGFile("romfs:/img/button/buttonA_drk.png");
                buttonB = texLoadPNGFile("romfs:/img/button/buttonB_drk.png");
                buttonX = texLoadPNGFile("romfs:/img/button/buttonX_drk.png");
                buttonY = texLoadPNGFile("romfs:/img/button/buttonY_drk.png");
                buttonMin = texLoadPNGFile("romfs:/img/button/buttonMin_drk.png");

                clearClr = clrCreateU32(0xFFEBEBEB);
                mnuTxt = clrCreateU32(0xFF000000);
                txtClr = clrCreateU32(0xFFFFFFFF);
                rectLt = clrCreateU32(0xFFDFDFDF);
                rectSh = clrCreateU32(0xFFCACACA);
                tboxClr = clrCreateU32(0xFF505050);
                break;

            default:
            case ColorSetId_Dark:
                //Light corners
                cornerTopLeft = texLoadPNGFile("romfs:/img/tboxLght/tboxCornerTopLeft.png");
                cornerTopRight = texLoadPNGFile("romfs:/img/tboxLght/tboxCornerTopRight.png");
                cornerBottomLeft = texLoadPNGFile("romfs:/img/tboxLght/tboxCornerBotLeft.png");
                cornerBottomRight = texLoadPNGFile("romfs:/img/tboxLght/tboxCornerBotRight.png");

                //Light buttons
                buttonA = texLoadPNGFile("romfs:/img/button/buttonA_lght.png");
                buttonB = texLoadPNGFile("romfs:/img/button/buttonB_lght.png");
                buttonX = texLoadPNGFile("romfs:/img/button/buttonX_lght.png");
                buttonY = texLoadPNGFile("romfs:/img/button/buttonY_lght.png");
                buttonMin = texLoadPNGFile("romfs:/img/button/buttonMin_lght.png");

                clearClr = clrCreateU32(0xFF2D2D2D);
                mnuTxt = clrCreateU32(0xFFFFFFFF);
                txtClr = clrCreateU32(0xFF000000);
                rectLt = clrCreateU32(0xFF505050);
                rectSh = clrCreateU32(0xFF202020);
                tboxClr = clrCreateU32(0xFFEBEBEB);
                break;
        }

        if(fs::fileExists(fs::getWorkDir() + "font.ttf"))
            shared = fontLoadTTF(std::string(fs::getWorkDir() + "font.ttf").c_str());
        else
            shared = fontLoadSharedFont(PlSharedFontType_KO);

        setupSelButtons();

        selBox = texLoadPNGFile("romfs:/img/icn/icnSelBox.png");

        if(fs::fileExists(fs::getWorkDir() + "back.jpg"))
            background = texLoadJPEGFile(std::string(fs::getWorkDir() + "back.jpg").c_str());

        if(fs::fileExists(fs::getWorkDir() + "cls.txt"))
        {
            clsUserPrep();
            clsMode = true;
            mstate = CLS_USR;
        }

        advCopyMenuPrep();
    }

    void exit()
    {
        texDestroy(cornerTopLeft);
        texDestroy(cornerTopRight);
        texDestroy(cornerBottomLeft);
        texDestroy(cornerBottomRight);

        texDestroy(buttonA);
        texDestroy(buttonB);
        texDestroy(buttonX);
        texDestroy(buttonY);
        texDestroy(buttonMin);

        texDestroy(selBox);

        if(background != NULL)
            texDestroy(background);

        fontDestroy(shared);
    }

    void setupSelButtons()
    {
        int x = 70, y = 80;
        for(int i = 0; i < 32; y += 144)
        {
            int endRow = i + 8;
            for(int tX = x; i < endRow; tX += 144, i++)
            {
                //Make a new button with no text. We're not drawing them anyway
                ui::button newSelButton("", tX, y, 128, 128);
                selButtons.push_back(newSelButton);
            }
        }
    }

    void drawUI()
    {
        if(background == NULL)
            texClearColor(frameBuffer, clearClr);
        else
            texDrawNoAlpha(background, frameBuffer, 0, 0);

        drawText(TITLE_TEXT, frameBuffer, shared, 16, 16, 32, mnuTxt);

        switch(mstate)
        {
            case FLD_SEL:
                drawRect(frameBuffer, 16, 64, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 65, 1248, 2, rectSh);

                drawRect(frameBuffer, 288, 65, 1, 592, rectLt);
                drawRect(frameBuffer, 289, 65, 2, 592, rectSh);

                drawRect(frameBuffer, 16, 656, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 657, 1248, 2, rectSh);
                break;

            case USR_SEL:
            case TTL_SEL:
                drawRect(frameBuffer, 16, 64, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 65, 1248, 2, rectSh);

                drawRect(frameBuffer, 16, 656, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 657, 1248, 2, rectSh);
                break;

            case ADV_MDE:
                drawRect(frameBuffer, 16, 64, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 65, 1248, 2, rectSh);

                drawRect(frameBuffer, 640, 64, 1, 592, rectLt);
                drawRect(frameBuffer, 641, 64, 2, 592, rectSh);

                drawRect(frameBuffer, 16, 656, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 657, 1248, 2, rectSh);
                break;

            case CLS_TTL:
            case CLS_USR:
                drawRect(frameBuffer, 16, 64, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 65, 1248, 2, rectSh);

                drawRect(frameBuffer, 448, 64, 1, 592, rectLt);
                drawRect(frameBuffer, 449, 64, 2, 592, rectSh);

                drawRect(frameBuffer, 16, 656, 1248, 1, rectLt);
                drawRect(frameBuffer, 16, 657, 1248, 2, rectSh);
                break;
        }

        switch(mstate)
        {
            case USR_SEL:
            case CLS_USR:
                {
                    //Input guide
                    unsigned startX = 848;
                    texDraw(buttonA, frameBuffer, startX, 672);
                    drawText("Select", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonY, frameBuffer, startX += 72, 672);
                    drawText("Dump All", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonX, frameBuffer, startX += 96, 672);
                    drawText("Classic Mode", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                }
                break;

            case TTL_SEL:
            case CLS_TTL:
                {
                    unsigned startX = 914;
                    texDraw(buttonA, frameBuffer, startX, 672);
                    drawText("Select", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonY, frameBuffer, startX += 72, 672);
                    drawText("Dump All", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonB, frameBuffer, startX += 96, 672);
                    drawText("Back", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                }
                break;

            case FLD_SEL:
                {
                    //Input guide
                    unsigned startX = 690;
                    texDraw(buttonMin, frameBuffer, startX, 672);
                    drawText("Adv. Mode", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonA, frameBuffer, startX += 100, 672);
                    drawText("Backup", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonY, frameBuffer, startX += 72, 672);
                    drawText("Restore", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonX, frameBuffer, startX += 72, 672);
                    drawText("Delete", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                    texDraw(buttonB, frameBuffer, startX += 72, 672);
                    drawText("Back", frameBuffer, shared, startX += 38, 680, 14, mnuTxt);
                }
                break;
        }
    }

    void runApp(const uint64_t& down, const uint64_t& held, const touchPosition& p)
    {
        //Draw first. Shouldn't, but it simplifies the showX functions
        drawUI();

        switch(mstate)
        {
            case USR_SEL:
                updateUserMenu(down, held, p);
                break;

            case TTL_SEL:
                updateTitleMenu(down, held, p);
                break;

            case FLD_SEL:
                updateFolderMenu(down, held, p);
                break;

            case ADV_MDE:
                updateAdvMode(down, held, p);
                break;

            case CLS_USR:
                classicUserMenuUpdate(down, held, p);
                break;

            case CLS_TTL:
                classicTitleMenuUpdate(down, held, p);
                break;
        }
    }
}
