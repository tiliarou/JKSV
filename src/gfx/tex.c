#include <switch.h>
#include <string.h>
#include <malloc.h>
#include <png.h>
#include <jpeglib.h>

#include "gfx.h"
#include "clr.h"

texArgs *texArgsCreate(const tex *t, tex *target, int x, int y)
{
    texArgs *ret = malloc(sizeof(texArgs));
    ret->t = t;
    ret->target = target;
    ret->x = x;
    ret->y = y;
    return ret;
}

tex *texCreate(int w, int h)
{
    tex *ret = malloc(sizeof(tex));

    ret->width = w;
    ret->height = h;

    ret->data = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    memset(ret->data, 0, w * h * sizeof(uint32_t));
    ret->size = w * h;

    return ret;
}

tex *texLoadPNGFile(const char *path)
{
    FILE *pngIn = fopen(path, "rb");
    if(pngIn != NULL)
    {
        png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if(png == 0)
            return NULL;

        png_infop pngInfo = png_create_info_struct(png);
        if(pngInfo == 0)
            return NULL;

        int jmp = setjmp(png_jmpbuf(png));
        if(jmp)
            return NULL;

        png_init_io(png, pngIn);
        png_read_info(png, pngInfo);

        if(png_get_color_type(png, pngInfo) != PNG_COLOR_TYPE_RGBA)
        {
            png_destroy_read_struct(&png, &pngInfo, NULL);
            return NULL;
        }

        tex *ret = malloc(sizeof(tex));
        ret->width = png_get_image_width(png, pngInfo);
        ret->height = png_get_image_height(png, pngInfo);

        ret->data = (uint32_t *)malloc((ret->width * ret->height) * sizeof(uint32_t));
        ret->size = ret->width * ret->height;

        png_bytep *rows = malloc(sizeof(png_bytep) * ret->height);
        for(int i = 0; i < ret->height; i++)
            rows[i] = malloc(png_get_rowbytes(png, pngInfo));

        png_read_image(png, rows);

        uint32_t *dataPtr = &ret->data[0];
        for(int y = 0; y < ret->height; y++)
        {
            uint32_t *rowPtr = (uint32_t *)rows[y];
            for(int x = 0; x < ret->width; x++)
                *dataPtr++ = *rowPtr++;
        }

        for(int i = 0; i < ret->height; i++)
            free(rows[i]);

        free(rows);

        png_destroy_read_struct(&png, &pngInfo, NULL);
        fclose(pngIn);

        return ret;
    }
    return NULL;
}

tex *texLoadJPEGFile(const char *path)
{
    FILE *jpegIn = fopen(path, "rb");
    if(jpegIn != NULL)
    {
        struct jpeg_decompress_struct jpegInfo;
        struct jpeg_error_mgr error;

        jpegInfo.err = jpeg_std_error(&error);

        jpeg_create_decompress(&jpegInfo);
        jpeg_stdio_src(&jpegInfo, jpegIn);
        jpeg_read_header(&jpegInfo, true);

        if(jpegInfo.jpeg_color_space == JCS_YCbCr)
            jpegInfo.out_color_space = JCS_RGB;

        tex *ret = malloc(sizeof(tex));

        ret->width = jpegInfo.image_width;
        ret->height = jpegInfo.image_height;

        ret->data = (uint32_t *)malloc((ret->width * ret->height) * sizeof(uint32_t));
        ret->size = ret->width * ret->height;

        jpeg_start_decompress(&jpegInfo);

        JSAMPARRAY row = malloc(sizeof(JSAMPROW));
        row[0] = malloc(sizeof(JSAMPLE) * ret->width * 3);

        uint32_t *dataPtr = &ret->data[0];
        for(int y = 0; y < ret->height; y++)
        {
            jpeg_read_scanlines(&jpegInfo, row, 1);
            uint8_t *jpegPtr = row[0];
            for(int x = 0; x < ret->width; x++, jpegPtr += 3)
                *dataPtr++ = (0xFF << 24 | jpegPtr[2] << 16 | jpegPtr[1] << 8 | jpegPtr[0]);
        }

        jpeg_finish_decompress(&jpegInfo);
        jpeg_destroy_decompress(&jpegInfo);

        free(row[0]);
        free(row);

        fclose(jpegIn);

        return ret;
    }
    return NULL;
}

