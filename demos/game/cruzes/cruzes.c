#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libretro.h"
#include "ttf2c.h"
#include "font24.h"
#include "font16.h"
#include "font10.h"

static retro_environment_t   env_cb   = NULL;
static retro_input_state_t   input_cb = NULL;
static retro_input_poll_t    poll_cb  = NULL;
static retro_video_refresh_t video_cb = NULL;

static bool bool_env(unsigned env, bool value) { return env_cb(env, &value); }
static bool uint_env(unsigned env, unsigned value) { return env_cb(env, &value); }


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 360
#define SCREEN_MAX_POS MAX(SCREEN_WIDTH, SCREEN_HEIGHT)
#define SCREEN_PITCH  (SCREEN_MAX_POS * sizeof(uint16_t))
#define CELL_SIZE   28
#define GRID_GROUP_SIZE 3
#define GRID_COLS   (GRID_GROUP_SIZE*6)
#define GRID_ROWS   (GRID_GROUP_SIZE*3)
#define GRID_X      100
#define GRID_Y      70

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
   return  ((r >> 3U) << 11U)
         | ((g >> 2U) <<  5U)
         | ((b >> 3U) <<  0U);
}

static inline void rgb565_decompose(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b)
{
   *r = ((color >> 11) << 3) & 0xff;
   *g = ((color >>  5) << 2) & 0xff;
   *b = ((color >>  0) << 3) & 0xff;
}

#define WHITE rgb565(255, 255, 255)
#define BLACK rgb565(255, 255, 255)
#define GREY  rgb565(63, 63, 63)
#define LIGHTGREY  RGB(127, 127, 127)

enum
{
   XALIGN_LEFT   = 0,
   XORIG_RIGHT   = 1 << 0,
   XORIG_CENTER  = 1 << 1,
   YALIGN_TOP    = 0,
   YORIG_BOTTOM  = 1 << 2,
   YORIG_CENTER  = 1 << 3
};

typedef enum
{
   CELL_CLEAR    = 0,
   CELL_SET      = 1,
   CELL_DISABLED = 2,
   CELL_WRONG    = 3,
} cell_state_t;

typedef cell_state_t grid_t[GRID_ROWS][GRID_COLS];

struct {
   struct
   {
      grid_t front; // modifiable by the user
      grid_t back;  // the grid to test against
      int focus_x, focus_y;
   } grid;

   int row_counts[GRID_ROWS][GRID_COLS];
   int col_counts[GRID_COLS][GRID_ROWS];

   bool first_run;

   uint16_t fb[SCREEN_MAX_POS * SCREEN_MAX_POS];


   struct {
      int x, y;
      bool left, right;
      bool prev_left, prev_right;
      bool moved;
   } mouse;

   struct {
      int x, y;
      bool a, b;
      bool moved;
   } joy;

   struct
   {
      unsigned active;
      unsigned hot;
      unsigned last_hot;

      unsigned last_focus;
      unsigned focus;
   } ui;
} g;

static int16_t read_mouse(unsigned id)
{
   return input_cb(0, RETRO_DEVICE_MOUSE, 0, id);
}

static int16_t read_joypad(unsigned id)
{
   return input_cb(0, RETRO_DEVICE_JOYPAD, 0, id);
}

static inline void clip_pos(int *x, int *y)
{
   if (x)
      *x = MAX(MIN(*x, SCREEN_WIDTH), 0);

   if (y)
      *y = MAX(MIN(*y, SCREEN_HEIGHT), 0);
}

static inline void *memset16(void *dst, uint16_t value, size_t count)
{
   if (count > 0)
   {
      uint16_t *dst16 = (uint16_t*)dst;
      int n  = (count + 7) >> 3;

      switch (count & 7)
      {
         case 0: do { *dst16++ = value;
         case 7:      *dst16++ = value;
         case 6:      *dst16++ = value;
         case 5:      *dst16++ = value;
         case 4:      *dst16++ = value;
         case 3:      *dst16++ = value;
         case 2:      *dst16++ = value;
         case 1:      *dst16++ = value; } while (--n);
      }

      return dst16;
   }

   return dst;
}

