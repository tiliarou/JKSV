#ifndef GFX_H
#define GFX_H

#include <stdint.h>

#define MAX_GFXCMD_DEFAULT 128

#ifdef __cplusplus
extern "C"
{
#endif

#include "gfx/clr.h"
#include "gfx/tex.h"
#include "gfx/text.h"

typedef enum
{
    DRAW_TEXT,
    DRAW_IMG,
    DRAW_IMG_NO_ALPHA,
    DRAW_IMG_SKIP,
    DRAW_IMG_SKIP_NO_ALPHA,
    DRAW_IMG_INVERT,
    DRAW_RECT
} gfxCommands;

typedef struct
{
    int cmd;
    void *argStruct;
} gfxCmd;

void graphicsInit(int windowWidth, int windowHeight, int maxCmd);
void graphicsExit();
void gfxHandleBuffs();
extern tex *frameBuffer;

gfxCmd *gfxCmdCreate(int cmd, void *argStruct);
void gfxCmdDestroy(gfxCmd *g);
void gfxCmdAddToQueue(gfxCmd *g);
void gfxProcQueue();

void drawRect(tex *target, int x, int y, int w, int h, clr c);
void drawRect_t(void *argStruct);

#ifdef __cplusplus
}
#endif

#endif
