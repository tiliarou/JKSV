#ifndef CLR_H
#define CLR_H

typedef struct
{
    uint8_t r, g, b, a;
} clr;

inline clr clrCreateU32(const uint32_t color)
{
    clr ret;
    ret.a = color >> 24 & 0xFF;
    ret.b = color >> 16 & 0xFF;
    ret.g = color >>  8 & 0xFF;
    ret.r = color & 0xFF;
    return ret;
}

inline clr clrCreateRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    clr ret;
    ret.r = r;
    ret.g = g;
    ret.b = b;
    ret.a = a;
    return ret;
}

inline uint32_t clrGetPixel(const clr c)
{
    return (c.a << 24 | c.b << 16 | c.g << 8 | c.r);
}

inline void clrInvert(clr *c)
{
    c->r = 0xFF - c->r;
    c->g = 0xFF - c->g;
    c->b = 0xFF - c->b;
}

//Alpha blends two colors
inline uint32_t blend(const clr px1, const clr px2)
{
    if(px1.a == 0x00)
        return clrGetPixel(px2);
    else if(px1.a == 0xFF)
        return clrGetPixel(px1);

    uint8_t subAl = 0xFF - px1.a;

    uint8_t fR = (px1.r * px1.a + px2.r * subAl) / 0xFF;
    uint8_t fG = (px1.g * px1.a + px2.g * subAl) / 0xFF;
    uint8_t fB = (px1.b * px1.a + px2.b * subAl) / 0xFF;

    return (0xFF << 24 | fB << 16 | fG << 8 | fR);
}

//Very subtle blur by averaging two pixels
inline uint32_t smooth(const clr px1, const clr px2)
{
    uint8_t fR = (px1.r + px2.r) / 2;
    uint8_t fG = (px1.g + px2.g) / 2;
    uint8_t fB = (px1.b + px2.b) / 2;
    uint8_t fA = (px1.a + px2.a) / 2;

    return (fA << 24 | fB << 16 | fG << 8 | fR);
}

#endif
