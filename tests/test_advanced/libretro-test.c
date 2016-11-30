static int numgroups=4;
static int groupsizes[]={5,2,1,1};

#define init_grp 1
#define init_sub 'a'

/* Also tests the following libretro env callbacks: */
//RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10
//RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME 18

//do not use the names as primary - they're an enum, the preprocessor can't read them
#define PIXFMT 0//RETRO_PIXEL_FORMAT_0RGB1555
//#define PIXFMT 1//RETRO_PIXEL_FORMAT_XRGB8888
//#define PIXFMT 2//RETRO_PIXEL_FORMAT_RGB565

#if PIXFMT==0
#define pixel_t uint16_t
#define p_red 0x7C00
#define p_grn 0x03E0
#define p_blu 0x001F
#define p_x 0
#define p_dark 0x3DEF
#endif
#if PIXFMT==1
#define pixel_t uint32_t
#define p_red 0xFF0000
#define p_grn 0x00FF00
#define p_blu 0x0000FF
#define p_x 0xFF000000
#define p_dark 0x7F7F7F
#endif
#if PIXFMT==2
#define pixel_t uint16_t
#define p_red 0xF800
#define p_grn 0x07E0
#define p_blu 0x001F
#define p_x 0
#define p_dark 0x7BEF
#endif
#define p_blk 0
#define p_yel (p_red|p_grn)
#define p_pur (p_red|p_blu)
#define p_tel (p_grn|p_blu)
#define p_wht (p_red|p_grn|p_blu)

#include "libretro.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define PI 3.14159265358979323846

static retro_log_printf_t log_cb;
static void log_null(enum retro_log_level level, const char *fmt, ...) {}

retro_environment_t environ_cb = NULL;
retro_video_refresh_t video_cb = NULL;
retro_audio_sample_t audio_cb = NULL;
retro_audio_sample_batch_t audio_batch_cb = NULL;
retro_input_poll_t poller_cb = NULL;
retro_input_state_t input_state_cb = NULL;

static struct
{
   int testgroup;
   int testsub;
   bool canchange;

   int frame;

   uint8_t test3a_activate;
   uint64_t test3a_last;
   uint16_t test4a[28*3];
} state;

uint16_t inpstate[2];
bool sound_enable;
pixel_t pixels[240*320];

void renderchr(pixel_t col, int chr, int x, int y);
void renderstr(pixel_t col, const char * str, int x, int y);
unsigned long crc32_calc(unsigned char *ptr, unsigned cnt, unsigned long crc);

#if defined(__unix__)
#include <time.h>
uint64_t cpu_features_get_time_usec()
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

#elif defined(_WIN32)
#include <windows.h>
uint64_t cpu_features_get_time_usec()
{
   static LARGE_INTEGER freq = {};
   if (!freq.QuadPart) QueryPerformanceFrequency(&freq);
   LARGE_INTEGER count;
   QueryPerformanceCounter(&count);

   return count.QuadPart * 1000000 / freq.QuadPart;
}
#else
uint64_t cpu_features_get_time_usec()
{
   static uint64_t last = 0;
   last += 1000000;
   return last;
}
#endif

static void test1a(void)
{
   unsigned x, y;

   for (y=0;y<240;y++)
   for (x=0;x<320;x++)
   {
      pixels[y*320+x] = p_wht;
   }

   for (y=80;y<160;y++)
   for (x=0;x<80;x++)
   {
      pixels[y*320 +  40+x] = p_red;
      pixels[y*320 + 120+x] = p_grn;
      pixels[y*320 + 200+x] = p_blu;
   }
}

static void test1b(void)
{
   unsigned x, y;

   for (x=0;x<320;x++)
   {
      if ((x+state.frame)%40 > 20)
         pixels[x] = p_blk;
      else
         pixels[x] = p_wht;
   }

   for (y=1;y<240;y++)
      memcpy(pixels+(320*y), pixels+0, sizeof(*pixels)*320);
}

static void test1c(void)
{
   unsigned x,y;
   for (y=0;y<240;y++)
   {
      for (x=0;x<320;x++)
      {
         if (((240-y)+state.frame)%30 > 15)
            pixels[y*320+x]=p_blk|p_x;
         else
            pixels[y*320+x]=p_wht|p_x;
      }
   }
}

