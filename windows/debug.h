#pragma once

#ifdef _DEBUG
#include <windows.h>

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <sstream>
#include <string>

#include "fmt/core.h"
#include "fmt/format.h"

extern WCHAR logFile[MAX_PATH];
extern FILE *logStream;

#ifndef UNICODE
typedef std::ostringstream tostringstream;
typedef std::string tstring;
#define ftprintf(file, fmt, ...) fprintf(file, fmt, __VA_ARGS__)
#else
typedef std::wostringstream tostringstream;
typedef std::wstring tstring;
#define ftprintf(file, fmt, ...) fwprintf(file, fmt, __VA_ARGS__)
#endif

void log_init(TCHAR *logPath);
void log(const tostringstream &line);
void logA(std::string line);
void logW(std::wstring line);

using namespace fmt::literals;

template <typename... Args>
void Mydebug(Args &&... args) {
  tostringstream stream;
  // (stream << ... << std::forward<Args>(args)) << '\n';

  ((stream << std::forward<Args>(args)), ...) << _T('\n');

  // OutputDebugString(stream.str().c_str());
  log(stream);
}
template <typename... Args>
void Mydebug2(const char format_str[], Args &&... args) {
  // std::string str = fmt::format(format_str, args...);
  try {
    const auto &vargs = fmt::make_args_checked<Args...>(format_str, args...);
    std::string str =
        fmt::detail::vformat(fmt::to_string_view(format_str), vargs);
    // printf(format_str, args...);
    logA(str);
  } catch (fmt::format_error e) {
    std::cerr << "Mydebug2 err:" << e.what();
  }
}

#define debug(format, ...) Mydebug2(format, __VA_ARGS__)

#else

#define debug(format, ...)
#define log_init(logPath)
#define log(line)

#endif
