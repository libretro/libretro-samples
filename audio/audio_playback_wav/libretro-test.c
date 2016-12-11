#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "libretro.h"

/* NOTE: This core does not work on big endian systems. */

#define BUFSIZE 128
#define AMP_MUL 64

retro_environment_t environ_cb            = NULL;
retro_video_refresh_t video_cb            = NULL;
retro_audio_sample_t audio_cb             = NULL;
retro_audio_sample_batch_t audio_batch_cb = NULL;
retro_input_poll_t poller_cb              = NULL;
retro_input_state_t input_state_cb        = NULL;

#ifndef EXTERNC
 #ifdef __cplusplus
  #define EXTERNC extern "C"
 #else
  #define EXTERNC
 #endif
#endif

#ifndef EXPORT
 #if defined(CPPCLI)
  #define EXPORT EXTERNC
 #elif defined(_WIN32)
  #define EXPORT EXTERNC __declspec(dllexport)
 #else
  #define EXPORT EXTERNC __attribute__((visibility("default")))
 #endif
#endif

struct WAVhead
{
   char ChunkID[4];
   uint32_t ChunkSize;
   char Format[4];

   char Subchunk1ID[4];
   uint32_t Subchunk1Size;
   uint16_t AudioFormat;
   uint16_t NumChannels;
   uint32_t SampleRate;
   uint32_t ByteRate;
   uint16_t BlockAlign;
   uint16_t BitsPerSample;

   char Subchunk2ID[4];
   uint32_t Subchunk2Size;
} head;

void* rawsamples                 = NULL;
unsigned int sample_pos          = 0;
unsigned int samples_tot         = 0;

unsigned int g_samples_to_play   = 0;
unsigned int sample_rate         = 0;
unsigned int bytes_per_sample    = 0;

enum
{
   ST_OFF = 0,
   ST_ON,
   ST_AUTO
} state;

static void emit_audio(void)
{
	unsigned int samples_to_play;
   unsigned int samples_played  = 0;

	if (state == ST_OFF)
      return;

	if (state == ST_ON)
      samples_to_play=BUFSIZE;
	if (state == ST_AUTO)
      samples_to_play=g_samples_to_play;

   /* no locking here, despite threading; 
    * if we touch this variable, threading is off. */

	while (samples_to_play >= BUFSIZE)
	{
      unsigned int played;
		int16_t samples[2*BUFSIZE];
		unsigned int samples_to_read = samples_to_play;
      
		if (samples_to_read > BUFSIZE)
         samples_to_read = BUFSIZE;
		if (sample_pos > samples_tot)
         sample_pos = samples_tot;
		if (sample_pos + samples_to_read > samples_tot)
         samples_to_read = samples_tot-sample_pos;
		
		if (samples_to_read != 0)
		{
         unsigned i;
			uint8_t* rawsamples8  = (uint8_t*)rawsamples + bytes_per_sample * sample_pos;
			int16_t* rawsamples16 = (int16_t*)rawsamples8;
			
			for (i = 0; i < samples_to_read; i++)
			{
				int16_t left  = 0;
				int16_t right = 0;

				if (head.NumChannels == 1 && head.BitsPerSample==8)
            {
               left  = rawsamples8[i] * AMP_MUL;
               right = rawsamples8[i] * AMP_MUL;
            }
				if (head.NumChannels == 2 && head.BitsPerSample==8)
            {
               left  = rawsamples8[i*2] * AMP_MUL;
               right = rawsamples8[i*2+1]*AMP_MUL;
            }
				if (head.NumChannels==1 && head.BitsPerSample==16)
            {
               left  = rawsamples16[i];
               right = rawsamples16[i];
            }
				if (head.NumChannels==2 && head.BitsPerSample==16)
            {
               left  = rawsamples16[i*2];
               right = rawsamples16[i*2+1];
            }
				
				samples[i*2+0] = left;
				samples[i*2+1] = right;
			}
		}

		if (samples_to_read!=BUFSIZE)
         memset(samples + samples_to_read * 2,
               0,
               sizeof(int16_t) * 2 * (BUFSIZE-samples_to_read));
		
		played               = audio_batch_cb(samples, BUFSIZE);
		sample_pos          += played;
		samples_played      += played;

		if (samples_to_play < played)
         break;

		samples_to_play -= played;

		if (played != BUFSIZE)
         break;
	}

	if (state == ST_AUTO)
      g_samples_to_play-=samples_played;
}

