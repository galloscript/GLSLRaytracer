
#include "Common.h"
#include "Utils.h"

#include <stdio.h>
#include <Windows.h>

void WriteLog(const char* format, ...)
{
    static char mesg[49152];
    va_list  arglist;

    va_start(arglist, format);
    vsprintf_s(mesg, sizeof(mesg), format, arglist);
    strcat_s(mesg, sizeof(mesg), "\n");
    va_end(arglist);
    OutputDebugString(mesg);
}