static void draw_rect(int x1, int y1, int x2, int y2, uint16_t color)
{
   uint16_t *pos = NULL;
   int height, width;

   clip_pos(&x1, &y1);
   clip_pos(&x2, &y2);

   if (x2 - x1 <= 0 || y2 - y1 <= 0)
      return;

   height = y2 - y1;
   width  = x2 - x1;

   pos = &g.fb[y1 * SCREEN_MAX_POS + x1];

   /* use system's memset for black and white */
   if (color == 0xffff || color == 0)
   {
      width <<= 1;
      color >>= 8;

      do
      {
         memset(pos, color, width);

         pos += SCREEN_MAX_POS;
      } while (--height);
   }
   else
   {
      do
      {
         memset16(pos, color, width);

         pos += SCREEN_MAX_POS;
      } while (--height);
   }
}

static void draw_square(int x, int y, int side, uint16_t color)
{
   draw_rect(x, y, x+side, y+side, color);
}

void draw_str(const ttf2c_font *font, int x1, int y1, unsigned orig, uint16_t color, const char *str)
{
   uint8_t sr, sg, sb;

   int line_width  = ttf2c_get_width(font, str);
   int line_height = font->line_height;

   if (orig & XORIG_RIGHT)
      x1 -= line_width;
   else if (orig & XORIG_CENTER)
      x1 -= line_width / 2;

   if (orig & YORIG_BOTTOM)
      y1 -= line_height;
   else if (orig & YORIG_CENTER)
      y1 -= line_height / 2;

   int x2 = x1 + ttf2c_get_width(font, str);
   int y2 = y1 + line_height;

   clip_pos(&x1, &y1);
   clip_pos(&x2, &y2);

   if (x2 - x1 <= 0 || y2 - y1 <= 0 || !*str)
      return;
   rgb565_decompose(color, &sr, &sg, &sb);

   do {
      const ttf2c_glyph *glyph = &font->glyphs[(unsigned)*str++];

      if (glyph->width * glyph->height && glyph->bitmap)
      {
         int x, y;
         int target_y = y1 + font->ascent + glyph->off_y - font->descent;
         int height = MIN(glyph->height, (y2 - y1));
         int width  = glyph->width;

         for (y = 0; y < height; ++y)
         {
            const uint8_t *src = &glyph->bitmap[y * width];
            uint16_t      *dst = &g.fb[target_y * SCREEN_MAX_POS + x1];
            for (x = 0; x < width; ++x)
            {
               uint8_t dr, dg, db;
               rgb565_decompose(*dst, &dr, &dg, &db);
               *dst = rgb565(
                        sr * *src / 255 + dr * (255 - *src) / 255,
                        sg * *src / 255 + dg * (255 - *src) / 255,
                        sb * *src / 255 + db * (255 - *src) / 255);
               ++dst;
               ++src;
            }
            target_y++;
         }
      }

      x1 += glyph->adv_x;

      if (glyph->adv_x <= glyph->width)
         x1++;

   } while (*str);
}

/* UI CODE   ---------------------------------------------------------------- */
static void ui_begin(void)
{
   g.ui.hot = 0;
}

static void ui_end(void)
{
   if (!g.mouse.left && !g.mouse.right)
      g.ui.active = 0;
   else if (g.ui.active == 0)
      g.ui.active = -1;
}

static bool ui_mouse_over(int x1, int y1, int x2, int y2)
{
   unsigned mx = g.mouse.x;
   unsigned my = g.mouse.y;

   return mx >= x1 && mx <= x2 && my >= y1 && my <= y2;
}

static bool ui_mouse_down(bool left, bool right)
{
   return (left && g.mouse.left)
         || (right && g.mouse.right);
}

static bool ui_mouse_up(bool left, bool right)
{
   return (left && !g.mouse.left && g.mouse.prev_left)
         || (right && !g.mouse.right && g.mouse.prev_right);
}

static void ui_check_rect_hotness(unsigned id, int x1, int y1, int x2, int y2)
{
   bool inside = ui_mouse_over(x1, y1, x2, y2);

   if (inside)
   {
      g.ui.last_hot = g.ui.hot;
      g.ui.hot      = id;
   }
}

static void ui_check_focus(unsigned id)
{
   if (g.ui.focus == 0)
      g.ui.focus = id;
}

static bool ui_button_down(unsigned id, bool left, bool right, int x1, int y1, int x2, int y2)
{
   bool result = false;
   ui_check_rect_hotness(id, x1, y1, x2, y2);

   if (g.ui.hot == id && ui_mouse_down(left, right))
   {
      g.ui.last_focus = g.ui.focus;
      g.ui.focus      = id;
      g.ui.active     = id;
      result          = true;
   }

   return result;
}

