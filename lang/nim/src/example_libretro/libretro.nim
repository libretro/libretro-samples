const
  RETRO_API_VERSION* = 1
  RETRO_DEVICE_TYPE_SHIFT* = 8
  RETRO_DEVICE_MASK* = ((1 shl RETRO_DEVICE_TYPE_SHIFT) - 1)
  RETRO_MEMORY_ACCESS_WRITE* = (1 shl 0)
  RETRO_MEMORY_ACCESS_READ* = (1 shl 1)
  RETRO_MEMORY_TYPE_CACHED* = (1 shl 0)
  RETRO_DEVICE_NONE* = 0
  RETRO_DEVICE_JOYPAD* = 1
  RETRO_DEVICE_MOUSE* = 2
  RETRO_DEVICE_KEYBOARD* = 3
  RETRO_DEVICE_LIGHTGUN* = 4
  RETRO_DEVICE_ANALOG* = 5
  RETRO_DEVICE_POINTER* = 6
  RETRO_DEVICE_ID_JOYPAD_B* = 0
  RETRO_DEVICE_ID_JOYPAD_Y* = 1
  RETRO_DEVICE_ID_JOYPAD_SELECT* = 2
  RETRO_DEVICE_ID_JOYPAD_START* = 3
  RETRO_DEVICE_ID_JOYPAD_UP* = 4
  RETRO_DEVICE_ID_JOYPAD_DOWN* = 5
  RETRO_DEVICE_ID_JOYPAD_LEFT* = 6
  RETRO_DEVICE_ID_JOYPAD_RIGHT* = 7
  RETRO_DEVICE_ID_JOYPAD_A* = 8
  RETRO_DEVICE_ID_JOYPAD_X* = 9
  RETRO_DEVICE_ID_JOYPAD_L* = 10
  RETRO_DEVICE_ID_JOYPAD_R* = 11
  RETRO_DEVICE_ID_JOYPAD_L2* = 12
  RETRO_DEVICE_ID_JOYPAD_R2* = 13
  RETRO_DEVICE_ID_JOYPAD_L3* = 14
  RETRO_DEVICE_ID_JOYPAD_R3* = 15
  RETRO_DEVICE_INDEX_ANALOG_LEFT* = 0
  RETRO_DEVICE_INDEX_ANALOG_RIGHT* = 1
  RETRO_DEVICE_ID_ANALOG_X* = 0
  RETRO_DEVICE_ID_ANALOG_Y* = 1
  RETRO_DEVICE_ID_MOUSE_X* = 0
  RETRO_DEVICE_ID_MOUSE_Y* = 1
  RETRO_DEVICE_ID_MOUSE_LEFT* = 2
  RETRO_DEVICE_ID_MOUSE_RIGHT* = 3
  RETRO_DEVICE_ID_MOUSE_WHEELUP* = 4
  RETRO_DEVICE_ID_MOUSE_WHEELDOWN* = 5
  RETRO_DEVICE_ID_MOUSE_MIDDLE* = 6
  RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP* = 7
  RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN* = 8
  RETRO_DEVICE_ID_LIGHTGUN_X* = 0
  RETRO_DEVICE_ID_LIGHTGUN_Y* = 1
  RETRO_DEVICE_ID_LIGHTGUN_TRIGGER* = 2
  RETRO_DEVICE_ID_LIGHTGUN_CURSOR* = 3
  RETRO_DEVICE_ID_LIGHTGUN_TURBO* = 4
  RETRO_DEVICE_ID_LIGHTGUN_PAUSE* = 5
  RETRO_DEVICE_ID_LIGHTGUN_START* = 6
  RETRO_DEVICE_ID_POINTER_X* = 0
  RETRO_DEVICE_ID_POINTER_Y* = 1
  RETRO_DEVICE_ID_POINTER_PRESSED* = 2
  RETRO_REGION_NTSC* = 0
  RETRO_REGION_PAL* = 1
  RETRO_MEMORY_MASK* = 0xff
  RETRO_MEMORY_SAVE_RAM* = 0
  RETRO_MEMORY_RTC* = 1
  RETRO_MEMORY_SYSTEM_RAM* = 2
  RETRO_MEMORY_VIDEO_RAM* = 3
  RETRO_ENVIRONMENT_EXPERIMENTAL* = 0x10000
  RETRO_ENVIRONMENT_PRIVATE* = 0x20000
  RETRO_ENVIRONMENT_SET_ROTATION* = 1
  RETRO_ENVIRONMENT_GET_OVERSCAN* = 2
  RETRO_ENVIRONMENT_GET_CAN_DUPE* = 3
  RETRO_ENVIRONMENT_SET_MESSAGE* = 6
  RETRO_ENVIRONMENT_SHUTDOWN* = 7
  RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL* = 8
  RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY* = 9
  RETRO_ENVIRONMENT_SET_PIXEL_FORMAT* = 10
  RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS* = 11
  RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK* = 12
  RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE* = 13
  RETRO_ENVIRONMENT_SET_HW_RENDER* = 14
  RETRO_ENVIRONMENT_GET_VARIABLE* = 15
  RETRO_ENVIRONMENT_SET_VARIABLES* = 16
  RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE* = 17
  RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME* = 18
  RETRO_ENVIRONMENT_GET_LIBRETRO_PATH* = 19
  RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK* = 22
  RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK* = 21
  RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE* = 23
  RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES* = 24
  RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE* = 25
  RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE* = 26
  RETRO_ENVIRONMENT_GET_LOG_INTERFACE* = 27
  RETRO_ENVIRONMENT_GET_PERF_INTERFACE* = 28
  RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE* = 29
  RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY* = 30
  RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY* = 30
  RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY* = 31
  RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO* = 32
  RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK* = 33
  RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO* = 34
  RETRO_ENVIRONMENT_SET_CONTROLLER_INFO* = 35
  RETRO_ENVIRONMENT_SET_MEMORY_MAPS* = (36 or RETRO_ENVIRONMENT_EXPERIMENTAL)
  RETRO_ENVIRONMENT_SET_GEOMETRY* = 37
  RETRO_ENVIRONMENT_GET_USERNAME* = 38
  RETRO_ENVIRONMENT_GET_LANGUAGE* = 39
  RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER* = 40
  RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE* = 41
  RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS* = 42
  RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE* = 43
  RETRO_MEMDESC_CONST* = (1 shl 0)
  RETRO_MEMDESC_BIGENDIAN* = (1 shl 1)
  RETRO_MEMDESC_ALIGN_2* = (1 shl 16)
  RETRO_MEMDESC_ALIGN_4* = (2 shl 16)
  RETRO_MEMDESC_ALIGN_8* = (3 shl 16)
  RETRO_MEMDESC_MINSIZE_2* = (1 shl 24)
  RETRO_MEMDESC_MINSIZE_4* = (2 shl 24)
  RETRO_MEMDESC_MINSIZE_8* = (3 shl 24)
  RETRO_SENSOR_ACCELEROMETER_X* = 0
  RETRO_SENSOR_ACCELEROMETER_Y* = 1
  RETRO_SENSOR_ACCELEROMETER_Z* = 2
  # RETRO_HW_FRAME_BUFFER_VALID* = (cast[pointer](-1))

