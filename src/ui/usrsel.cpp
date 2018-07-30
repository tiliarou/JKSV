#include <string>
#include <fstream>
#include <switch.h>

#include "ui.h"
#include "uiupdate.h"
#include "file.h"

std::string username;

namespace ui
{
    void updateUserMenu(const uint64_t& down, const uint64_t& held, const touchPosition& p)
    {
        //Static so they don't get reset every loop
        static int start = 0, selected = 0;

        static uint8_t clrShft = 0;
        static bool clrAdd = true;

        static unsigned selRectX = 64, selRectY = 74;

        static ui::touchTrack track;

        //Color swapping selBox
        clr clrPrev = clrCreateRGBA(0x00, 0x60 + clrShft, 0xBB + clrShft, 0xFF);

        if(clrAdd)
        {
            clrShft += 6;
            if(clrShft > 63)
                clrAdd = false;
        }
        else
        {
            clrShft--;
            if(clrShft == 0)
                clrAdd = true;
        }

        //Update selBox color
        clr clrUpdt = clrCreateRGBA(0x00, 0x60 + clrShft, 0xBB + clrShft, 0xFF);

        unsigned x = 70, y = 80;
        unsigned endUser = start + 32;
        if(start + 32 > (int)data::users.size())
            endUser = data::users.size();

        texSwapColors(ui::selBox, clrPrev, clrUpdt);
        texDraw(ui::selBox, frameBuffer, selRectX, selRectY, false);

        for(unsigned i = start; i < endUser; y += 144)
        {
            unsigned endRow = i + 8;
            for(unsigned tX = x; i < endRow; i++, tX += 144)
            {
                if(i == endUser)
                    break;

                if((int)i == selected)
                {
                    if(selRectX != tX - 6)
                    {
                        if(selRectX < tX - 6)
                            selRectX += 24;
                        else
                            selRectX -= 24;
                    }

                    if(selRectY != y - 6)
                    {
                        if(selRectY < y - 6)
                            selRectY += 24;
                        else
                            selRectX -= 24;
                    }

                    username = data::users[selected].getUsername();
                    unsigned userWidth = textGetWidth(username.c_str(), ui::shared, 16);
                    int userRectWidth = userWidth + 32, userRectX = (tX + 64) - (userRectWidth  / 2);
                    if(userRectX < 16)
                        userRectX = 16;

                    if(userRectX + userRectWidth > 1264)
                        userRectX = 1264 - userRectWidth;

                    drawTextbox(userRectX, y - 50, userRectWidth, 38, false);
                    drawText(username.c_str(), frameBuffer, ui::shared, userRectX + 16, y - 38, 16, txtClr, false);
                }

                texDrawSkipNoAlpha(data::users[i].userIcon, frameBuffer, tX, y, false);
            }
        }


        //Update invisible buttons
        for(int i = 0; i < 32; i++)
        {
            selButtons[i].update(p);
            if(selected == i && selButtons[i].getEvent() == BUTTON_RELEASED)
            {
                data::curUser = data::users[selected];
                mstate = TTL_SEL;
            }
            else if(selButtons[i].getEvent() == BUTTON_RELEASED)
            {
                if(start + i < (int)data::users.size())
                    selected = start + i;
            }
        }

        //Update touch tracking
        track.update(p);

        if(down & KEY_RIGHT)
        {
            if(selected < (int)data::users.size() - 1)
                selected++;

            if(selected >= (int)start + 32)
                start += 8;
        }
        else if(down & KEY_LEFT)
        {
            if(selected > 0)
                selected--;

            if(selected < start)
                start -= 8;
        }
        else if(down & KEY_UP)
        {
            selected -= 8;
            if(selected < 0)
                selected = 0;

            if(selected - start >= 32)
                start -= 8;
        }
        else if(down & KEY_DOWN)
        {
            selected += 8;
            if(selected > (int)data::users.size() - 1)
                selected = data::users.size() - 1;

            if(selected - start >= 32)
                start += 8;
        }
        else if(down & KEY_A)
        {
            data::curUser = data::users[selected];
            //Reset this
            start = 0;
            selected = 0;
            selRectX = 64, selRectY = 74;
            mstate = TTL_SEL;
        }
        else if(down & KEY_Y)
        {
            for(unsigned i = 0; i < data::users.size(); i++)
                fs::dumpAllUserSaves(data::users[i]);
        }
        else if(down & KEY_X)
        {
            //Just create file so user doesn't have to constantly enable
            std::fstream cls(fs::getWorkDir() + "cls.txt", std::ios::out);
            cls.close();
            clsUserPrep();
            mstate = CLS_USR;
            clsMode = true;
        }

    }
}
