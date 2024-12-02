#ifndef __PROGPATH_H__
#define __PROGPATH_H__

#if defined(WIN32) || defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define IS_WINDOWS

#else

#include <unistd.h>

#endif

char * GetProgramPath(void);

#endif//__PROGPATH_H__