static void enable_audio(bool enabled)
{
   state = enabled;
}

EXPORT void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

EXPORT void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

EXPORT void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

EXPORT void retro_set_input_poll(retro_input_poll_t cb)
{
   poller_cb = cb;
}

EXPORT void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

EXPORT void retro_set_environment(retro_environment_t cb)
{
	struct retro_audio_callback aud = { emit_audio, enable_audio };
	environ_cb = cb;
	state      = ST_AUTO;

	if (environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &aud))
      state = ST_OFF;
}

EXPORT void retro_deinit(void) {}

EXPORT unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

EXPORT void retro_init(void)
{
   rawsamples = NULL;
}

EXPORT void retro_get_system_info(struct retro_system_info* info)
{
	const struct retro_system_info myinfo={ "WAV player", "v1", "wav", false, false };
	memcpy(info, &myinfo, sizeof(myinfo));
}

EXPORT void retro_get_system_av_info(struct retro_system_av_info* info)
{
	const struct retro_system_av_info myinfo={
		{ 320, 240, 320, 240, 0.0 },
		{ 60.0, head.SampleRate }
	};
	memcpy(info, &myinfo, sizeof(myinfo));
}

EXPORT void retro_reset(void)
{
   sample_pos=0; g_samples_to_play=0;
}

EXPORT void retro_run(void)
{
	static uint16_t pixels[240][320];
   unsigned int x;

	poller_cb();
	
	if (state == ST_AUTO)
	{
		g_samples_to_play += head.SampleRate / 60.0;
		emit_audio();
	}
	
	memset(pixels, 0xFF, sizeof(pixels));
	
	x = 320 * sample_pos / samples_tot;

	if (x<320)
	{
      unsigned y;
		for (y = 0; y < 240; y++)
			pixels[y][x] = 0x0000;
	}
	
	video_cb(pixels, 320, 240, sizeof(uint16_t) * 320);
}

EXPORT size_t retro_serialize_size(void)
{
   return sizeof(sample_pos);
}

EXPORT bool retro_serialize(void* data, size_t size)
{
   memcpy(data, &sample_pos, sizeof(sample_pos));
   return true;
}

EXPORT bool retro_unserialize(const void* data, size_t size)
{
   memcpy(&sample_pos, data, sizeof(sample_pos));
   return true;
}

EXPORT bool retro_load_game(const struct retro_game_info* game)
{
	enum retro_pixel_format rgb565 = RETRO_PIXEL_FORMAT_RGB565;

	retro_reset();
	
   if (rawsamples)
      free(rawsamples);
	
	if (game->size < 44)
      return false;

	memcpy(&head, game->data, 44);

	if (game->size != 44 + head.Subchunk2Size)
      return false;
	if (head.NumChannels   != 1 && head.NumChannels   != 2)
      return false;
	if (head.BitsPerSample != 8 && head.BitsPerSample != 16)
      return false;

	bytes_per_sample       = head.NumChannels   * head.BitsPerSample / 8;
	samples_tot            = head.Subchunk2Size / bytes_per_sample;
	
	rawsamples             = malloc(head.Subchunk2Size);

	memcpy(rawsamples, (uint8_t*)game->data + 44, game->size - 44);
	
	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565))
      return false;
	
	return true;
}

EXPORT bool retro_load_game_special(unsigned game_type,
      const struct retro_game_info* info, size_t num_info)
{
   return false;
}

EXPORT void retro_unload_game(void)
{
   free(rawsamples);
}

EXPORT unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

EXPORT void* retro_get_memory_data(unsigned id)  { return NULL; }
EXPORT size_t retro_get_memory_size(unsigned id) { return 0; }
EXPORT void retro_cheat_reset(void) {}
EXPORT void retro_cheat_set(unsigned index, bool enabled, const char* code) {}
EXPORT void retro_set_controller_port_device(unsigned port, unsigned device) {}
