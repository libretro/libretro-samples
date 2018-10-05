
#include "libretro.h"

#include <stdint.h>
#include <stdlib.h>

#define WIDHT  960
#define HEIGHT 540
#define FPS    60

typedef enum
{
   MIDI_TEST_OFF,
   MIDI_TEST_METRONOME,
   MIDI_TEST_LOOPBACK,
   MIDI_TEST_KEYBOARD
} midi_test_t;

retro_environment_t g_env;
struct retro_log_callback * g_log_cb;
struct retro_midi_interface * g_midi_iface;
uint16_t * g_video_frame;
retro_video_refresh_t g_video_refresh;
retro_input_poll_t g_input_poll;
retro_input_state_t g_input_state;
midi_test_t g_midi_test;

void run_metronome_test(void)
{
   static unsigned i;
   static int frames_elapsed;
   static uint8_t data[2][3] =
   {
      { 0x99, 35, 127 },
      { 0x89, 35, 127 }
   };

   if (!g_midi_iface->output_enabled())
      return;

   if (frames_elapsed == 0)
   {
      if ((!g_midi_iface->write(data[i][0], 250000) ||
            !g_midi_iface->write(data[i][1], 320) ||
            !g_midi_iface->write(data[i][2], 320)) && g_log_cb)
         g_log_cb->log(RETRO_LOG_ERROR, "Metronome Test: write error.\n");

      i ^= 1;
   }

   if (++frames_elapsed == FPS / 4)
      frames_elapsed = 0;
}

void run_loopback_test(void)
{
   uint8_t data;

   if (!g_midi_iface->input_enabled())
      return;

   while (g_midi_iface->read(&data))
   {
      if (!g_midi_iface->output_enabled())
         continue;

      if (!g_midi_iface->write(data, 0) && g_log_cb)
         g_log_cb->log(RETRO_LOG_ERROR, "Loopback Test: write error.\n");
   }
}

void run_keyboard_test(void)
{
   uint8_t i;
   static uint16_t old_keys_state;
   uint16_t new_keys_state = 0;

   if (!g_midi_iface->output_enabled())
      return;

   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_z) ? 1 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_s) ? 2 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_x) ? 4 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_d) ? 8 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_c) ? 16 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_v) ? 32 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_g) ? 64 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_b) ? 128 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_h) ? 256 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_n) ? 512 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_j) ? 1024 : 0;
   new_keys_state |= g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_m) ? 2048 : 0;

   if (new_keys_state == old_keys_state)
      return;

   for (i = 0; i < 12; ++i)
   {
      if ((new_keys_state & (1 << i)) != (old_keys_state & (1 << i)))
      {
         bool key_down = new_keys_state & (1 << i);

         if ((!g_midi_iface->write(key_down ? 0x90 : 0x80, 0) ||
               !g_midi_iface->write(60 + i, 0) ||
               !g_midi_iface->write(127, 0)) && g_log_cb)
            g_log_cb->log(RETRO_LOG_ERROR, "Keyboard Test: write error.\n");
      }
   }

   old_keys_state = new_keys_state;
}

void retro_set_environment(retro_environment_t env)
{
   bool support = true;

   env(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &support);

   g_env = env;
}

void retro_set_video_refresh(retro_video_refresh_t video_refresh)
{
   g_video_refresh = video_refresh;
}

void retro_set_audio_sample(retro_audio_sample_t p)
{
   (void)p;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t p)
{
   (void)p;
}

void retro_set_input_poll(retro_input_poll_t input_poll)
{
   g_input_poll = input_poll;
}

void retro_set_input_state(retro_input_state_t input_state)
{
   g_input_state = input_state;
}

