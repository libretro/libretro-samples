import bitops
import std/terminal
import example_libretro/libretro

const WIDTH = 320
const HEIGHT = 240 
const FPS = 60
const SAMPLE_RATE = 48000

var video_cb: retro_video_refresh_t
var audio_cb: retro_audio_sample_t
var audio_batch_cb: retro_audio_sample_batch_t
var input_poll_cb: retro_input_poll_t
var environ_cb: retro_environment_t
var input_state_cb: retro_input_state_t

proc NimMain() {.cdecl, importc.}

# video framebuffer
var buf:ptr UncheckedArray[cuint]

# colors used for checkerboard
const color_r:uint32 = 0xff shl 16
const color_g:uint32 = 0xff shl 8

proc log_cb(level: retro_log_level, message: string) =
  if level == RETRO_LOG_DEBUG:
    stdout.styledWriteLine(fgBlue, "DEBUG: ", fgDefault, message)
  elif level == RETRO_LOG_INFO:
    stdout.styledWriteLine(fgYellow, "INFO: ", fgDefault, message)
  elif level == RETRO_LOG_WARN:
    stdout.styledWriteLine(bgMagenta, "WARN: ", fgDefault, message)
  elif level == RETRO_LOG_ERROR:
    stdout.styledWriteLine(fgRed, "ERROR: ", fgDefault, message)

proc retro_set_video_refresh*(cb: retro_video_refresh_t) {.cdecl,exportc,dynlib.} =
  video_cb = cb

proc retro_set_audio_sample*(cb: retro_audio_sample_t) {.cdecl,exportc,dynlib.} =
  audio_cb = cb

proc retro_set_audio_sample_batch*(cb: retro_audio_sample_batch_t) {.cdecl,exportc,dynlib.} =
  audio_batch_cb = cb

proc retro_set_input_poll*(cb: retro_input_poll_t) {.cdecl,exportc,dynlib.} =
  input_poll_cb = cb

proc retro_set_environment*(cb: retro_environment_t) {.cdecl,exportc,dynlib.} =
  environ_cb = cb
  var no_content:bool = true
  discard cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, addr no_content)

proc retro_set_input_state*(cb: retro_input_state_t) {.cdecl,exportc,dynlib.} =
  input_state_cb = cb

proc retro_init*() {.cdecl,exportc,dynlib.} =
  NimMain() 
  var bufPtr0 = alloc0(sizeof(cuint) * WIDTH * HEIGHT)
  buf = cast[ptr UncheckedArray[cuint]](bufPtr0)
  log_cb(RETRO_LOG_DEBUG, "retro_init() called.")

proc retro_deinit*() {.cdecl,exportc,dynlib.} =
  log_cb(RETRO_LOG_DEBUG, "retro_deinit() called.")
  dealloc(buf)
  GC_FullCollect()

proc retro_api_version*(): cuint {.cdecl,exportc,dynlib.} =
  return RETRO_API_VERSION

proc retro_set_controller_port_device*(port: cuint; device: cuint) {.cdecl,exportc,dynlib.} =
  log_cb(RETRO_LOG_DEBUG, "retro_set_controller_port_device() called.")
  echo port, device

proc retro_get_system_info*(info: ptr retro_system_info) {.cdecl,exportc,dynlib.} =
  info.library_name = "nim_example";
  info.library_version = "v1"
  info.need_fullpath = false
  info.valid_extensions = nil # we don't use any ROMs

proc retro_get_system_av_info*(info: ptr retro_system_av_info) {.cdecl,exportc,dynlib.} =
  info.timing.fps = FPS
  info.timing.sample_rate = SAMPLE_RATE
  info.geometry.base_width = WIDTH
  info.geometry.base_height = HEIGHT
  info.geometry.max_width = WIDTH
  info.geometry.max_height = HEIGHT
  info.geometry.aspect_ratio = WIDTH / HEIGHT

proc retro_reset*() {.cdecl,exportc,dynlib.} =
  log_cb(RETRO_LOG_DEBUG, "retro_reset() called.")

proc retro_run*() {.cdecl,exportc,dynlib.} =
  for y in 0 ..< HEIGHT:
    let index_y = uint32 bitand((y shr 4), 1)
    for x in 0 ..< WIDTH:
      let b = ((y * WIDTH) + x)
      let index_x = uint32 bitand((x shr 4), 1)
      if bool bitxor(index_y, index_x):
        buf[b] = color_r
      else:
        buf[b] = color_g
  video_cb(buf, WIDTH, HEIGHT, (WIDTH shl 2))

proc retro_load_game*(info: ptr retro_game_info): bool {.cdecl,exportc,dynlib.} =
  var fmt = RETRO_PIXEL_FORMAT_XRGB8888
  if not environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, addr fmt):
    log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.")
    return false
  return true

proc retro_unload_game*() {.cdecl,exportc,dynlib.} =
  log_cb(RETRO_LOG_DEBUG, "retro_unload_game() called.")

proc retro_get_region*(): cuint {.cdecl,exportc,dynlib.} =
  return 0 # NTSC

proc retro_load_game_special*(`type`: cuint; info: ptr retro_game_info; num: csize_t): bool {.cdecl,exportc,dynlib.} =
  return true

proc retro_serialize_size*(): csize_t {.cdecl,exportc,dynlib.} =
  return 0

proc retro_serialize*(data: pointer; size: csize_t): bool {.cdecl,exportc,dynlib.} =
  return true

proc retro_unserialize*(data: pointer; size: csize_t): bool {.cdecl,exportc,dynlib.} =
  return true

proc retro_get_memory_data*(id: cuint): pointer {.cdecl,exportc,dynlib.} =
  discard

proc retro_get_memory_size*(id: cuint): csize_t {.cdecl,exportc,dynlib.} =
  return 0

proc retro_cheat_reset*() {.cdecl,exportc,dynlib.} =
  discard

proc retro_cheat_set*(index: cuint; enabled: bool; code: cstring) {.cdecl,exportc,dynlib.} =
  discard
