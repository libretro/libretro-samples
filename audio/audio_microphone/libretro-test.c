#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "libretro.h"

/*
 * audio_microphone
 *
 * Demonstrates the libretro microphone interface by recording audio from the
 * system microphone and playing it back through the frontend's audio output.
 *
 * Controls:
 *   A      - Record (stops playback)
 *   B      - Play   (stops recording)
 *   Start  - Clear the recording
 */

/* Display dimensions */
#define VIDEO_WIDTH  320
#define VIDEO_HEIGHT 240

/* Audio configuration */
#define SAMPLE_RATE  44100

/* Recording buffer: ~10 seconds of mono audio at 44100 Hz */
#define RECORD_BUFFER_SAMPLES (SAMPLE_RATE * 10)

/* Number of microphone samples to read per frame */
#define MIC_SAMPLES_PER_FRAME (SAMPLE_RATE / 60)

/* Magnitude of a full-scale 16-bit sample */
#define SAMPLE_MAX 32768

/* Samples inspected per waveform column; caps the cost of drawing a frame */
#define WAVE_SAMPLES_PER_COLUMN 16

/* How long on-screen messages stay up, in frames */
#define MESSAGE_FRAMES 120

/* ---- Colors (XRGB8888) ---- */
#define COLOR_BACKGROUND    0xFF1A1A2E
#define COLOR_CENTER_LINE   0xFF333355
#define COLOR_WAVE_RECORD   0xFF00FF88
#define COLOR_WAVE_PLAYBACK 0xFF00DDFF
#define COLOR_WAVE_IDLE     0xFFCCCCCC
#define COLOR_CURSOR        0xFFFF4444
#define COLOR_RECORD_DOT    0xFFFF0000

/* ---- Recording indicator layout ---- */
#define INDICATOR_SIZE 10
#define INDICATOR_Y    4
#define RECORD_DOT_X   4

/* The core is always in exactly one of these modes */
enum core_mode
{
   MODE_IDLE = 0,
   MODE_RECORDING,
   MODE_PLAYING
};

/* ---- State ---- */
static uint32_t *frame_buf;
static retro_log_printf_t log_cb;

static retro_video_refresh_t video_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

/* Microphone interface */
static struct retro_microphone_interface mic_interface;
static retro_microphone_t *microphone;

/* Recording buffer (mono, 16-bit signed) */
static int16_t *record_buffer;
static unsigned record_write_pos;  /* next write position in circular buffer */
static unsigned record_length;     /* total samples recorded (capped at buffer size) */

static unsigned playback_pos;
static enum core_mode mode;

/* Debounce state, indexed by RETRO_DEVICE_ID_JOYPAD_* */
static bool prev_buttons[RETRO_DEVICE_ID_JOYPAD_R3 + 1];

static const struct retro_input_descriptor input_descriptors[] = {
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "Record" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "Play" },
   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Clear" },
   { 0, 0, 0, 0, NULL }
};

/* ---- Logging ---- */
static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   va_list va;

   (void)level;

   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

/* Logs a line and asks the frontend to show it on screen. */
static void notify(const char *msg)
{
   struct retro_message message;

   log_cb(RETRO_LOG_INFO, "[audio_microphone] %s\n", msg);

   message.msg = msg;
   message.frames = MESSAGE_FRAMES;
   environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &message);
}

/* ---- Recording buffer helpers ---- */

/*
 * Reads the nth-oldest recorded sample.
 */
static int16_t record_sample(unsigned n)
{
   unsigned oldest = (record_write_pos + RECORD_BUFFER_SAMPLES - record_length)
      % RECORD_BUFFER_SAMPLES;

   return record_buffer[(oldest + n) % RECORD_BUFFER_SAMPLES];
}

/*
 * Switches modes, keeping the microphone enabled only while recording. Every
 * mode change goes through here, which is what makes recording and playback
 * mutually exclusive: entering one always leaves the other.
 */
static void set_mode(enum core_mode next)
{
   if (next == mode)
      return;

   /* Reject impossible transitions before touching any state */
   if (next == MODE_RECORDING && !microphone)
   {
      notify("No microphone available");
      return;
   }

   if (next == MODE_PLAYING && record_length == 0)
   {
      notify("Nothing recorded yet");
      return;
   }

   /* Leaving MODE_RECORDING always disables the microphone */
   if (mode == MODE_RECORDING)
      mic_interface.set_mic_state(microphone, false);

   if (next == MODE_RECORDING && !mic_interface.set_mic_state(microphone, true))
   {
      notify("Failed to enable the microphone");
      mode = MODE_IDLE;
      return;
   }

   if (next == MODE_PLAYING)
      playback_pos = 0;

   mode = next;
   notify(next == MODE_RECORDING ? "Recording" :
          next == MODE_PLAYING   ? "Playing"   :
                                   "Stopped");
}