static bool ui_button_up(unsigned id, bool left, bool right, int x1, int y1, int x2, int y2)
{
   bool result = false;
   ui_check_rect_hotness(id, x1, y1, x2, y2);

   if (g.ui.active == id && ui_mouse_up(left, right))
   {
      if (g.ui.hot == id)
         result = true;

      g.ui.active = 0;
   }

   ui_button_down(id, left, right, x1, y1, x2, y2);

   return result;
}

static int ui_joy_ab(unsigned id)
{
   int result = 0;

   if (!g.ui.focus)
      g.ui.focus = id;

   if (g.ui.focus == id)
      result = g.joy.a - g.joy.b;

   return result;
}

/* GRID CODE ---------------------------------------------------------------- */

static bool grid_valid_cell(int row, int col)
{
   return row >= 0 && row < GRID_ROWS && col >= 0 && col < GRID_COLS;
}

static void grid_clip(int *row, int *col)
{
   if (row)
      *row = (*row < 0) ? 0 : ((*row >= GRID_ROWS) ? GRID_ROWS - 1 : *row);
   if (col)
      *col = (*col < 0) ? 0 : ((*col >= GRID_COLS) ? GRID_COLS - 1 : *col);
}

static void grid_to_screen(int row, int col, int *x, int *y)
{
   if (!grid_valid_cell(row, col))
   {
      if (x)
         *x = INT_MIN;
      if (y)
         *y = INT_MIN;
   }
   if (x)
      *x = GRID_X + col * CELL_SIZE + col + (col / GRID_GROUP_SIZE);
   if (y)
      *y = GRID_Y + row * CELL_SIZE + row + (row / GRID_GROUP_SIZE);
}

static void grid_from_screen(int x, int y, int *row, int *col)
{
   /* many thanks to heez for helping me with the math */
   if (row)
   {
      *row = (GRID_GROUP_SIZE * (y - GRID_Y)) / (GRID_GROUP_SIZE * CELL_SIZE + 4);
      if (*row < 0 || *row >= GRID_COLS)
         *row = INT_MIN;
   }

   if (col)
   {
      *col = (GRID_GROUP_SIZE * (x - GRID_X)) / (GRID_GROUP_SIZE * CELL_SIZE + 4);

      if (*col < 0 || *col >= GRID_COLS)
         *col = INT_MIN;
   }
}

static uint16_t grid_cell_color(grid_t grid, int row, int col)
{
   if (!grid_valid_cell(row, col))
      return rgb565(255, 0, 0);

   cell_state_t val = grid[row][col];
   switch (val)
   {
      case CELL_CLEAR:
      case CELL_SET:
         return !val * ~0;
      case CELL_DISABLED:
         return rgb565(127, 127, 127);
      case CELL_WRONG:
         return rgb565(255, 0, 0);
      default:
         return rgb565(0, 255, 0);
   }
}

static bool ui_grid_cell(grid_t grid, int row, int col, bool *left_down)
{
   unsigned id = (uintptr_t)&grid[row][col];
   bool result;
   int joy = 0;
   int x, y;

   grid_to_screen(row, col, &x, &y);

   result = ui_button_up(id, true, true, x, y, x+CELL_SIZE, y+CELL_SIZE);
   joy    = ui_joy_ab(id);

   if (g.ui.hot == id || g.ui.focus == id)
      draw_square(x-1, y-1, CELL_SIZE+2, rgb565(255, 0, 0));
   else if (g.ui.last_hot == id || g.ui.last_focus == id)
      draw_square(x-1, y-1, CELL_SIZE+2, rgb565(0, 255, 0));

   if (result)
   {
      *left_down = g.mouse.prev_left;
      g.grid.focus_x = col;
      g.grid.focus_y = row;
   }
   else if (joy)
   {
      *left_down = joy > 0;
      result = true;
   }

   draw_square(x, y, CELL_SIZE, grid_cell_color(grid, row, col));

   return result;
}