static void test1d(void)
{
   int i;
   if (state.frame&1)
   {
      for (i=0;i<320*240;i++)
         pixels[i] = p_wht;
   }
   else
   {
      for (i=0;i<320*240;i++)
         pixels[i] = p_blk;
   }
}

static void test1e(void)
{
   unsigned x, y;
   for (y=0;y<240;y++)
   {
      for (x=0;x<320;x++)
      {
         if ((y^x) & 1)
            pixels[y*320+x] = p_wht;
         else
            pixels[y*320+x] = p_blk;
      }
   }
}

static void test1f(void)
{
   unsigned x, y;
   for (x=0;x<320*240;x++)
   {
      pixels[x] = p_wht;
   }

   for (x=0;x<320;x++)
   {
      pixels[        x] = ((x&1)?p_red:p_yel);
      pixels[239*320+x] = ((x&1)?p_yel:p_red);
   }

   for (y=0;y<240;y++)
   {
      pixels[y*320+  0] = ((y&1)?p_red:p_yel);
      pixels[y*320+319] = ((y&1)?p_yel:p_red);
   }
}

static void test2a(void)
{
   if (state.frame%240 >= 120)
   {
      memset(pixels, 0x00, sizeof(pixels));
      memset(pixels+(320*(state.frame%120)*2), 0xFF, sizeof(*pixels)*2);
      sound_enable = true;
   }
   else
   {
      memset(pixels, 0xFF, sizeof(pixels));
      memset(pixels+(320*(state.frame%120)*2), 0x00, sizeof(*pixels)*2);
      sound_enable = false;
   }
}

static void test2b(void)
{
   if (inpstate[0]&0x0F0F)
   {
      memset(pixels, 0x00, sizeof(pixels));
      sound_enable = true;
   }
   else
   {
      memset(pixels, 0xFF, sizeof(pixels));
      sound_enable = false;
   }
}

static void formatnum(char* out, unsigned int in)
{
   char tmp[16];
   int pos=0;

   sprintf(tmp, "%.10u", in);

   while (tmp[pos] == '0')
      pos++;
   *out++ = tmp[pos++];

   while (tmp[pos])
   {
      if (pos%3 == 1) *out++ = ',';
      *out++ = tmp[pos++];
   }
   *out='\0';
}

static size_t test_inputspeed(void)
{
   size_t calls = 0;

   uint64_t start = cpu_features_get_time_usec();
   unsigned iterlen = 32;
   uint64_t now;

   double seconds;

   while (true)
   {
      unsigned i;

      now = cpu_features_get_time_usec();
      if (now < start+10000 && iterlen<0x10000000) iterlen*=2; /* try to call the time function once per 10ms */
      if (now > start+2000000) break;

      for (i=0;i<iterlen;i++)
      {
         input_state_cb(((now^i)>>4)%2,
               RETRO_DEVICE_JOYPAD, 0, (now^i)%16);
         calls++;
      }
   }

   seconds = (double)(now-start) / 1000000.0;
   return calls/seconds;
}

static void test3a(void)
{
   unsigned i;
   if (state.test3a_activate == 1)
   {
      state.test3a_last = test_inputspeed();
      state.test3a_activate = 2;
   }

   if (state.test3a_activate == 0 && inpstate[0]&0xFF0F)
      state.test3a_activate = 1;
   if (state.test3a_activate == 2 && !inpstate[0])
      state.test3a_activate = 0;

   for (i=0;i<320*240;i++)
      pixels[i] = p_wht;

   if (state.test3a_activate == 1)
      renderstr(p_blk, "Running...", 8, 16);
   else if (state.test3a_last == 0)
      renderstr(p_blk, "Ready", 8, 16);
   else
   {
      char line[128];
      formatnum(line, state.test3a_last);
      strcat(line, " calls per second");
      renderstr(p_blk, line, 8, 16);

      formatnum(line, state.test3a_last/60);
      strcat(line, " calls per frame");
      renderstr(p_blk, line, 8, 24);
   }
}

