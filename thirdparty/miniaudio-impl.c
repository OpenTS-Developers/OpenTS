/*
 * Single translation unit for the vendored miniaudio library. The MA_NO_* and
 * MA_ENABLE_* options come from the CMake target so every consumer of
 * miniaudio.h sees the same configuration as this implementation.
 */

#define MINIAUDIO_IMPLEMENTATION

/* stb_vorbis provides the Ogg Vorbis decoder; miniaudio expects it before its own header. */
#include "extras/stb_vorbis.c"
#undef L
#undef C
#undef R

#include "miniaudio.h"
