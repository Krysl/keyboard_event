#include "debug.h"

#ifdef _DEBUG

WCHAR logFile[MAX_PATH];
FILE *logStream;

void log(const tostringstream &line)
{
    ftprintf(logStream, _T("%s"), line.str().c_str());
    OutputDebugString(line.str().c_str());
}

#endif