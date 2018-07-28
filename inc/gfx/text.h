#ifndef TEXT_H
#define TEXT_H

#include <switch.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "gfx/clr.h"
#include "gfx/tex.h"

typedef struct
{
    FT_Library lib;
    //Threading needs two faces for safety
    FT_Face    face[2];
    FT_Error libRet, faceRet;
    //Loads TTF to memory for speed
    uint8_t *fntData;
} font;

//Args for threaded gfx
typedef struct
{
    const char *str;
    tex *target;
    const font *f;
    int x, y, sz;
    clr c;
    int faceID;
} textArgs;

//Loads Switch's shared font
font *fontLoadSharedFont(PlSharedFontType fontType);

//Loads a TTF from path
font *fontLoadTTF(const char *path);

//Frees memory used by font
void fontDestroy(font *f);

//draws text using font f
void drawText(const char *str, tex *target, font *f, int x, int y, int sz, clr c);
void drawText_t(void *argStruct);

size_t textGetWidth(const char *str, const font *f, int sz);

#endif
