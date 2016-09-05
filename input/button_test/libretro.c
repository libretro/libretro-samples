#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#if defined(__CELLOS_LV2__) && !defined(__PSL1GHT__)
#include <sys/timer.h>
#elif defined(XENON)
#include <time/time.h>
#elif defined(GEKKO) || defined(__PSL1GHT__) || defined(__QNX__)
#include <unistd.h>
#elif defined(PSP)
#include <pspthreadman.h> 
#elif defined(VITA)
#include <psp2/kernel/threadmgr.h>
#elif defined(_3DS)
#include <3ds.h>
#else
#include <time.h>
#endif

#include "libretro.h"
#include "remotepad.h"

#ifdef RARCH_INTERNAL
#include "internal_cores.h"
#define NETRETROPAD_CORE_PREFIX(s) libretro_netretropad_##s
#else
#define NETRETROPAD_CORE_PREFIX(s) s
#endif

#define DESC_NUM_PORTS(desc) ((desc)->port_max - (desc)->port_min + 1)
#define DESC_NUM_INDICES(desc) ((desc)->index_max - (desc)->index_min + 1)
#define DESC_NUM_IDS(desc) ((desc)->id_max - (desc)->id_min + 1)

#define DESC_OFFSET(desc, port, index, id) ( \
   port * ((desc)->index_max - (desc)->index_min + 1) * ((desc)->id_max - (desc)->id_min + 1) + \
   index * ((desc)->id_max - (desc)->id_min + 1) + \
   id \
)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#if defined(_WIN32) || defined(__INTEL_COMPILER)
#define INLINE __inline
#elif defined(__STDC_VERSION__) && __STDC_VERSION__>=199901L
#define INLINE inline
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#define INLINE
#endif

/**
 * retro_sleep:
 * @msec         : amount in milliseconds to sleep
 *
 * Sleeps for a specified amount of milliseconds (@msec).
 **/
static INLINE void retro_sleep(unsigned msec)
{
#if defined(__CELLOS_LV2__) && !defined(__PSL1GHT__)
   sys_timer_usleep(1000 * msec);
#elif defined(PSP) || defined(VITA)
   sceKernelDelayThread(1000 * msec);
#elif defined(_3DS)
   svcSleepThread(1000000 * (s64)msec);
#elif defined(_WIN32)
   Sleep(msec);
#elif defined(XENON)
   udelay(1000 * msec);
#elif defined(GEKKO) || defined(__PSL1GHT__) || defined(__QNX__)
   usleep(1000 * msec);
#else
   struct timespec tv = {0};
   tv.tv_sec = msec / 1000;
   tv.tv_nsec = (msec % 1000) * 1000000;
   nanosleep(&tv, NULL);
#endif
}

struct descriptor {
   int device;
   int port_min;
   int port_max;
   int index_min;
   int index_max;
   int id_min;
   int id_max;
   uint16_t *value;
};

struct remote_joypad_message {
   int port;
   int device;
   int index;
   int id;
   uint16_t state;
};

static struct retro_log_callback logger;

static retro_log_printf_t NETRETROPAD_CORE_PREFIX(log_cb);
static retro_video_refresh_t NETRETROPAD_CORE_PREFIX(video_cb);
static retro_audio_sample_t NETRETROPAD_CORE_PREFIX(audio_cb);
static retro_audio_sample_batch_t NETRETROPAD_CORE_PREFIX(audio_batch_cb);
static retro_environment_t NETRETROPAD_CORE_PREFIX(environ_cb);
static retro_input_poll_t NETRETROPAD_CORE_PREFIX(input_poll_cb);
static retro_input_state_t NETRETROPAD_CORE_PREFIX(input_state_cb);

static uint16_t *frame_buf;

static struct descriptor joypad = {
   .device = RETRO_DEVICE_JOYPAD,
   .port_min = 0,
   .port_max = 0,
   .index_min = 0,
   .index_max = 0,
   .id_min = RETRO_DEVICE_ID_JOYPAD_B,
   .id_max = RETRO_DEVICE_ID_JOYPAD_R3
};

