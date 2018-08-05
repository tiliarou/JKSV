#ifndef TEX_H
#define TEX_H

#include "clr.h"

typedef struct
{
    size_t size;
    int width, height;
    uint32_t *data;
} tex;

//Arg struct for threaded gfx
typedef struct
{
    const tex *t;
    tex *target;
    int x, y;
} texArgs;

texArgs *texArgsCreate(const tex *t, tex *target, int x, int y);

//Inits empty tex
tex *texCreate(int w, int h);

//Loads PNG from path
tex *texLoadPNGFile(const char *path);

//Loads JPEG from path
tex *texLoadJPEGFile(const char *path);

//Loads JPEG from memory
tex *texLoadJPEGMem(const uint8_t *jpegData, size_t jpegSize);

//Frees memory used be t
void texDestroy(tex *t);

//Clears tex with c
void texClearColor(tex *t, const clr c);

//Draws t at x, y on target. Lock being true prevents any drawing until this one is finished
//This is only for stuff that absolutely needs to be drawn for layers
void texDraw(const tex *t, tex *target, int x, int y, bool lock);
//For threads
void texDraw_t(void *argStruct);

//Same as above, but without alpha blending for speed
void texDrawNoAlpha(const tex *t, tex *target, int x, int y, bool lock);
void texDrawNoAlpha_t(void *argStruct);

//Skips every other pixel + row
void texDrawSkip(const tex *t, tex *target, int x, int y,  bool lock);
void texDrawSkip_t(void *argStruct);

//^ - alpha
void texDrawSkipNoAlpha(const tex *t, tex *target, int x, int y, bool lock);
void texDrawSkipNoAlpha_t(void *argStruct);

//Inverts img colors then draws
void texDrawInvert(const tex *t, tex *target, int x, int y, bool lock);
void texDrawInvert_t(void *argStruct);

//Swaps old for new
void texSwapColors(tex *t, const clr old, const clr newClr);
#endif