static void render_labels(void)
{
   draw_rect(0, 0, GRID_X, GRID_Y, ~0);
   draw_str(&font24, 10, 0, 0, rgb565(99, 0, 0), "CRUZES");
   draw_str(&font24, 10, GRID_Y - font24.line_height, 0, rgb565(99, 0, 0), "00:23:04");

   for (int col = 0; col < GRID_COLS; ++col)
   {
      int x, y;
      grid_to_screen(0, col, &x, &y);
      y -= 2;

      for (int index = GRID_ROWS-1; index >= 0; --index)
      {
         char tmp[10];

         if (!g.col_counts[col][index])
            continue;

         snprintf(tmp, sizeof(tmp), "%i", g.col_counts[col][index]);
         draw_str(&font10, x + CELL_SIZE/2, y, YORIG_BOTTOM | XORIG_CENTER, ~0, tmp);
         y -= 12;
      }
   }

   for (int row = 0; row < GRID_ROWS; ++row)
   {
      int x, y;
      grid_to_screen(row, 0, &x, &y);
      x -= 4;

      for (int index = GRID_COLS-1; index >= 0; --index)
      {
         char tmp[10];

         if (!g.row_counts[row][index])
            continue;


         snprintf(tmp, sizeof(tmp), "%i", g.row_counts[row][index]);
         draw_str(&font10, x, y + CELL_SIZE/2, YORIG_CENTER | XORIG_RIGHT, ~0, tmp);

         x -= ttf2c_get_width(&font10, tmp) + 6;
      }
   }
}

static void game_step()
{
   draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GREY);

   if (g.joy.moved && !g.mouse.moved)
   {
      g.grid.focus_x += g.joy.x;
      g.grid.focus_y += g.joy.y;
      grid_clip(&g.grid.focus_y, &g.grid.focus_x);

      g.ui.last_focus = g.ui.focus;
      g.ui.focus      = (uintptr_t)&g.grid.front[g.grid.focus_y][g.grid.focus_x];
   }

   for (int row = 0; row < GRID_ROWS; ++row)
   {
      for (int col = 0; col < GRID_COLS; ++col)
      {
         bool left_down = false;
         if (ui_grid_cell(g.grid.front, row, col, &left_down))
         {
            cell_state_t *cell = &g.grid.front[row][col];

            if (left_down)
            {
               if (*cell == CELL_CLEAR)
               {
                  *cell = CELL_SET;

                  if (g.grid.back[row][col] != CELL_SET)
                  {
                     *cell = CELL_WRONG;
                  }
               }
            }
            else
               switch (*cell)
               {
                  case CELL_CLEAR: *cell = CELL_DISABLED; break;
                  case CELL_SET:
                  case CELL_DISABLED: *cell = CELL_CLEAR; break;
                  default: break;
               }
         }
      }
   }

   render_labels();

   draw_str(&font16, SCREEN_WIDTH/2, SCREEN_HEIGHT-26, XORIG_CENTER, ~0, "Press START or click here for menu.");

   static uint8_t blinker = 0;
   draw_square(g.mouse.x, g.mouse.y, 4, rgb565(blinker, blinker, blinker));
   blinker += 20;
}


RETRO_API void retro_run(void)
{
   if (g.first_run)
   {
      g.first_run = false;
   }

   poll_cb();

   int delta_x, delta_y;

   delta_x = read_mouse(RETRO_DEVICE_ID_MOUSE_X);
   delta_y = read_mouse(RETRO_DEVICE_ID_MOUSE_Y);

   g.mouse.moved = delta_x || delta_y;
   g.mouse.x    += g.mouse.moved ? delta_x : 0;
   g.mouse.y    += g.mouse.moved ? delta_y : 0;

   g.mouse.left  = read_mouse(RETRO_DEVICE_ID_MOUSE_LEFT);
   g.mouse.right = read_mouse(RETRO_DEVICE_ID_MOUSE_RIGHT);

   delta_x = read_joypad(RETRO_DEVICE_ID_JOYPAD_RIGHT) - read_joypad(RETRO_DEVICE_ID_JOYPAD_LEFT);
   delta_y = read_joypad(RETRO_DEVICE_ID_JOYPAD_DOWN)  - read_joypad(RETRO_DEVICE_ID_JOYPAD_UP);

   g.joy.moved = (delta_x != g.joy.x) || (delta_y != g.joy.y);

   if (g.joy.moved)
   {
      g.joy.x = delta_x;
      g.joy.y = delta_y;
   }

   g.joy.a = read_joypad(RETRO_DEVICE_ID_JOYPAD_A)
         ||  read_joypad(RETRO_DEVICE_ID_JOYPAD_Y);

   g.joy.b = read_joypad(RETRO_DEVICE_ID_JOYPAD_B)
         ||  read_joypad(RETRO_DEVICE_ID_JOYPAD_X);

   clip_pos(&g.mouse.x, &g.mouse.y);

   ui_begin();

   game_step();

   ui_end();

   g.mouse.prev_left  = g.mouse.left;
   g.mouse.prev_right = g.mouse.right;

   video_cb(g.fb, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_PITCH);
}

