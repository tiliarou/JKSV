#include <switch.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SIZES_H

#include "gfx.h"
#include "gfx/clr.h"
#include "gfx/text.h"

textArgs *textArgsCreate(const char *str, tex *target, const font *f, int x, int y, int sz, clr c)
{
    textArgs *ret = malloc(sizeof(textArgs));
    ret->str = str;
    ret->target = target;
    ret->f = f;
    ret->x = x;
    ret->y = y;
    ret->sz = sz;
    ret->c = c;
    return ret;
}

font *fontLoadSharedFont(PlSharedFontType fontType)
{
    PlFontData plFont;

    if(R_FAILED(plGetSharedFontByType(&plFont, fontType)))
        return NULL;

    font *ret = malloc(sizeof(font));

    if((ret->libRet = FT_Init_FreeType(&ret->lib)))
    {
        free(ret);
        return NULL;
    }

    if((ret->faceRet = FT_New_Memory_Face(ret->lib, plFont.address, plFont.size, 0, &ret->face[0])))
    {
        free(ret);
        return NULL;
    }

    if((ret->faceRet = FT_New_Memory_Face(ret->lib, plFont.address, plFont.size, 0, &ret->face[1])))
    {
        free(ret);
        return NULL;
    }

    ret->fntData = NULL;

    return ret;
}

font *fontLoadTTF(const char *path)
{
    font *ret = malloc(sizeof(font));
    if((ret->libRet = FT_Init_FreeType(&ret->lib)))
    {
        free(ret);
        return NULL;
    }

    FILE *ttf = fopen(path, "rb");
    fseek(ttf, 0, SEEK_END);
    size_t ttfSize = ftell(ttf);
    fseek(ttf, 0, SEEK_SET);

    ret->fntData = malloc(ttfSize);
    fread(ret->fntData, 1, ttfSize, ttf);
    fclose(ttf);

    if((ret->faceRet = FT_New_Memory_Face(ret->lib, ret->fntData, ttfSize, 0, &ret->face[0])))
    {
        free(ret->fntData);
        free(ret);
        return NULL;
    }

    if((ret->faceRet = FT_New_Memory_Face(ret->lib, ret->fntData, ttfSize, 0, &ret->face[1])))
    {
        free(ret->fntData);
        free(ret);
        return NULL;
    }

    return ret;
}

void fontDestroy(font *f)
{
    if(f->faceRet == 0)
    {
        FT_Done_Face(f->face[0]);
        FT_Done_Face(f->face[1]);
    }
    if(f->libRet == 0)
        FT_Done_FreeType(f->lib);
    if(f->fntData != NULL)
        free(f->fntData);

    free(f);
}

static void drawGlyph(const FT_Bitmap *bmp, tex *target, int _x, int _y, const clr c)
{
    if(bmp->pixel_mode != FT_PIXEL_MODE_GRAY)
        return;

    uint8_t *bmpPtr = bmp->buffer;
    uint32_t *rowPtr =  NULL;
    for(int y = _y; y < _y + bmp->rows; y++)
    {
        if(y > target->height || y < 0)
            continue;

        rowPtr = &target->data[y * target->width + _x];
        for(int x = _x; x < _x + bmp->width; x++, bmpPtr++, rowPtr++)
        {
            if(x > target->width || x < 0)
                continue;

            if(*bmpPtr > 0)
            {
                clr txClr = clrCreateRGBA(c.r, c.g, c.b, *bmpPtr);
                clr tgtClr = clrCreateU32(*rowPtr);

                *rowPtr = blend(txClr, tgtClr);
            }
        }
    }
}

void drawText(const char *str, tex *target, font *f, int x, int y, int sz, clr c, bool locked)
{
    textArgs *args = textArgsCreate(str, target, f, x, y, sz, c);
    gfxCmd *cmd = gfxCmdCreate(DRAW_TEXT, locked, args);
    gfxCmdAddToQueue(cmd);
}

void drawText_t(void *argStruct)
{
    textArgs *args = (textArgs *)argStruct;
    const char *str = args->str;
    tex *target = args->target;
    const font *f = args->f;
    int x = args->x, y = args->y, sz = args->sz;
    clr c = args->c;
    int faceID = args->faceID;

    int tmpX = x;
    FT_Error ret = 0;
    FT_GlyphSlot slot = f->face[faceID]->glyph;
    uint32_t tmpChr = 0;
    ssize_t unitCnt = 0;

    FT_Set_Char_Size(f->face[faceID], 0, sz * 64, 90, 90);

    size_t length = strlen(str);
    for(unsigned i = 0; i < length; )
    {
        unitCnt = decode_utf8(&tmpChr, (uint8_t *)&str[i]);
        if(unitCnt <= 0)
            break;

        i += unitCnt;
        if(tmpChr == '\n')
        {
            tmpX = x;
            y += sz + 8;
            continue;
        }

        ret = FT_Load_Glyph(f->face[faceID], FT_Get_Char_Index(f->face[faceID], tmpChr), FT_LOAD_RENDER);
        if(ret)
            return;

        int drawY = y + (sz - slot->bitmap_top);
        drawGlyph(&slot->bitmap, target, tmpX + slot->bitmap_left, drawY, c);

        tmpX += slot->advance.x >> 6;
    }
}

size_t textGetWidth(const char *str, const font *f, int sz)
{
    size_t width = 0;

    uint32_t untCnt = 0, tmpChr = 0;
    FT_GlyphSlot slot = f->face[0]->glyph;
    FT_Error ret = 0;

    FT_Set_Char_Size(f->face[0], 0, 64 * sz, 90, 90);

    size_t length = strlen(str);
    for(unsigned i = 0; i < length; )
    {
        untCnt = decode_utf8(&tmpChr, (uint8_t *)&str[i]);

        if(untCnt <= 0)
            break;

        i += untCnt;
        ret = FT_Load_Glyph(f->face[0], FT_Get_Char_Index(f->face[0], tmpChr), FT_LOAD_RENDER);
        if(ret)
            return 0;

        width += slot->advance.x >> 6;
    }

    return width;
}
