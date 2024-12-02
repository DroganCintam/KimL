#include "progpath.h"

#define MAX_PROG_PATH 1024

#ifdef IS_WINDOWS

char * GetProgramPath()
{
	LPSTR buffer = new CHAR[MAX_PROG_PATH];
	
	DWORD result = GetModuleFileNameA(NULL, buffer, MAX_PROG_PATH);
	if (!result && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		buffer[MAX_PROG_PATH - 1] = '\0';

	return (char *)buffer;
}

#else

char * GetProgramPath()
{
	char *buffer = new char[MAX_PROG_PATH];
	ssize_t len = readlink("/proc/self/exe", buffer, MAX_PROG_PATH - 1);
	if (len != -1)
		buffer[len] = '\0';
	else
		buffer[0] = '\0';

	return buffer;
}

#endif
