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

