#ifdef _DEBUG

#include "debug.h"

#include <pathcch.h>
#pragma comment(lib, "Pathcch.lib")

WCHAR logFile[MAX_PATH];
FILE *logStream;

void log_init(TCHAR *logPath) {
  if (logPath == NULL) {
    TCHAR moduleFile[MAX_PATH];
    DWORD len = GetModuleFileName(NULL, moduleFile, MAX_PATH);
    if (len == 0) return;
#ifdef UNICODE
    HRESULT ret = PathCchRenameExtension(moduleFile, MAX_PATH, _T("txt"));
    if (ret != S_OK) return;
#else
    _tcscat_s(moduleFile, MAX_PATH, _T("txt"));
#endif
    _tcscpy_s(logFile, MAX_PATH, moduleFile);
  } else {
    _tcscpy_s(logFile, MAX_PATH, logPath);
  }

  logStream = _tfsopen(logFile, _T("w"), _SH_DENYNO);
  if (logStream == NULL) {
    MessageBox(NULL, TEXT("can't open log file"), TEXT("Warnning!"), MB_OK);
  }
}

void log(const tostringstream &line) {
  if (logStream) {
    ftprintf(logStream, line.str().c_str());
    fflush(logStream);
  }
  // OutputDebugString(line.str().c_str());
  _tprintf(line.str().c_str());
}
void logA(std::string line) {
  if (logStream) {
    fprintf(logStream, line.c_str());
    fflush(logStream);
  }
  // OutputDebugString(line.str().c_str());
  printf(line.c_str());
}
void logW(std::wstring line) {
  if (logStream) {
    fwprintf(logStream, line.c_str());
    fflush(logStream);
  }
  // OutputDebugString(line.str().c_str());
  wprintf(line.c_str());
}

#endif