/*

Copyright (c) 2015-2016, Higor Eurípedes
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include "ttf2c.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

int ttf2c_get_width(const ttf2c_font *font, const char *text)
{
   int width = 0;
   const char *c;

   for (c = text; *c; ++c)
   {
      const ttf2c_glyph *g = &font->glyphs[(unsigned)*c];
      width += g->adv_x;
   }

   return width;
}

/* use negative height for EM-sizes */
ttf2c_font *ttf2c_create(const uint8_t *data, int index, int height)
{
   ttf2c_font font, *font_ptr = NULL;
   stbtt_fontinfo info;
   float scale;
   int i;

   if (!stbtt_InitFont(&info, data, stbtt_GetFontOffsetForIndex(data, index)))
      return NULL;

   if (height < 0)
      scale = stbtt_ScaleForMappingEmToPixels(&info, -height);
   else
      scale = stbtt_ScaleForPixelHeight(&info, height);

   memset(&font, 0, sizeof(font));

   stbtt_GetFontVMetrics(&info, &font.ascent, &font.descent, &font.line_gap);
   font.line_height  = (font.ascent - font.descent + font.line_gap) * scale;
   font.line_gap    *= scale;
   font.ascent      *= scale;
   font.descent     *= scale;

   for (i = 0; i < 256; ++i)
   {
      ttf2c_glyph *g = &font.glyphs[i];
      void *bitmap   = stbtt_GetCodepointBitmap(&info, 0, scale, i,
                                                &g->width, &g->height,
                                                &g->off_x, &g->off_y);

      stbtt_GetCodepointHMetrics(&info, i, &g->adv_x, 0);
      g->adv_x *= scale;

      if (bitmap)
      {
         g->bitmap = (uint8_t*)malloc(g->width * g->height);

         if (!g->bitmap)
            goto error;

         memcpy((uint8_t*)g->bitmap, bitmap, g->width * g->height);

         if (g->width > font.max_width)
            font.max_width = g->width;
      }
   }

   font_ptr = (ttf2c_font*)malloc(sizeof(font));

   if (!font_ptr)
      goto error;

   memcpy(font_ptr, &font, sizeof(font));

   return font_ptr;
error:
   for (i = 0; i < 256; ++i)
   {
      if (font.glyphs[i].bitmap)
         free((uint8_t*)font.glyphs[i].bitmap);
   }

   return NULL;
}

void ttf2c_destroy(ttf2c_font *font)
{
   int i;

   for (i = 0; i < 256; ++i)
   {
      if (font->glyphs[i].bitmap)
      {
         free((uint8_t*)font->glyphs[i].bitmap);
         font->glyphs[i].bitmap = NULL;
      }
   }

   free(font);
}

#ifdef TTF2C_MAIN
static uint8_t *load_file(const char *file)
{
   uint8_t *buf;
   FILE *fp = fopen(file, "rb");
   size_t fsize = 0;

   fseek(fp, 0, SEEK_END);
   fsize = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   buf = calloc(1, fsize);
   fread(buf, 1, fsize, fp);
   fclose(fp);

   return buf;
}

void write_data(const ttf2c_font *font, const char *symbol, const char *filename)
{
   int i;
   FILE *fp = fopen(filename, "w");

   fprintf(fp, "unsigned char %s___data[] = {", symbol);
   int column = 0;
   for (i = 0; i < 256; ++i)
   {
      const ttf2c_glyph *g = &font->glyphs[i];
      size_t size = g->width * g->height;

      int j;
      for (j = 0; j < size; ++j)
      {
         if (column % 20 == 0)
            fprintf(fp, "\n  ");

         fprintf(fp, "%u", (unsigned)g->bitmap[j]);

         if (j <= (size-1))
         {
            if (j < (size - 1) || i < 255)
               fputc(',' , fp);
         }
         column++;
      }
   }
   fprintf(fp, "};\n");

   fprintf(fp , "ttf2c_font %s = {\n %i, %i, %i, %i, %i, {\n", symbol,
           font->max_width, font->line_height, font->ascent, font->descent, font->line_gap);

   uint32_t offset = 0;
   for (i = 0; i < 256; ++i)
   {
      const ttf2c_glyph *g = &font->glyphs[i];
      fprintf(fp, "  { %i, %i, %i, %i, %i, %i, ",
              g->off_x, g->off_y,
              g->adv_x, g->adv_y,
              g->width, g->height);

      if (g->bitmap)
         fprintf(fp, "%s___data+%u }", symbol, offset);
      else
         fprintf(fp, "NULL }");

      if (i < 255)
         fputc(',', fp);

      fputc('\n', fp);
      offset += g->width * g->height;
   }

   fprintf(fp, "}};\n");

   fclose(fp);
}

#if 0 /* sample */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static uint8_t *render(const ttf2c_font *font, const char *text, int *width, int *height)
{
   *width  = ttf2c_get_width(font, text);
   *height = font->line_height;

   uint8_t *bmp = calloc(1, (*width * *height));
   const char *c;
   int x = 0;

   for (c = text; *c; ++c)
   {
      const ttf2c_glyph *g = &font->glyphs[(unsigned)*c];
      int line = 0;
      int y    = font->ascent + g->off_y - font->descent;


      for (line = 0; line < g->height; ++line)
         memcpy(&bmp[y++ * *width + x + g->off_x], &g->bitmap[line * g->width], g->width);

      x += g->adv_x;
   }

   return bmp;
}
#endif

int main(int argc, char *argv[])
{
   uint8_t *buf     = load_file(argv[3]);
   ttf2c_font *font = ttf2c_create(buf, 0, atoi(argv[1]));
   free(buf);

   write_data(font, argv[2], argv[4]);

   ttf2c_destroy(font);

   return 0;
}
#endif