/* Stops everything and discards what was recorded. */
static void clear_recording(void)
{
   set_mode(MODE_IDLE);

   record_write_pos = 0;
   record_length    = 0;
   playback_pos     = 0;
}

/* ---- Rendering ---- */

/* Maps a sample (-32768..32767) to a screen row, loudest at the edges. */
static int sample_to_y(int sample)
{
   int y = VIDEO_HEIGHT / 2 - (sample * (VIDEO_HEIGHT / 2)) / SAMPLE_MAX;

   if (y < 0)
      return 0;
   if (y >= VIDEO_HEIGHT)
      return VIDEO_HEIGHT - 1;
   return y;
}

static void render_frame(void)
{
   uint32_t *buf = frame_buf;
   unsigned x, y, i;

   /* Background: dark grey */
   for (i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++)
      buf[i] = COLOR_BACKGROUND;

   /* Draw center line (dim) */
   for (x = 0; x < VIDEO_WIDTH; x++)
      buf[(VIDEO_HEIGHT / 2) * VIDEO_WIDTH + x] = COLOR_CENTER_LINE;

   /*
    * Draw the whole recording as a peak envelope: one vertical bar per column,
    * spanning the loudest positive and negative samples in that slice. Only a
    * fixed number of samples per column is inspected, so the cost of a frame
    * does not grow with the length of the recording.
    */
   if (record_length > 0)
   {
      uint32_t color = (mode == MODE_RECORDING) ? COLOR_WAVE_RECORD :
                       (mode == MODE_PLAYING)   ? COLOR_WAVE_PLAYBACK :
                                                  COLOR_WAVE_IDLE;

      for (x = 0; x < VIDEO_WIDTH; x++)
      {
         unsigned first = x * record_length / VIDEO_WIDTH;
         unsigned last  = (x + 1) * record_length / VIDEO_WIDTH;
         unsigned step, n;
         int min = 0, max = 0;
         int row;

         if (last <= first)
            last = first + 1;
         step = (last - first) / WAVE_SAMPLES_PER_COLUMN + 1;

         for (n = first; n < last; n += step)
         {
            int sample = record_sample(n);

            if (sample < min)
               min = sample;
            if (sample > max)
               max = sample;
         }

         for (row = sample_to_y(max); row <= sample_to_y(min); row++)
            buf[row * VIDEO_WIDTH + x] = color;
      }
   }

   /* Playback cursor; playing guarantees record_length > playback_pos */
   if (mode == MODE_PLAYING)
   {
      unsigned cursor_x = playback_pos * VIDEO_WIDTH / record_length;

      for (y = 0; y < VIDEO_HEIGHT; y++)
         buf[y * VIDEO_WIDTH + cursor_x] = COLOR_CURSOR;
   }

   /*
    * Recording indicator: a red dot in the top-left corner. Playback needs no
    * counterpart, since the moving cursor already marks it.
    */
   if (mode == MODE_RECORDING)
   {
      for (y = 0; y < INDICATOR_SIZE; y++)
      {
         for (x = 0; x < INDICATOR_SIZE; x++)
         {
            int dx = (int)x - INDICATOR_SIZE / 2;
            int dy = (int)y - INDICATOR_SIZE / 2;

            if (dx * dx + dy * dy <= (INDICATOR_SIZE / 2) * (INDICATOR_SIZE / 2))
               buf[(INDICATOR_Y + y) * VIDEO_WIDTH + RECORD_DOT_X + x] = COLOR_RECORD_DOT;
         }
      }
   }

   video_cb(buf, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH * sizeof(uint32_t));
}

/* ---- Core implementation ---- */

void retro_init(void)
{
   frame_buf     = (uint32_t *)calloc(VIDEO_WIDTH * VIDEO_HEIGHT, sizeof(uint32_t));
   record_buffer = (int16_t *)calloc(RECORD_BUFFER_SAMPLES, sizeof(int16_t));

   mode = MODE_IDLE;
   memset(prev_buttons, 0, sizeof(prev_buttons));
   clear_recording();
}

void retro_deinit(void)
{
   if (microphone)
   {
      mic_interface.close_mic(microphone);
      microphone = NULL;
   }

   free(frame_buf);
   frame_buf = NULL;

   free(record_buffer);
   record_buffer = NULL;
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "Microphone Test";
   info->library_version  = "v1";
   info->need_fullpath    = false;
   info->valid_extensions = NULL;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->timing = (struct retro_system_timing) {
      .fps = 60.0,
      .sample_rate = (double)SAMPLE_RATE,
   };

   info->geometry = (struct retro_game_geometry) {
      .base_width   = VIDEO_WIDTH,
      .base_height  = VIDEO_HEIGHT,
      .max_width    = VIDEO_WIDTH,
      .max_height   = VIDEO_HEIGHT,
      .aspect_ratio = 4.0f / 3.0f,
   };
}