tex *texLoadJPEGMem(const uint8_t *jpegData, size_t jpegSize)
{
    struct jpeg_decompress_struct jpegInfo;
    struct jpeg_error_mgr error;

    jpegInfo.err = jpeg_std_error(&error);

    jpeg_create_decompress(&jpegInfo);
    jpeg_mem_src(&jpegInfo, jpegData, jpegSize);
    jpeg_read_header(&jpegInfo, true);

    if(jpegInfo.jpeg_color_space == JCS_YCbCr)
        jpegInfo.out_color_space = JCS_RGB;

    tex *ret = malloc(sizeof(tex));
    ret->width = jpegInfo.image_width;
    ret->height = jpegInfo.image_height;

    ret->data = (uint32_t *)malloc((ret->width * ret->height) * sizeof(uint32_t));
    ret->size = ret->width * ret->height;

    jpeg_start_decompress(&jpegInfo);

    JSAMPARRAY row = malloc(sizeof(JSAMPARRAY));
    row[0] = malloc(sizeof(JSAMPLE) * ret->width * 3);

    uint32_t *dataPtr = &ret->data[0];
    for(int y = 0; y < ret->height; y++)
    {
        jpeg_read_scanlines(&jpegInfo, row, 1);
        uint8_t *jpegPtr = row[0];
        for(int x = 0; x < ret->width; x++, jpegPtr += 3)
            *dataPtr++ = (0xFF << 24 | jpegPtr[2] << 16 | jpegPtr[1] << 8 | jpegPtr[0]);
    }

    jpeg_finish_decompress(&jpegInfo);
    jpeg_destroy_decompress(&jpegInfo);

    free(row[0]);
    free(row);

    return ret;
}

void texDestroy(tex *t)
{
    if(t->data != NULL)
        free(t->data);

    if(t != NULL)
        free(t);
}

void texClearColor(tex *in, const clr c)
{
    uint32_t *dataPtr = &in->data[0];
    uint32_t clr = clrGetPixel(c);
    for(int i = 0; i < in->size; i++)
        *dataPtr++ = clr;
}

void texDraw(const tex *t, tex *target, int x, int y, bool lock)
{
    texArgs *args = texArgsCreate(t, target, x, y);
    gfxCmd *cmd = gfxCmdCreate(DRAW_IMG, lock, args);
    gfxCmdAddToQueue(cmd);
}

void texDraw_t(void *argStruct)
{
    texArgs *args = (texArgs *)argStruct;

    const tex *t = args->t;
    tex *target = args->target;
    int x = args->x, y = args->y;
     if(t != NULL)
    {
        uint32_t *dataPtr = &t->data[0];
        uint32_t *rowPtr = NULL;
        for(int tY = y; tY < y + t->height; tY++)
        {
            if(tY < 0 || tY > target->height)
                continue;

            rowPtr = &target->data[tY * target->width + x];
            for(int tX = x; tX < x + t->width; tX++, rowPtr++)
            {
                if(tX < 0 || tX > target->width)
                    continue;

                clr dataClr = clrCreateU32(*dataPtr++);
                clr fbClr   = clrCreateU32(*rowPtr);

                *rowPtr = blend(dataClr, fbClr);
            }
        }
    }
}

void texDrawNoAlpha(const tex *t, tex *target, int x, int y, bool lock)
{
    texArgs *args = texArgsCreate(t, target, x, y);
    gfxCmd *add = gfxCmdCreate(DRAW_IMG_NO_ALPHA, lock, args);
    gfxCmdAddToQueue(add);
}

void texDrawNoAlpha_t(void *argStruct)
{
    texArgs *args = (texArgs *)argStruct;
    const tex *t = args->t;
    tex *target = args->target;
    int x = args->x, y = args->y;
    if(t != NULL)
    {
        uint32_t *dataPtr = &t->data[0];
        uint32_t *rowPtr = NULL;
        for(int tY = y; tY < y + t->height; tY++)
        {
            if(tY < 0 || tY > target->height)
                continue;

            rowPtr = &target->data[tY * target->width + x];
            for(int tX = x; tX < x + t->width; tX++)
            {
                if(tX < 0 || tX > target->width)
                    continue;

                *rowPtr++ = *dataPtr++;
            }
        }
    }
}