static void test4a(void)
{
   uint16_t color;
   unsigned i;

   if (inpstate[0] != state.test4a[27*3+1] || inpstate[1] != state.test4a[27*3+2])
   {
      for (i=0;i<27*3;i++)
         state.test4a[0*3+i] = state.test4a[0*3+i+3];

      state.test4a[27*3+0] = state.frame+1;
      state.test4a[27*3+1] = inpstate[0];
      state.test4a[27*3+2] = inpstate[1];
   }

   color = (~crc32_calc((unsigned char*)state.test4a, 6*28, ~0U))&p_dark;

   for (i=0;i<320*240;i++)
      pixels[i] = color;

   for (i=0;i<28;i++)
   {
      if (state.test4a[i*3+0])
      {
         char line[17];
         sprintf(line, "%i: %.4X %.4X", state.test4a[i*3+0], state.test4a[i*3+1], state.test4a[i*3+2]);
         renderstr(p_wht, line, 8, 8+i*8);
      }
   }
}

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { poller_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   bool True = true;
   environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &True);

#if 0
   environ_cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &frametime_g);
   environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf);
#endif
}

void retro_init(void)
{
   struct retro_log_callback log;
   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = log_null;

   log_cb(RETRO_LOG_DEBUG, "0123 test");
   log_cb(RETRO_LOG_DEBUG, "%.3i%c t%st", 12, '3', "es");
}

void retro_deinit(void) {}
unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_get_system_info(struct retro_system_info *info)
{
   const struct retro_system_info myinfo = { "minir test core", "v1.00", "c|h", false, false };
   memcpy(info, &myinfo, sizeof(myinfo));
}

void retro_get_system_av_info(struct retro_system_av_info* info)
{
   static const struct retro_system_av_info myinfo = {
      { 320, 240, 320, 240, 0.0 },
      { 60.0, 30720.0 }
   };
   memcpy(info, &myinfo, sizeof(myinfo));
}

void retro_set_controller_port_device(unsigned port, unsigned device) {}

void retro_reset(void)
{
   memset(&state, 0, sizeof(state));
   state.testgroup = init_grp;
   state.testsub = init_sub;
}

void retro_run(void)
{
   unsigned i;

   char testid[3];

   poller_cb();
   inpstate[0] = 0x0000;
   inpstate[1] = 0x0000;

   for (i=0;i<16;i++)
   {
      inpstate[0] |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i))<<i;
      inpstate[1] |= (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, i))<<i;
   }
   sound_enable = false;

   if (state.canchange)
   {
      bool changed = false;
      if (inpstate[0]&(1<<RETRO_DEVICE_ID_JOYPAD_UP))
      {
         state.testgroup--;
         if (state.testgroup == 0) state.testgroup = numgroups;
         state.testsub = 'a';
         changed=true;
      }
      if (inpstate[0]&(1<<RETRO_DEVICE_ID_JOYPAD_DOWN))
      {
         state.testgroup++;
         if (state.testgroup-1 == numgroups) state.testgroup=1;
         state.testsub = 'a';
         changed = true;
      }
      if (inpstate[0]&(1<<RETRO_DEVICE_ID_JOYPAD_LEFT) && groupsizes[state.testgroup-1] != 1)
      {
         state.testsub--;
         if (state.testsub == 'a'-1) state.testsub=groupsizes[state.testgroup-1]+'a'-1;
         changed=true;
      }
      if (inpstate[0]&(1<<RETRO_DEVICE_ID_JOYPAD_RIGHT) && groupsizes[state.testgroup-1] != 1)
      {
         state.testsub++;
         if (state.testsub-1 == groupsizes[state.testgroup-1]+'a'-1) state.testsub='a';
         changed=true;
      }
      if (changed)
      {
         state.frame = 0;
      }
   }
   state.canchange = ((inpstate[0]&0x00F0)==0);

   if (state.testgroup == 1)
   {
      if (state.testsub == 'a')
         test1a();
      if (state.testsub == 'b')
         test1b();
      if (state.testsub == 'c')
         test1c();
      if (state.testsub == 'd')
         test1d();
      if (state.testsub == 'e')
         test1e();
      if (state.testsub == 'f')
         test1f();
   }
   if (state.testgroup == 2)
   {
      if (state.testsub == 'a')
         test2a();
      if (state.testsub == 'b')
         test2b();
   }
   if (state.testgroup == 3)
   {
      if (state.testsub == 'a')
         test3a();
   }
   if (state.testgroup == 4)
   {
      if (state.testsub=='a')
         test4a();
   }

   testid[0] = '0'+state.testgroup;
   testid[1] = state.testsub;
   testid[2] = '\0';
   renderstr(p_wht, testid, 7, 7);
   renderstr(p_wht, testid, 8, 7);
   renderstr(p_wht, testid, 9, 7);
   renderstr(p_wht, testid, 7, 8);
   renderstr(p_wht, testid, 9, 8);
   renderstr(p_wht, testid, 7, 9);
   renderstr(p_wht, testid, 8, 9);
   renderstr(p_wht, testid, 9, 9);
   renderstr(p_blk, testid, 8, 8);

   state.frame++;

   if (sound_enable)
   {
      int16_t data[8*64*2];
      for (i=0;i<8*64;i++)
      {
         data[i*2] = sin(((double)(state.frame*8*64 + i))/30720*2*PI * 440)*4096;
         data[i*2+1] = data[i*2];
      }
      for (i=0;i<8;i++)
      {
         audio_batch_cb(data+(i*128), 64);
      }
   }
   else
   {
      int16_t data[64*2];
      memset(data, 0, sizeof(data));
      for (i=0;i<8;i++) audio_batch_cb(data, 64);
   }

   video_cb(pixels, 320, 240, sizeof(pixel_t)*320);
}

