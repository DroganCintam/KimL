#ifndef __SIMPL_CONFIG_H__
#define __SIMPL_CONFIG_H__

#define LANGVER	"17.11"
#define APPVER	"1.0"

#if defined(WIN32) || defined(_WIN32)

#define IS_WIN32
#define EXESUFFIX	".exe"
#define EXENATIVE	"kimln.exe"

#else

#define EXESUFFIX	".out"
#define EXENATIVE	"kimln"

#endif

#endif//__SIMPL_CONFIG_H__
