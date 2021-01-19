#if !defined(__CODECONVERT_H__)
#define __CODECONVERT_H__

#include <codecvt>
#include <locale>
#include <string>

// string的编码方式为utf8，则采用：
std::string wstring2utf8string(const std::wstring &str);
std::wstring utf8string2wstring(const std::string &str);

// string的编码方式为除utf8外的其它编码方式，可采用：
std::string wstring2string(const std::wstring &str, const std::string &locale);
std::wstring string2wstring(const std::string &str, const std::string &locale);

// gbk => utf8
std::string gbkstring2utf8string(const std::string gbkstr);

UINT getCodePage();
#endif  // __CODECONVERT_H__