void retro_init(void)
{
   static struct retro_log_callback callback;
   static struct retro_midi_interface interface;

   if (g_env(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &callback))
      g_log_cb = &callback;
   else
      g_log_cb = NULL;

   if(g_env(RETRO_ENVIRONMENT_GET_MIDI_INTERFACE, &interface))
      g_midi_iface = &interface;
   else
   {
      g_midi_iface = NULL;
      if (g_log_cb)
         g_log_cb->log(RETRO_LOG_ERROR, "MIDI interface unavailable.\n");
   }
}

void retro_deinit(void)
{
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name     = "MIDI Test";
   info->library_version  = "v0.1.0";
   info->valid_extensions = "";
   info->block_extract    = false;
   info->need_fullpath    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.aspect_ratio = (float)WIDHT / (float)HEIGHT;
   info->geometry.base_height  = HEIGHT;
   info->geometry.base_width   = WIDHT;
   info->geometry.max_height   = HEIGHT;
   info->geometry.max_width    = WIDHT;
   info->timing.fps            = FPS;
   info->timing.sample_rate    = 48000.0;
}

void retro_set_controller_port_device(unsigned p1, unsigned p2)
{
   (void)p1;
   (void)p2;
}

void retro_reset(void)
{
}

void retro_run(void)
{
   midi_test_t old_midi_test = g_midi_test;

   g_input_poll();

   if (g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_0))
      g_midi_test = MIDI_TEST_OFF;
   else if (g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_1))
      g_midi_test = MIDI_TEST_METRONOME;
   else if (g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_2))
      g_midi_test = MIDI_TEST_LOOPBACK;
   else if (g_input_state(0, RETRO_DEVICE_KEYBOARD, 0, RETROK_3))
      g_midi_test = MIDI_TEST_KEYBOARD;

   if (g_midi_test != old_midi_test)
   {
      struct retro_message message;

      if (g_midi_test == MIDI_TEST_METRONOME)
         message.msg = "Metronome test: ~120 BPM, note B1, channel 10";
      else if (g_midi_test == MIDI_TEST_LOOPBACK)
         message.msg = "Loopback test: input to output";
      else if (g_midi_test == MIDI_TEST_KEYBOARD)
         message.msg = "Keyboard test: keys Z..M, notes C4..B4, channel 1";
      else
         message.msg = "MIDI test off";

      message.frames = FPS * 4;
      g_env(RETRO_ENVIRONMENT_SET_MESSAGE, &message);
   }

   if (g_midi_iface)
   {
      if (g_midi_test == MIDI_TEST_METRONOME)
         run_metronome_test();
      else if (g_midi_test == MIDI_TEST_LOOPBACK)
         run_loopback_test();
      else if (g_midi_test == MIDI_TEST_KEYBOARD)
         run_keyboard_test();

      g_midi_iface->flush();
   }

   g_video_refresh(g_video_frame, WIDHT, HEIGHT, sizeof(uint16_t) * WIDHT);
}

size_t retro_serialize_size()
{
   return 0;
}

bool retro_serialize(void *p1, size_t p2)
{
   (void)p1;
   (void)p2;

   return false;
}

bool retro_unserialize(const void *p1, size_t p2)
{
   (void)p1;
   (void)p2;

   return false;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned p1, bool p2, const char *p3)
{
   (void)p1;
   (void)p2;
   (void)p3;
}

bool retro_load_game(const struct retro_game_info *p)
{
   (void)p;

   g_video_frame = (uint16_t *)calloc(sizeof(uint16_t) * WIDHT * HEIGHT, 1);
   g_midi_test   = MIDI_TEST_OFF;

   return g_video_frame != NULL;
}

bool retro_load_game_special(unsigned p1, const struct retro_game_info *p2, size_t p3)
{
   (void)p1;
   (void)p2;
   (void)p3;

   return false;
}

void retro_unload_game(void)
{
   free(g_video_frame);
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

void *retro_get_memory_data(unsigned p)
{
   (void)p;

   return NULL;
}

size_t retro_get_memory_size(unsigned p)
{
   (void)p;

   return 0;
}