static void load_challenge(const char *chal)
{
   if (strlen(chal) != GRID_COLS * GRID_ROWS)
      abort();

   memset(g.row_counts, 0, sizeof(g.row_counts));
   memset(g.col_counts, 0, sizeof(g.col_counts));

   for (int row = 0; row < GRID_ROWS; ++row)
      for (int col = 0; col < GRID_COLS; ++col)
         g.grid.back[row][col] = chal[row*GRID_COLS+col] != ' ' ? CELL_SET : CELL_CLEAR;


   for (int row = 0; row < GRID_ROWS; ++row)
   {
      bool was_set = false;
      int index = 0;
      for (int col = 0; col < GRID_COLS; ++col)
      {
         bool set = g.grid.back[row][col] == CELL_SET;

         if (set != was_set)
         {
            was_set = set;
            ++index;
         }

         if (set)
            ++g.row_counts[row][index];
      }
   }

   for (int col = 0; col < GRID_COLS; ++col)
   {
      bool was_set = false;
      int index = 0;
      for (int row = 0; row < GRID_ROWS; ++row)
      {
         bool set = g.grid.back[row][col] == CELL_SET;

         if (set != was_set)
         {
            was_set = set;
            ++index;
         }

         if (set)
            ++g.col_counts[col][index];
      }
   }
}

RETRO_API void retro_init(void)
{
   memset(&g, 0, sizeof(g));
   g.first_run = true;

   const char *chal =
         "                  "
         "  ##     #        "
         "  ##      #       "
         "       #   #      "
         "      # #  #      "
         "           #      "
         "  ##      #       "
         "  ##     #        "
         "                  ";

   load_challenge(chal);
}

RETRO_API void retro_deinit(void) { }


RETRO_API void retro_reset(void)
{

}

RETRO_API bool retro_load_game(const struct retro_game_info *game)
{
   uint_env(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, RETRO_PIXEL_FORMAT_RGB565);
   return true;
}

RETRO_API void retro_unload_game(void)
{

}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
{

}

RETRO_API void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name     = "cruzes";
   info->library_version  = "0.1";
   info->need_fullpath    = false;
   info->valid_extensions = "";
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width  = SCREEN_WIDTH;
   info->geometry.base_height = SCREEN_HEIGHT;
   info->geometry.aspect_ratio = (double)info->geometry.base_width / info->geometry.base_height;
   info->geometry.max_width = info->geometry.max_height = MAX(SCREEN_WIDTH, SCREEN_HEIGHT);
   info->timing.fps         = 60;
   info->timing.sample_rate = 44100;
}

RETRO_API unsigned retro_api_version(void) { return RETRO_API_VERSION; }

RETRO_API void retro_set_video_refresh(retro_video_refresh_t p) { video_cb = p; }
RETRO_API void retro_set_audio_sample(retro_audio_sample_t p) { }
RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t p) { }
RETRO_API void retro_set_input_poll(retro_input_poll_t p) { poll_cb = p; }
RETRO_API void retro_set_input_state(retro_input_state_t p) { input_cb = p; }
RETRO_API void retro_set_environment(retro_environment_t p)
{
   env_cb = p;
   bool_env(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, true);
}

RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
   return false;
}

RETRO_API unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }
RETRO_API size_t retro_serialize_size(void) { return 0; }
RETRO_API bool retro_serialize(void *data, size_t size) { return false; }
RETRO_API bool retro_unserialize(const void *data, size_t size) { return false; }
RETRO_API void retro_cheat_reset(void) { }
RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code) { }
RETRO_API void *retro_get_memory_data(unsigned id) { return NULL; }
RETRO_API size_t retro_get_memory_size(unsigned id) { return 0; }