type
  retro_language* = enum
    RETRO_LANGUAGE_ENGLISH = 0, RETRO_LANGUAGE_JAPANESE = 1,
    RETRO_LANGUAGE_FRENCH = 2, RETRO_LANGUAGE_SPANISH = 3, RETRO_LANGUAGE_GERMAN = 4,
    RETRO_LANGUAGE_ITALIAN = 5, RETRO_LANGUAGE_DUTCH = 6,
    RETRO_LANGUAGE_PORTUGUESE = 7, RETRO_LANGUAGE_RUSSIAN = 8,
    RETRO_LANGUAGE_KOREAN = 9, RETRO_LANGUAGE_CHINESE_TRADITIONAL = 10,
    RETRO_LANGUAGE_CHINESE_SIMPLIFIED = 11, RETRO_LANGUAGE_ESPERANTO = 12,
    RETRO_LANGUAGE_POLISH = 13, RETRO_LANGUAGE_LAST,
    RETRO_LANGUAGE_DUMMY = 2147483647
  
  retro_key* = enum
    RETROK_UNKNOWN = 0, RETROK_BACKSPACE = 8, RETROK_TAB = 9, RETROK_CLEAR = 12,
    RETROK_RETURN = 13, RETROK_PAUSE = 19, RETROK_ESCAPE = 27, RETROK_SPACE = 32,
    RETROK_EXCLAIM = 33, RETROK_QUOTEDBL = 34, RETROK_HASH = 35, RETROK_DOLLAR = 36,
    RETROK_AMPERSAND = 38, RETROK_QUOTE = 39, RETROK_LEFTPAREN = 40,
    RETROK_RIGHTPAREN = 41, RETROK_ASTERISK = 42, RETROK_PLUS = 43, RETROK_COMMA = 44,
    RETROK_MINUS = 45, RETROK_PERIOD = 46, RETROK_SLASH = 47, RETROK_0 = 48, RETROK_1 = 49,
    RETROK_2 = 50, RETROK_3 = 51, RETROK_4 = 52, RETROK_5 = 53, RETROK_6 = 54, RETROK_7 = 55,
    RETROK_8 = 56, RETROK_9 = 57, RETROK_COLON = 58, RETROK_SEMICOLON = 59,
    RETROK_LESS = 60, RETROK_EQUALS = 61, RETROK_GREATER = 62, RETROK_QUESTION = 63,
    RETROK_AT = 64, RETROK_LEFTBRACKET = 91, RETROK_BACKSLASH = 92,
    RETROK_RIGHTBRACKET = 93, RETROK_CARET = 94, RETROK_UNDERSCORE = 95, RETROK_BACKQUOTE = 96, 
    RETROK_LOWER_A = 97, RETROK_LOWER_B = 98, RETROK_LOWER_C = 99, RETROK_LOWER_D = 100,
    RETROK_LOWER_E = 101, RETROK_LOWER_F = 102, RETROK_LOWER_G = 103, RETROK_LOWER_H = 104, RETROK_LOWER_I = 105,
    RETROK_LOWER_J = 106, RETROK_LOWER_K = 107, RETROK_LOWER_L = 108, RETROK_LOWER_M = 109, RETROK_LOWER_N = 110,
    RETROK_LOWER_O = 111, RETROK_LOWER_P = 112, RETROK_LOWER_Q = 113, RETROK_LOWER_R = 114, RETROK_LOWER_S = 115,
    RETROK_LOWER_T = 116, RETROK_LOWER_U = 117, RETROK_LOWER_V = 118, RETROK_LOWER_W = 119, RETROK_LOWER_X = 120,
    RETROK_LOWER_Y = 121, RETROK_LOWER_Z = 122,
    RETROK_DELETE = 127, RETROK_KP0 = 256, RETROK_KP1 = 257,
    RETROK_KP2 = 258, RETROK_KP3 = 259, RETROK_KP4 = 260, RETROK_KP5 = 261,
    RETROK_KP6 = 262, RETROK_KP7 = 263, RETROK_KP8 = 264, RETROK_KP9 = 265,
    RETROK_KP_PERIOD = 266, RETROK_KP_DIVIDE = 267, RETROK_KP_MULTIPLY = 268,
    RETROK_KP_MINUS = 269, RETROK_KP_PLUS = 270, RETROK_KP_ENTER = 271,
    RETROK_KP_EQUALS = 272, RETROK_UP = 273, RETROK_DOWN = 274, RETROK_RIGHT = 275,
    RETROK_LEFT = 276, RETROK_INSERT = 277, RETROK_HOME = 278, RETROK_END = 279,
    RETROK_PAGEUP = 280, RETROK_PAGEDOWN = 281, RETROK_F1 = 282, RETROK_F2 = 283,
    RETROK_F3 = 284, RETROK_F4 = 285, RETROK_F5 = 286, RETROK_F6 = 287, RETROK_F7 = 288,
    RETROK_F8 = 289, RETROK_F9 = 290, RETROK_F10 = 291, RETROK_F11 = 292, RETROK_F12 = 293,
    RETROK_F13 = 294, RETROK_F14 = 295, RETROK_F15 = 296, RETROK_NUMLOCK = 300,
    RETROK_CAPSLOCK = 301, RETROK_SCROLLOCK = 302, RETROK_RSHIFT = 303,
    RETROK_LSHIFT = 304, RETROK_RCTRL = 305, RETROK_LCTRL = 306, RETROK_RALT = 307,
    RETROK_LALT = 308, RETROK_RMETA = 309, RETROK_LMETA = 310, RETROK_LSUPER = 311,
    RETROK_RSUPER = 312, RETROK_MODE = 313, RETROK_COMPOSE = 314, RETROK_HELP = 315,
    RETROK_PRINT = 316, RETROK_SYSREQ = 317, RETROK_BREAK = 318, RETROK_MENU = 319,
    RETROK_POWER = 320, RETROK_EURO = 321, RETROK_UNDO = 322, RETROK_LAST, RETROK_DUMMY = 2147483647

  retro_mod* = enum
    RETROKMOD_NONE = 0x0000, RETROKMOD_SHIFT = 0x01, RETROKMOD_CTRL = 0x02,
    RETROKMOD_ALT = 0x04, RETROKMOD_META = 0x08, RETROKMOD_NUMLOCK = 0x10,
    RETROKMOD_CAPSLOCK = 0x20, RETROKMOD_SCROLLOCK = 0x40, RETROKMOD_DUMMY = 2147483647

  retro_hw_render_interface_type* = enum
    RETRO_HW_RENDER_INTERFACE_VULKAN = 0,
    RETRO_HW_RENDER_INTERFACE_DUMMY = 2147483647

  retro_hw_render_interface* {.bycopy.} = object
    interface_type*: retro_hw_render_interface_type
    interface_version*: cuint

  retro_hw_render_context_negotiation_interface_type* = enum
    RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN = 0,
    RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_DUMMY = 2147483647

  retro_hw_render_context_negotiation_interface* {.bycopy.} = object
    interface_type*: retro_hw_render_context_negotiation_interface_type
    interface_version*: cuint

  retro_memory_descriptor* {.bycopy.} = object
    flags*: cuint
    `ptr`*: pointer
    offset*: csize_t
    start*: csize_t
    select*: csize_t
    disconnect*: csize_t
    len*: csize_t
    addrspace*: cstring

  retro_memory_map* {.bycopy.} = object
    descriptors*: ptr retro_memory_descriptor
    num_descriptors*: cuint

  retro_controller_description* {.bycopy.} = object
    desc*: cstring
    id*: cuint

  retro_controller_info* {.bycopy.} = object
    types*: ptr retro_controller_description
    num_types*: cuint

  retro_subsystem_memory_info* {.bycopy.} = object
    extension*: cstring
    `type`*: cuint

  retro_subsystem_rom_info* {.bycopy.} = object
    desc*: cstring
    valid_extensions*: cstring
    need_fullpath*: bool
    block_extract*: bool
    required*: bool
    memory*: ptr retro_subsystem_memory_info
    num_memory*: cuint

  retro_subsystem_info* {.bycopy.} = object
    desc*: cstring
    ident*: cstring
    roms*: ptr retro_subsystem_rom_info
    num_roms*: cuint
    id*: cuint

  retro_log_level* = enum
    RETRO_LOG_DEBUG = 0, RETRO_LOG_INFO, RETRO_LOG_WARN, RETRO_LOG_ERROR,
    RETRO_LOG_DUMMY = 2147483647

  retro_sensor_action* = enum
    RETRO_SENSOR_ACCELEROMETER_ENABLE = 0,
    RETRO_SENSOR_ACCELEROMETER_DISABLE,
    RETRO_SENSOR_DUMMY = 2147483647

  retro_camera_buffer* = enum
    RETRO_CAMERA_BUFFER_OPENGL_TEXTURE = 0,
    RETRO_CAMERA_BUFFER_RAW_FRAMEBUFFER,
    RETRO_CAMERA_BUFFER_DUMMY = 2147483647

  retro_rumble_effect* = enum
    RETRO_RUMBLE_STRONG = 0,
    RETRO_RUMBLE_WEAK = 1,
    RETRO_RUMBLE_DUMMY = 2147483647

  retro_hw_context_type* = enum
    RETRO_HW_CONTEXT_NONE = 0,
    RETRO_HW_CONTEXT_OPENGL = 1,
    RETRO_HW_CONTEXT_OPENGLES2 = 2,
    RETRO_HW_CONTEXT_OPENGL_CORE = 3,
    RETRO_HW_CONTEXT_OPENGLES3 = 4,
    RETRO_HW_CONTEXT_OPENGLES_VERSION = 5,
    RETRO_HW_CONTEXT_VULKAN = 6,
    RETRO_HW_CONTEXT_DUMMY = 2147483647

  retro_pixel_format* = enum
    RETRO_PIXEL_FORMAT_0RGB1555 = 0,
    RETRO_PIXEL_FORMAT_XRGB8888 = 1,
    RETRO_PIXEL_FORMAT_RGB565 = 2,
    RETRO_PIXEL_FORMAT_UNKNOWN = 2147483647

  retro_message* {.bycopy.} = object
    msg*: cstring
    frames*: cuint

  retro_input_descriptor* {.bycopy.} = object
    port*: cuint
    device*: cuint
    index*: cuint
    id*: cuint
    description*: cstring

  retro_system_info* {.bycopy.} = object
    library_name*: cstring
    library_version*: cstring
    valid_extensions*: cstring
    need_fullpath*: bool
    block_extract*: bool

  retro_game_geometry* {.bycopy.} = object
    base_width*: cuint
    base_height*: cuint
    max_width*: cuint
    max_height*: cuint
    aspect_ratio*: cfloat

  retro_system_timing* {.bycopy.} = object
    fps*: cdouble
    sample_rate*: cdouble

  retro_system_av_info* {.bycopy.} = object
    geometry*: retro_game_geometry
    timing*: retro_system_timing

  retro_variable* {.bycopy.} = object
    key*: cstring
    value*: cstring

  retro_game_info* {.bycopy.} = object
    path*: cstring
    data*: pointer
    size*: csize_t
    meta*: cstring

  retro_framebuffer* {.bycopy.} = object
    data*: pointer
    width*: cuint
    height*: cuint
    pitch*: csize_t
    format*: retro_pixel_format
    access_flags*: cuint
    memory_flags*: cuint

  # typedef bool(RETRO_CALLCONV* retro_environment_t)(unsigned cmd, void* data);
  retro_environment_t* = proc(cmd:cuint, data:pointer):bool {.cdecl.}

  # typedef void(RETRO_CALLCONV* retro_video_refresh_t)(const void* data, unsigned width, unsigned height, size_t pitch);
  retro_video_refresh_t* = proc(data:seq[cuint], width:cuint, height:cuint, pitch:csize_t):void {.cdecl.}

  # typedef void(RETRO_CALLCONV* retro_audio_sample_t)(int16_t left, int16_t right);
  retro_audio_sample_t* = proc(left:int16, right:int16):void {.cdecl.}

  # typedef size_t(RETRO_CALLCONV* retro_audio_sample_batch_t)(const int16_t* data, size_t frames);
  retro_audio_sample_batch_t* = proc(data:seq[int16], frames:csize_t):csize_t {.cdecl.} # 48000/60

  # typedef void(RETRO_CALLCONV* retro_input_poll_t)(void);
  retro_input_poll_t* = proc():void {.cdecl.}

  # typedef int16_t(RETRO_CALLCONV* retro_input_state_t)(unsigned port, unsigned device, unsigned index, unsigned id);
  retro_input_state_t* = proc(port:cuint, device:cuint, index:cuint, id:cuint):int16 {.cdecl.}

  # XXX: not sure if I need these interface/callback-object types
  # retro_get_proc_address_interface* {.bycopy.} = object
  #   get_proc_address*: retro_get_proc_address_t
  # retro_sensor_interface* {.bycopy.} = object
  #   set_sensor_state*: retro_set_sensor_state_t
  #   get_sensor_input*: retro_sensor_get_input_t
  # retro_camera_callback* {.bycopy.} = object
  #   caps*: cuint
  #   width*: cuint
  #   height*: cuint
  #   start*: retro_camera_start_t
  #   stop*: retro_camera_stop_t
  #   frame_raw_framebuffer*: retro_camera_frame_raw_framebuffer_t
  #   frame_opengl_texture*: retro_camera_frame_opengl_texture_t
  #   initialized*: retro_camera_lifetime_status_t
  #   deinitialized*: retro_camera_lifetime_status_t
  # retro_location_callback* {.bycopy.} = object
  #   start*: retro_location_start_t
  #   stop*: retro_location_stop_t
  #   get_position*: retro_location_get_position_t
  #   set_interval*: retro_location_set_interval_t
  #   initialized*: retro_location_lifetime_status_t
  #   deinitialized*: retro_location_lifetime_status_t
  # retro_rumble_interface* {.bycopy.} = object
  #   set_rumble_state*: retro_set_rumble_state_t
  # retro_audio_callback* {.bycopy.} = object
  #   callback*: retro_audio_callback_t
  #   set_state*: retro_audio_set_state_callback_t
  # retro_frame_time_callback* {.bycopy.} = object
  #   callback*: retro_frame_time_callback_t
  #   reference*: retro_usec_t
  # retro_hw_render_callback* {.bycopy.} = object
  #   context_type*: retro_hw_context_type
  #   context_reset*: retro_hw_context_reset_t
  #   get_current_framebuffer*: retro_hw_get_current_framebuffer_t
  #   get_proc_address*: retro_hw_get_proc_address_t
  #   depth*: bool
  #   stencil*: bool
  #   bottom_left_origin*: bool
  #   version_major*: cuint
  #   version_minor*: cuint
  #   cache_context*: bool
  #   context_destroy*: retro_hw_context_reset_t
  #   debug_context*: bool
  # retro_keyboard_callback* {.bycopy.} = object
  #   callback*: retro_keyboard_event_t
  # retro_disk_control_callback* {.bycopy.} = object
  #   set_eject_state*: retro_set_eject_state_t
  #   get_eject_state*: retro_get_eject_state_t
  #   get_image_index*: retro_get_image_index_t
  #   set_image_index*: retro_set_image_index_t
  #   get_num_images*: retro_get_num_images_t
  #   replace_image_index*: retro_replace_image_index_t
  #   add_image_index*: retro_add_image_index_t
  # retro_log_callback* {.bycopy.} = object
  #   log*: retro_log_printf_t