size_t retro_serialize_size(void) { return sizeof(state); }
bool retro_serialize(void* data, size_t size)
{
   if (size<sizeof(state)) return false;
   memcpy(data, &state, sizeof(state));
   return true;
}
bool retro_unserialize(const void* data, size_t size)
{
   if (size<sizeof(state)) return false;
   memcpy(&state, data, sizeof(state));
   return true;
}

void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char* code) {}
bool retro_load_game(const struct retro_game_info* game)
{
   retro_reset();
   enum retro_pixel_format rgb565=(enum retro_pixel_format)PIXFMT;
   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565)) return false;
   return true;
}
bool retro_load_game_special(unsigned game_type, const struct retro_game_info* info, size_t num_info) { return false; }
void retro_unload_game(void) {}
unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }
void* retro_get_memory_data(unsigned id) { return NULL; }
size_t retro_get_memory_size(unsigned id) { return 0; }

/* blatant zsnes copypasta */
unsigned char zfont[]={
0,0,0,0,0,
0x70,0x98,0xA8,0xC8,0x70, 0x20,0x60,0x20,0x20,0x70, 0x70,0x88,0x30,0x40,0xF8, 0x70,0x88,0x30,0x88,0x70,
0x50,0x90,0xF8,0x10,0x10, 0xF8,0x80,0xF0,0x08,0xF0, 0x70,0x80,0xF0,0x88,0x70, 0xF8,0x08,0x10,0x10,0x10,
0x70,0x88,0x70,0x88,0x70, 0x70,0x88,0x78,0x08,0x70, 0x70,0x88,0xF8,0x88,0x88, 0xF0,0x88,0xF0,0x88,0xF0,
0x70,0x88,0x80,0x88,0x70, 0xF0,0x88,0x88,0x88,0xF0, 0xF8,0x80,0xF0,0x80,0xF8, 0xF8,0x80,0xF0,0x80,0x80,
0x78,0x80,0x98,0x88,0x70, 0x88,0x88,0xF8,0x88,0x88, 0xF8,0x20,0x20,0x20,0xF8, 0x78,0x10,0x10,0x90,0x60,
0x90,0xA0,0xE0,0x90,0x88, 0x80,0x80,0x80,0x80,0xF8, 0xD8,0xA8,0xA8,0xA8,0x88, 0xC8,0xA8,0xA8,0xA8,0x98,
0x70,0x88,0x88,0x88,0x70, 0xF0,0x88,0xF0,0x80,0x80, 0x70,0x88,0xA8,0x90,0x68, 0xF0,0x88,0xF0,0x90,0x88,
0x78,0x80,0x70,0x08,0xF0, 0xF8,0x20,0x20,0x20,0x20, 0x88,0x88,0x88,0x88,0x70, 0x88,0x88,0x50,0x50,0x20,
0x88,0xA8,0xA8,0xA8,0x50, 0x88,0x50,0x20,0x50,0x88, 0x88,0x50,0x20,0x20,0x20, 0xF8,0x10,0x20,0x40,0xF8,
0x00,0x00,0xF8,0x00,0x00, 0x00,0x00,0x00,0x00,0xF8, 0x68,0x90,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x20,
0x08,0x10,0x20,0x40,0x80, 0x10,0x20,0x40,0x20,0x10, 0x40,0x20,0x10,0x20,0x40, 0x70,0x40,0x40,0x40,0x70,
0x70,0x10,0x10,0x10,0x70, 0x00,0x20,0x00,0x20,0x00, 0x60,0x98,0x70,0x98,0x68, 0x20,0x20,0xA8,0x70,0x20,
0x50,0xF8,0x50,0xF8,0x50, 0x00,0xF8,0x00,0xF8,0x00, 0x48,0x90,0x00,0x00,0x00, 0x80,0x40,0x20,0x10,0x08,
0xA8,0x70,0xF8,0x70,0xA8, 0x70,0x88,0x30,0x00,0x20, 0x88,0x10,0x20,0x40,0x88, 0x20,0x20,0xF8,0x20,0x20,
0x00,0x00,0x00,0x20,0x40, 0x30,0x40,0x40,0x40,0x30, 0x60,0x10,0x10,0x10,0x60, 0x70,0x98,0xB8,0x80,0x70,
0x20,0x40,0x00,0x00,0x00, 0x20,0x20,0x20,0x00,0x20, 0x78,0xA0,0x70,0x28,0xF0, 0x00,0x20,0x00,0x20,0x40,
0x40,0x20,0x00,0x00,0x00, 0x20,0x50,0x00,0x00,0x00, 0x30,0x40,0xC0,0x40,0x30, 0x60,0x10,0x18,0x10,0x60,
0x20,0x20,0x70,0x70,0xF8, 0xF8,0x70,0x70,0x20,0x20, 0x08,0x38,0xF8,0x38,0x08, 0x80,0xE0,0xF8,0xE0,0x80,
0x20,0x60,0xF8,0x60,0x20, 0x38,0x20,0x30,0x08,0xB0, 0xFC,0x84,0xFC,0x00,0x00, 0x00,0xFC,0x00,0x00,0x00,
0xF8,0x88,0x88,0x88,0xF8,
};

