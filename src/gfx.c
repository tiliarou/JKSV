#include <switch.h>
#include <malloc.h>

#include "gfx.h"
#include "gfx/text.h"
#include "gfx/tex.h"

//gfx command buffer/queue
gfxCmd **gfxCmdBuf;

//cmd count related stuff
static int cmdCur = 0, cmdAdd = 0, cmdMax = 0;

//Framebuffer lock
bool fbLock = false;

inline void waitForBufferLock()
{
    while(fbLock){ svcSleepThread(10); }
}

inline void lockFB()
{
    fbLock = true;
}

inline void unlockFB()
{
    fbLock = false;
}

tex *frameBuffer;

void gfxProcFunc(void *args)
{
    gfxCmd *proc = NULL;
    int threadID = (int)(args);
    while(cmdCur < cmdMax - 1 && (proc = gfxCmdBuf[cmdCur++]) != NULL)
    {
        waitForBufferLock();
        if(proc->lock)
            lockFB();

        switch(proc->cmd)
        {
            case DRAW_TEXT:
                {

                    //Send the faceID to use according to thread
                    textArgs *tmp = (textArgs *)proc->argStruct;
                    tmp->faceID = threadID;
                    drawText_t(proc->argStruct);

                }
                break;

            case DRAW_IMG:
                texDraw_t(proc->argStruct);
                break;

            case DRAW_IMG_NO_ALPHA:
                texDrawNoAlpha_t(proc->argStruct);
                break;

            case DRAW_IMG_SKIP:
                texDrawSkip_t(proc->argStruct);
                break;

            case DRAW_IMG_SKIP_NO_ALPHA:
                texDrawSkipNoAlpha_t(proc->argStruct);
                break;

            case DRAW_IMG_INVERT:
                texDrawInvert_t(proc->argStruct);
                break;

            case DRAW_RECT:
                drawRect_t(proc->argStruct);
                break;
        }
        unlockFB();
    }
}

void graphicsInit(int windowWidth, int windowHeight, int maxCmd)
{
    gfxInitResolution(windowWidth, windowHeight);
    gfxInitDefault();
    consoleInit(NULL);
    plInitialize();

    gfxSetMode(GfxMode_LinearDouble);

    //Make a fake tex that points to framebuffer
    frameBuffer = malloc(sizeof(tex));
    frameBuffer->width = windowWidth;
    frameBuffer->height = windowHeight;
    frameBuffer->data = (uint32_t *)gfxGetFramebuffer(NULL, NULL);
    frameBuffer->size = windowWidth * windowHeight;

    gfxCmdBuf = malloc(sizeof(gfxCmd *) * maxCmd);
    for(int i = 0; i < maxCmd; i++)
        gfxCmdBuf[i] = NULL;

    cmdMax = maxCmd;
}

void graphicsExit()
{
    free(frameBuffer);
    plExit();
    gfxExit();
}

void gfxHandleBuffs()
{
    gfxFlushBuffers();
    gfxSwapBuffers();
    gfxWaitForVsync();
}

gfxCmd *gfxCmdCreate(int cmd, bool lock, void *argStruct)
{
    gfxCmd *ret = malloc(sizeof(gfxCmd));
    ret->cmd = cmd;
    ret->lock = lock;
    ret->argStruct = argStruct;
    return ret;
}

void gfxCmdDestroy(gfxCmd *g)
{
    if(g->argStruct != NULL)
        free(g->argStruct);

    free(g);
}

void gfxCmdAddToQueue(gfxCmd * g)
{
    if(cmdAdd < cmdMax - 1)
        gfxCmdBuf[cmdAdd++] = g;
}

void gfxProcQueue()
{
    cmdCur = 0;

    Thread gfxThread[2];
    threadCreate(&gfxThread[0], gfxProcFunc, (void *)0, 0x5000, 0x2C, 1);
    threadStart(&gfxThread[0]);
    svcSleepThread(100000);
    threadCreate(&gfxThread[1], gfxProcFunc, (void *)1, 0x5000, 0x2C, 2);
    threadStart(&gfxThread[1]);
    svcSleepThread(100000);
    gfxProcFunc((void *)2);

    threadWaitForExit(&gfxThread[0]);
    threadClose(&gfxThread[0]);
    threadWaitForExit(&gfxThread[1]);
    threadClose(&gfxThread[1]);

    //reset
    cmdAdd = 0;
    cmdCur = 0;
    for(int i = 0; i < cmdMax; i++)
    {
        if(gfxCmdBuf[i] != NULL)
            gfxCmdDestroy(gfxCmdBuf[i]);

        gfxCmdBuf[i] = NULL;
    }
}

typedef struct
{
    tex *target;
    int x, y, w, h;
    clr c;
} rectArgs;

void drawRect(tex *target, int x, int y, int w, int h, clr c, bool locked)
{
    rectArgs *args = malloc(sizeof(rectArgs));
    args->target = target;
    args->x = x;
    args->y = y;
    args->w = w;
    args->h = h;
    args->c = c;

    gfxCmd *add = gfxCmdCreate(DRAW_RECT, locked, args);
    gfxCmdAddToQueue(add);
}

void drawRect_t(void *argStruct)
{
    rectArgs *args = (rectArgs *)argStruct;
    tex *target = args->target;
    int x = args->x, y = args->y, w = args->w, h = args->h;
    uint32_t color = clrGetPixel(args->c);

    for(int tY = y; tY < y + h; tY++)
    {
        if(tY < 0 || tY > target->height)
            continue;

        uint32_t *rowPtr = &target->data[tY * target->width + x];
        for(int tX = x; tX < x + w; tX++)
        {
            if(tX < 0 || tX > target->width)
                continue;

            *rowPtr++ = color;
        }
    }
}