static struct descriptor analog = {
   .device = RETRO_DEVICE_ANALOG,
   .port_min = 0,
   .port_max = 0,
   .index_min = RETRO_DEVICE_INDEX_ANALOG_LEFT,
   .index_max = RETRO_DEVICE_INDEX_ANALOG_RIGHT,
   .id_min = RETRO_DEVICE_ID_ANALOG_X,
   .id_max = RETRO_DEVICE_ID_ANALOG_Y
};

static struct descriptor *descriptors[] = {
   &joypad,
   &analog
};

void NETRETROPAD_CORE_PREFIX(retro_init)(void)
{
   struct descriptor *desc;
   int size;
   int i;

   frame_buf = (uint16_t*)calloc(320 * 240, sizeof(uint16_t));

   if (frame_buf)
   {
      uint16_t *pixel = frame_buf + 49 * 320 + 32;

      for (unsigned rle = 0; rle < sizeof(body); )
      {
         uint16_t color = 0;

         for (unsigned runs = body[rle++]; runs > 0; runs--)
         {
            for (unsigned count = body[rle++]; count > 0; count--)
            {
               *pixel++ = color;
            }

            color = 0x4208 - color;
         }

         pixel += 65;
      }
   }

   /* Allocate descriptor values */
   for (i = 0; i < ARRAY_SIZE(descriptors); i++) {
      desc = descriptors[i];
      size = DESC_NUM_PORTS(desc) * DESC_NUM_INDICES(desc) * DESC_NUM_IDS(desc);
      descriptors[i]->value = (uint16_t*)calloc(size, sizeof(uint16_t));
   }
}

void NETRETROPAD_CORE_PREFIX(retro_deinit)(void)
{
   int i;

   if (frame_buf)
      free(frame_buf);
   frame_buf = NULL;

   /* Free descriptor values */
   for (i = 0; i < ARRAY_SIZE(descriptors); i++) {
      free(descriptors[i]->value);
      descriptors[i]->value = NULL;
   }
}

unsigned NETRETROPAD_CORE_PREFIX(retro_api_version)(void)
{
   return RETRO_API_VERSION;
}

void NETRETROPAD_CORE_PREFIX(retro_set_controller_port_device)(
      unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

void NETRETROPAD_CORE_PREFIX(retro_get_system_info)(
      struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "RetroPad Remote";
   info->library_version  = "0.01";
   info->need_fullpath    = false;
   info->valid_extensions = ""; /* Nothing. */
}

void NETRETROPAD_CORE_PREFIX(retro_get_system_av_info)(
      struct retro_system_av_info *info)
{
   info->timing.fps = 60.0;
   info->timing.sample_rate = 30000.0;

   info->geometry.base_width  = 320;
   info->geometry.base_height = 240;
   info->geometry.max_width   = 320;
   info->geometry.max_height  = 240;
   info->geometry.aspect_ratio = 4.0 / 3.0;
}

void NETRETROPAD_CORE_PREFIX(retro_set_environment)(retro_environment_t cb)
{
   static const struct retro_variable vars[] = {
      { NULL, NULL },
   };
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);


   NETRETROPAD_CORE_PREFIX(environ_cb) = cb;
   bool no_content = true;
   cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

   NETRETROPAD_CORE_PREFIX(environ_cb)(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);

   if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logger))
      NETRETROPAD_CORE_PREFIX(log_cb) = logger.log;
}

static void netretropad_check_variables(void)
{
}

void NETRETROPAD_CORE_PREFIX(retro_set_audio_sample)(retro_audio_sample_t cb)
{
   NETRETROPAD_CORE_PREFIX(audio_cb) = cb;
}

void NETRETROPAD_CORE_PREFIX(retro_set_audio_sample_batch)(
      retro_audio_sample_batch_t cb)
{
   NETRETROPAD_CORE_PREFIX(audio_batch_cb) = cb;
}

void NETRETROPAD_CORE_PREFIX(retro_set_input_poll)(retro_input_poll_t cb)
{
   NETRETROPAD_CORE_PREFIX(input_poll_cb) = cb;
}

void NETRETROPAD_CORE_PREFIX(retro_set_input_state)(retro_input_state_t cb)
{
   NETRETROPAD_CORE_PREFIX(input_state_cb) = cb;
}

void NETRETROPAD_CORE_PREFIX(retro_set_video_refresh)(retro_video_refresh_t cb)
{
   NETRETROPAD_CORE_PREFIX(video_cb) = cb;
}

