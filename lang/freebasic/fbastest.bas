' 
' BUILDING
'    Run "fbc -dll fbastest.bas -x freebasic_libretro.so"
'
' RUNNING
'    ./retroarch -L freebasic_libretro.so
'
#lang "fb"

type RetroEnvironmentFunc as function cdecl(cmd as ulong, data_ as any ptr) as long
type RetroVideoRefreshProc as sub cdecl(data_ as any ptr, width_ as ulong, height_ as ulong, pitch_ as uinteger)
type RetroAudioSampleProc as sub cdecl(left_ as long, right_ as long)
type RetroAudioSampleBatchFunc as function cdecl(data_ as long ptr, frames_ as uinteger) as long
type RetroInputPollProc as sub cdecl
type RetroInputStateFunc as function cdecl(port_ as ulong, device_ as ulong, index_ as ulong, id_ as ulong) as long

type RetroSystemInfo
	LibraryName as zstring ptr
	LibraryVersion as zstring ptr
	ValidExtensions as zstring ptr
	NeedFullpath as long
	BlockExtract as long
end type

type RetroGameGeometry
	BaseWidth as ulong
	BaseHeight as ulong
	MaxWidth as ulong
	MaxHeight as ulong
	AspectRatio as single
end type

type RetroSystemTiming
	FPS as double
	SampleRate as double
end type

type RetroSystemAvInfo
	Geometry as RetroGameGeometry
	Timing as RetroSystemTiming
end type

type RetroGameInfo
	Path as zstring ptr
	Data as any ptr
	Size as uinteger
	Meta as zstring ptr
end type

const SCREEN_WIDTH  = 360
const SCREEN_HEIGHT = 180
dim shared env_cb as RetroEnvironmentFunc
dim shared poll_cb as RetroInputPollProc
dim shared video_cb as RetroVideoRefreshProc
dim shared framebuffer(0 to SCREEN_WIDTH * SCREEN_HEIGHT) as ushort

sub retro_set_environment alias "retro_set_environment" (p as RetroEnvironmentFunc)
	dim no_game as long = 1
	env_cb = p
	env_cb(18, @no_game)
end sub

sub retro_set_video_refresh cdecl alias "retro_set_video_refresh" (p as RetroVideoRefreshProc)
  video_cb = p
end sub

sub retro_set_audio_sample cdecl alias "retro_set_audio_sample" (p as RetroAudioSampleProc)

end sub

sub retro_set_audio_sample_batch cdecl alias "retro_set_audio_sample_batch" (p as RetroAudioSampleBatchFunc)

end sub

sub retro_set_input_poll cdecl alias "retro_set_input_poll" (p as RetroInputPollProc)
  poll_cb = p
end sub

sub retro_set_input_state cdecl alias "retro_set_input_state" (p as RetroInputStateFunc)
end sub

sub retro_init cdecl alias "retro_init"
end sub

sub retro_deinit cdecl alias "retro_deinit"
end sub

function retro_api_version cdecl alias "retro_api_version" as ulong
  return 1
end function

sub retro_get_system_info cdecl alias "retro_get_system_info" (info as RetroSystemInfo)
  info.LibraryName    = @"basic test"
  info.LibraryVersion = @"1.0"
end sub

sub retro_get_system_av_info cdecl alias "retro_get_system_av_info" (info as RetroSystemAvInfo)
  with info.Geometry
    .BaseWidth   = SCREEN_WIDTH
    .BaseHeight  = SCREEN_HEIGHT
    .MaxWidth    = .BaseWidth
    .MaxHeight   = .BaseHeight
    .AspectRatio = .BaseWidth / .BaseHeight
  end with

  with info.Timing
    .FPS = 60
    .SampleRate = 44000
  end with
end sub

sub retro_set_controller_port_device cdecl alias "retro_set_controller_port_device" (port as ulong, device as ulong)
end sub

sub retro_reset cdecl alias "retro_reset"

end sub

sub retro_run cdecl alias "retro_run"
  poll_cb()
  video_cb(@framebuffer(0), SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH * 2)
end sub

function retro_serialize_size cdecl alias "retro_serialize_size" as uinteger
  return 0
end function

function retro_serialize cdecl alias "retro_serialize" (d as any ptr, size as uinteger) as long
  return 0
end function

function retro_unserialize cdecl alias "retro_unserialize" (d as any ptr, size as uinteger) as long
  return 0
end function

sub retro_cheat_reset cdecl alias "retro_cheat_reset"

end sub

sub retro_cheat_set cdecl alias "retro_cheat_set" (index as ulong, Enabled as long, code as zstring ptr)

end sub

function retro_load_game cdecl alias "retro_load_game" (game as RetroGameInfo) as long
	dim pixel_format as long = 0 ' 0rgb1555
	
	env_cb(10, @pixel_format)
	for y as integer = 0 to SCREEN_HEIGHT
		for x as integer = 0 to SCREEN_WIDTH
			framebuffer(y * SCREEN_WIDTH + x) = fix((y + x) mod 2) * &he318
		next x
	next y

  return 1
end function

function retro_load_game_special cdecl alias "retro_load_game_special" (gameType as ulong, info as RetroGameInfo, numInfo as uinteger) as long
  return retro_load_game(info)
end function

sub retro_unload_game cdecl alias "retro_unload_game"

end sub

function retro_get_region cdecl alias "retro_get_region" as ulong
  return 1
end function

function retro_get_memory_data cdecl alias "retro_get_memory_data" (id as ulong) as any ptr
  return 0
end function

function retro_get_memory_size cdecl alias "retro_get_memory_size" (id as ulong) as uinteger
  return 0
end function
