#ifndef __HHOP_VIDEO_H__
#define __HHOP_VIDEO_H__

#include <SDL.h>

// Uncomment this to check cross-platform compilation compatibility
// #undef WIN32

//#define USE_BBTABLET

#ifdef __NDS__
#define SCREEN_W 256
#define SCREEN_H 192
#else
#define SCREEN_W 640
#define SCREEN_H 480
#endif

// Hacky workaround for MSVC's broken for scoping
//#define for if (0) ; else for

extern SDL_Surface * screen;

#endif
