#ifndef TTF2C_H
#define TTF2C_H

typedef struct
{
   int off_x, off_y;
   int adv_x, adv_y;
   int width, height;
   const uint8_t *bitmap;
} ttf2c_glyph;

typedef struct
{
   int max_width;
   int line_height;
   int ascent, descent, line_gap;
   ttf2c_glyph glyphs[256];
} ttf2c_font;

int ttf2c_get_width(const ttf2c_font *font, const char *text);

/* use negative height for EM-sizes */
ttf2c_font *ttf2c_create(const uint8_t *data, int index, int height);

/* only needed for ttf2c_create() created fonts */
void ttf2c_destroy(ttf2c_font *font);

#endif // TTF2C_H