void texDrawSkip(const tex *t, tex *target, int x, int y, bool lock)
{
    texArgs *args = texArgsCreate(t, target, x, y);
    gfxCmd *add = gfxCmdCreate(DRAW_IMG_SKIP, lock, args);
    gfxCmdAddToQueue(add);
}

void texDrawSkip_t(void *argStruct)
{
    texArgs *args = (texArgs *)argStruct;
    const tex *t = args->t;
    tex *target = args->target;
    int x = args->x, y = args->y;

    if(t != NULL)
    {
        uint32_t *dataPtr = &t->data[0];
        uint32_t *rowPtr = NULL;
        for(int tY = y; tY < y + (t->height / 2); tY++, dataPtr += t->width)
        {
            if(tY < 0 || tY > target->height)
                continue;

            rowPtr = &target->data[tY * target->width + x];
            for(int tX = x; tX < x + (t->width / 2); tX++, rowPtr++)
            {
                if(tX < 0 || tX > target->width)
                    continue;

                clr px1 = clrCreateU32(*dataPtr++);
                clr px2 = clrCreateU32(*dataPtr++);
                clr fbPx = clrCreateU32(*rowPtr);

                *rowPtr = blend(clrCreateU32(smooth(px1, px2)), fbPx);
            }
        }
    }
}

void texDrawSkipNoAlpha(const tex *t, tex *target, int x, int y, bool lock)
{
    texArgs *args = texArgsCreate(t, target, x, y);
    gfxCmd *add = gfxCmdCreate(DRAW_IMG_SKIP_NO_ALPHA, lock, args);
    gfxCmdAddToQueue(add);
}

void texDrawSkipNoAlpha_t(void *argStruct)
{
    texArgs *args = (texArgs *)argStruct;
    const tex *t = args->t;
    tex *target = args->target;
    int x = args->x, y = args->y;

    if(t != NULL)
    {
        uint32_t *dataPtr = &t->data[0];
        uint32_t *rowPtr = NULL;
        for(int tY = y; tY < y + (t->height / 2); tY++, dataPtr += t->width)
        {
            if(tY < 0 || tY > target->height)
                continue;

            rowPtr = &target->data[tY * target->width + x];
            for(int tX = x; tX < x + (t->width / 2); tX++, rowPtr++)
            {
                if(tX < 0 || tX > target->width)
                    continue;

                clr px1 = clrCreateU32(*dataPtr++);
                clr px2 = clrCreateU32(*dataPtr++);

                *rowPtr = smooth(px1, px2);
            }
        }
    }
}

void texDrawInvert(const tex *t, tex *target, int x, int y, bool lock)
{
    texArgs *args = texArgsCreate(t, target, x, y);
    gfxCmd *add = gfxCmdCreate(DRAW_IMG_INVERT, lock, args);
    gfxCmdAddToQueue(add);
}

void texDrawInvert_t(void *argStruct)
{
    texArgs *args = (texArgs *)argStruct;
    const tex *t = args->t;
    tex *target = args->target;
    int x = args->x, y = args->y;

    if(t != NULL)
    {
        uint32_t *dataPtr = &t->data[0];
        uint32_t *rowPtr = NULL;
        for(int tY = y; tY < y + t->height; tY++)
        {
            if(tY < 0 || tY > target->height)
                continue;

            rowPtr = &target->data[tY * target->width + x];
            for(int tX = x; tX < x + t->width; tX++, rowPtr++)
            {
                if(tX < 0 || tX > target->width)
                    continue;

                clr dataClr = clrCreateU32(*dataPtr++);
                clrInvert(&dataClr);
                clr fbClr = clrCreateU32(*rowPtr);

                *rowPtr = blend(dataClr, fbClr);
            }
        }
    }
}

void texSwapColors(tex *t, const clr old, const clr newColor)
{
    uint32_t oldClr = clrGetPixel(old), newClr = clrGetPixel(newColor);

    uint32_t *dataPtr = &t->data[0];
    for(int i = 0; i < t->size; i++, dataPtr++)
    {
        if(*dataPtr == oldClr)
            *dataPtr = newClr;
    }
}