void retro_set_environment(retro_environment_t cb)
{
   struct retro_log_callback logging;
   bool no_content = true;

   environ_cb = cb;
   cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

   if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = fallback_log;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   (void)cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_reset(void)
{
   clear_recording();
}

/* Returns true only on the frame a button goes from released to pressed. */
static bool pressed(unsigned id)
{
   bool now = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, id);
   bool edge = now && !prev_buttons[id];

   prev_buttons[id] = now;
   return edge;
}

static void handle_input(void)
{
   input_poll_cb();

   /* A records and B plays; each stops the other. Pressing again idles. */
   if (pressed(RETRO_DEVICE_ID_JOYPAD_A))
      set_mode(mode == MODE_RECORDING ? MODE_IDLE : MODE_RECORDING);

   if (pressed(RETRO_DEVICE_ID_JOYPAD_B))
      set_mode(mode == MODE_PLAYING ? MODE_IDLE : MODE_PLAYING);

   if (pressed(RETRO_DEVICE_ID_JOYPAD_START))
   {
      clear_recording();
      notify("Recording cleared");
   }
}

static void process_microphone(void)
{
   int16_t mic_buf[MIC_SAMPLES_PER_FRAME];
   int samples_read;
   int i;

   if (mode != MODE_RECORDING)
      return;

   samples_read = mic_interface.read_mic(microphone, mic_buf, MIC_SAMPLES_PER_FRAME);

   /* Write samples into the circular buffer, dropping the oldest once full */
   for (i = 0; i < samples_read; i++)
   {
      record_buffer[record_write_pos] = mic_buf[i];
      record_write_pos = (record_write_pos + 1) % RECORD_BUFFER_SAMPLES;

      if (record_length < RECORD_BUFFER_SAMPLES)
         record_length++;
   }
}

static void process_playback(void)
{
   int16_t stereo_buf[MIC_SAMPLES_PER_FRAME * 2];
   unsigned i;

   for (i = 0; i < MIC_SAMPLES_PER_FRAME; i++)
   {
      /* Silence unless playing back */
      int16_t sample = 0;

      if (mode == MODE_PLAYING)
      {
         sample = record_sample(playback_pos);

         /* Loop back to the beginning at the end of the recording */
         if (++playback_pos >= record_length)
            playback_pos = 0;
      }

      /* Mono to stereo */
      stereo_buf[i * 2 + 0] = sample;
      stereo_buf[i * 2 + 1] = sample;
   }

   audio_batch_cb(stereo_buf, MIC_SAMPLES_PER_FRAME);
}

void retro_run(void)
{
   handle_input();
   process_microphone();
   process_playback();
   render_frame();
}

bool retro_load_game(const struct retro_game_info *info)
{
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
   retro_microphone_params_t params;

   (void)info;

   if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   {
      log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
      return false;
   }

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, (void *)input_descriptors);

   /*
    * Request the microphone interface.
    */
   memset(&mic_interface, 0, sizeof(mic_interface));
   mic_interface.interface_version = RETRO_MICROPHONE_INTERFACE_VERSION;

   if (!environ_cb(RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE, &mic_interface) ||
       !mic_interface.open_mic  || !mic_interface.close_mic ||
       !mic_interface.read_mic  || !mic_interface.set_mic_state)
   {
      log_cb(RETRO_LOG_WARN,
         "[audio_microphone] Frontend provides no usable microphone interface.\n");
      microphone = NULL;
      return true;
   }

   log_cb(RETRO_LOG_INFO,
      "[audio_microphone] Microphone interface v%u obtained.\n",
      mic_interface.interface_version);

   /* Open the microphone; it stays inactive until the user starts recording */
   params.rate = SAMPLE_RATE;
   microphone  = mic_interface.open_mic(&params);

   if (!microphone)
      log_cb(RETRO_LOG_ERROR, "[audio_microphone] Failed to open the microphone.\n");

   return true;
}

void retro_unload_game(void)
{
   set_mode(MODE_IDLE);

   if (microphone)
   {
      mic_interface.close_mic(microphone);
      microphone = NULL;
   }
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   (void)type;
   (void)info;
   (void)num;
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *data_, size_t size)
{
   (void)data_;
   (void)size;
   return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
   (void)data_;
   (void)size;
   return false;
}

void *retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}