void NETRETROPAD_CORE_PREFIX(retro_reset)(void)
{}

static void retropad_update_input(void)
{
   struct descriptor *desc;
   struct remote_joypad_message msg;
   uint16_t state;
   uint16_t old;
   int offset;
   int port;
   int index;
   int id;
   int i;

   /* Poll input */
   NETRETROPAD_CORE_PREFIX(input_poll_cb)();

   /* Parse descriptors */
   for (i = 0; i < ARRAY_SIZE(descriptors); i++) {
      /* Get current descriptor */
      desc = descriptors[i];

      /* Go through range of ports/indices/IDs */
      for (port = desc->port_min; port <= desc->port_max; port++)
         for (index = desc->index_min; index <= desc->index_max; index++)
            for (id = desc->id_min; id <= desc->id_max; id++) {
               /* Compute offset into array */
               offset = DESC_OFFSET(desc, port, index, id);

               /* Get old state */
               old = desc->value[offset];

               /* Get new state */
               state = NETRETROPAD_CORE_PREFIX(input_state_cb)(
                  port,
                  desc->device,
                  index,
                  id);

               /* Continue if state is unchanged */
               if (state == old)
                  continue;

               /* Update state */
               desc->value[offset] = state;

               /* Attempt to send updated state */
               msg.port = port;
               msg.device = desc->device;
               msg.index = index;
               msg.id = id;
               msg.state = state;
            }
   }
}

void NETRETROPAD_CORE_PREFIX(retro_run)(void)
{
   unsigned rle, runs;
   uint16_t *pixel      = NULL;
   unsigned input_state = 0;
   int offset;
   int i;

   /* Update input states and send them if needed */
   retropad_update_input();

   /* Combine RetroPad input states into one value */
   for (i = joypad.id_min; i <= joypad.id_max; i++) {
      offset = DESC_OFFSET(&joypad, 0, 0, i);
      if (joypad.value[offset])
         input_state |= 1 << i;
   }

   pixel = frame_buf + 49 * 320 + 32;

   for (rle = 0; rle < sizeof(retropad_buttons); )
   {
      char paint = 0;

      for (runs = retropad_buttons[rle++]; runs > 0; runs--)
      {
         unsigned button = paint ? 1 << retropad_buttons[rle++] : 0;

         if (paint)
         {
            unsigned count;
            uint16_t color = (input_state & button) ? 0x0500 : 0xffff;

            for (count = retropad_buttons[rle++]; count > 0; count--)
               *pixel++ = color;
         }
         else
            pixel += retropad_buttons[rle++];

         paint = !paint;
      }

      pixel += 65;
   }


   NETRETROPAD_CORE_PREFIX(video_cb)(frame_buf, 320, 240, 640);

   retro_sleep(4);
}

bool NETRETROPAD_CORE_PREFIX(retro_load_game)(const struct retro_game_info *info)
{
   netretropad_check_variables();

   return true;
}

void NETRETROPAD_CORE_PREFIX(retro_unload_game)(void)
{}

unsigned NETRETROPAD_CORE_PREFIX(retro_get_region)(void)
{
   return RETRO_REGION_NTSC;
}

bool NETRETROPAD_CORE_PREFIX(retro_load_game_special)(unsigned type,
      const struct retro_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t NETRETROPAD_CORE_PREFIX(retro_serialize_size)(void)
{
   return 0;
}

bool NETRETROPAD_CORE_PREFIX(retro_serialize)(void *data, size_t size)
{
   (void)data;
   (void)size;
   return false;
}

bool NETRETROPAD_CORE_PREFIX(retro_unserialize)(const void *data,
      size_t size)
{
   (void)data;
   (void)size;
   return false;
}

void *NETRETROPAD_CORE_PREFIX(retro_get_memory_data)(unsigned id)
{
   (void)id;
   return NULL;
}

size_t NETRETROPAD_CORE_PREFIX(retro_get_memory_size)(unsigned id)
{
   (void)id;
   return 0;
}

void NETRETROPAD_CORE_PREFIX(retro_cheat_reset)(void)
{}

void NETRETROPAD_CORE_PREFIX(retro_cheat_set)(unsigned idx,
      bool enabled, const char *code)
{
   (void)idx;
   (void)enabled;
   (void)code;
}
