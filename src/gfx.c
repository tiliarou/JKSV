#include <switch.h>
#include <malloc.h>

#include "gfx.h"
#include "gfx/text.h"
#include "gfx/tex.h"

//gfx command buffer/queue
gfxCmd **gfxCmdBuf;

//cmd count related stuff
static volatile int cmdCur = 0, cmdAdd = 0, cmdMax = 0;

tex *frameBuffer;

void gfxProcFunc(void *args)
{
    gfxCmd *proc = NULL;
    int threadID = (int)(args);
    while(cmdCur < cmdMax - 1 && (proc = gfxCmdBuf[cmdCur++]) != NULL)
    {
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
        svcSleepThread(10000 * (threadID + 1));
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

gfxCmd *gfxCmdCreate(int cmd, void *argStruct)
{
    gfxCmd *ret = malloc(sizeof(gfxCmd));
    ret->cmd = cmd;
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

    Thread gfxThread;
    threadCreate(&gfxThread, gfxProcFunc, 0, 0x5000, 0x2D, 1);
    threadStart(&gfxThread);
    svcSleepThread(1000000);
    gfxProcFunc((void *)1);
    threadWaitForExit(&gfxThread);
    threadClose(&gfxThread);

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

void drawRect(tex *target, int x, int y, int w, int h, clr c)
{
    rectArgs *args = malloc(sizeof(rectArgs));
    args->target = target;
    args->x = x;
    args->y = y;
    args->w = w;
    args->h = h;
    args->c = c;

    gfxCmd *add = gfxCmdCreate(DRAW_RECT, args);
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
        for(int tX = x; tX < x + w; tX++, rowPtr)
        {
            if(tX < 0 || tX > target->width)
                continue;

            *rowPtr++ = color;
        }
    }
}