unsigned char convtable[256]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x3E,0x33,0x31,0x3F,0x37,0x2F,0x3D,0x3A,0x3B,0x35,0x38,0x39,0x25,0x28,0x29,
0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x2E,0x40,0x2A,0x32,0x2B,0x36,
0x3C,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x2C,0x34,0x2D,0x42,0x26,
0x41,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x43,0x00,0x44,0x27,0x00,
0x0D,0x1F,0x0F,0x0B,0x0B,0x0B,0x0B,0x0D,0x0F,0x0F,0x0F,0x13,0x13,0x13,0x0B,0x0B,
0x0F,0x0B,0x0B,0x19,0x19,0x19,0x1F,0x1F,0x23,0x19,0x1F,0x0D,0x10,0x23,0x1A,0x10,
0x0B,0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,
0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,
0x6D,0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,
0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4D,0x4C,0x4B,0x4A,0x45,0x46,0x47,0x48,0x49,
};

void renderchr(pixel_t col, int chr, int x, int y)
{
   int ix;
   int iy;

   for (iy=0;iy<5;iy++)
   {
      for (ix=0;ix<8;ix++)
      {
         if ((zfont[convtable[chr]*5 + iy]>>ix)&1)
            pixels[(y+iy)*320 + x+(ix^7)]=col;
      }
   }
}

void renderstr(pixel_t col, const char * str, int x, int y)
{
   int i;
   for (i=0;str[i];i++)
      renderchr(col, str[i], x+i*8, y);
}

/* Karl Malbrain's compact CRC-32.
 * See "A compact CCITT crc16 and crc32 C implementation that balances processor cache usage against speed":
 * http://www.geocities.ws/malbrain/
 */

unsigned long Crc32[] = {
0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

unsigned long crc32_calc (unsigned char *ptr, unsigned cnt, unsigned long crc)
{
    while( cnt-- ) {
        crc = ( crc >> 4 ) ^ Crc32[(crc & 0xf) ^ (*ptr & 0xf)];
        crc = ( crc >> 4 ) ^ Crc32[(crc & 0xf) ^ (*ptr++ >> 4)];
    }

    return crc;
}

