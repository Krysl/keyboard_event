#include <windows.h>

#include "codeconvert.h"

#ifdef KEYEVENT_DEBUG
#include "spdlog/spdlog.h"
#endif

/**
 * ===
 * char / string
 * 多字节：用多个字节表示一个字符，对应的就是ANSI
 *    ANSI：他的产生源于ASCII字符集，最开始7个bits,总共代表了128个字符，包括英文、字母、数字、和一些特殊字符。后面由于各个国家为了彰显自己的本土语音，进行了扩充，产生了GB2312,BIG5,JIS等编码，形成了ANSI编码，现在可以看到一些把和ASCII和GBK混合在一起说就是这个原因。
 *    GBK：它是之前提到的由ANSI产生的GB2312的一个扩充，相当于增强版，收录了更过的汉字字符。
 *    UTF-8：
 * ---
 * wchar / wstring
 * 宽字节：通常用两个字节表示一个字符，对应的就是Unicode。
 *    Unicode：他的产生是为了统一不同国家的字符编码，不同的语言无法存储在同一段ANSI编码的文本中，因此中间桥梁Unicode出现了，Unicode把所有语言都统一到一套编码里，因此就不要出现乱码情况。因此他又有另一个名字：万国码。
 * ---
 *
 */

// C++ 编码转换GBK,UTF8,UNICODE
// https://blog.csdn.net/u012861467/article/details/106518668

// warning C4996: 'std::codecvt_utf8<wchar_t,1114111,(std::codecvt_mode)0>':
// warning STL4017: std::wbuffer_convert, std::wstring_c onvert, and the
// <codecvt> header (containing std::codecvt_mode, std::codecvt_utf8,
// std::codecvt_utf16, and std::codecvt_utf8_utf16) are deprecated in C++17.
// (The std::codecvt class template is NOT deprecated.) The C++ Standard doesn't
// provide equivalent non-depr ecated functionality; consider using
// MultiByteToWideChar() and WideCharToMultiByte() from <Windows.h> instead. You
// can define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING or
// _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received
// this warning.
#pragma warning(disable : 4996)

// string的编码方式为utf8，则采用：
std::string wstring2utf8string(const std::wstring &str) {
  typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
  static std::wstring_convert<F> strCnv(new F(".65001"));
  return strCnv.to_bytes(str);
}
std::wstring utf8string2wstring(const std::string &str) {
  typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
  static std::wstring_convert<F> strCnv(new F(".65001"));
  return strCnv.from_bytes(str);
}
// string的编码方式为除utf8外的其它编码方式，可采用：
std::string wstring2string(const std::wstring &str, const std::string &locale) {
  typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
  static std::wstring_convert<F> strCnv(new F(locale));
  return strCnv.to_bytes(str);
}
std::wstring string2wstring(const std::string &str, const std::string &locale) {
  typedef std::codecvt_byname<wchar_t, char, std::mbstate_t> F;
  static std::wstring_convert<F> strCnv(new F(locale));
  return strCnv.from_bytes(str);
}

std::string gbkstring2utf8string(const std::string gbkstr) {
  return wstring2utf8string(string2wstring(gbkstr, ".936"));
}

UINT getCodePage() {
  UINT _codePage = 0;
  const std::string str("一");
  const unsigned char *_str = (const unsigned char *)str.c_str();

  if ((str.size() == 3) &&  // UTF8=[228,184,128]
      ((_str[0] == 228) &&  //
       (_str[1] == 184) &&  //
       (_str[2] == 128))) {
    _codePage = 65001;
  } else if ((str.size() == 2) &&  // GBK=[D2,BB]=[210,187]
             ((_str[0] == 210) &&  //
              (_str[1] == 187))) {
    _codePage = 936;
  } else {
    _codePage = 0;
#ifdef KEYEVENT_DEBUG
    spdlog::info("str={}, size={}", _str, str.size());
    for (size_t i = 0; i < str.size(); i++) {
      spdlog::info("[{}] {}", i, _str[i]);
    }

    spdlog::error(
        "当前代码页既不是GBK，也不是UTF-8! 检查下环境变量 CL 是不是 /utf-8。");
#endif
  }
  return _codePage;
}
