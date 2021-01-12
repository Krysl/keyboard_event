#include "include/keyboard_event/keyboard_event_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <tchar.h>
#include <map>
#include <memory>
#include <sstream>

#include "debug.h"
#include "key.h"

const char kOnLogCallbackMethod[] = "onLog";

LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wp, LPARAM lp);

std::string MBFromW(LPCWSTR pwsz, UINT cp) {
  int cch = WideCharToMultiByte(cp, 0, pwsz, -1, 0, 0, NULL, NULL);

  char *psz = new char[cch];

  WideCharToMultiByte(cp, 0, pwsz, -1, psz, cch, NULL, NULL);

  std::string st(psz);
  delete[] psz;

  return st;
}
std::string LPCTSTR_To_string(LPCTSTR str) {
#ifdef UNICODE
  return MBFromW(str, CP_ACP);
#else
  return std::string(str);
#endif
}

TCHAR comboChars[4];

BOOL visibleShift = FALSE;
BOOL visibleModifier = TRUE;
BOOL onlyCommandKeys = FALSE;

BOOL keyAutoRepeat;
HHOOK kbdhook;
namespace {
class KeyboardEventPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);
  std::unique_ptr<flutter::MethodChannel<>> channel_;
  void showText(LPCTSTR text, int behavior);

  KeyboardEventPlugin(const KeyboardEventPlugin &) = delete;
  KeyboardEventPlugin &operator=(const KeyboardEventPlugin &) = delete;

  static KeyboardEventPlugin *get_instance(
      std::unique_ptr<flutter::MethodChannel<>> channel) {
    static auto instance = KeyboardEventPlugin(std::move(channel));
    return &instance;
  }

  virtual ~KeyboardEventPlugin();

 private:
  KeyboardEventPlugin(std::unique_ptr<flutter::MethodChannel<>> channel);
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

// static
void KeyboardEventPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "keyboard_event",
          &flutter::StandardMethodCodec::GetInstance());
  auto plugin = std::unique_ptr<KeyboardEventPlugin>(
      KeyboardEventPlugin::get_instance(std::move(channel)));
  auto *channel_pointer = plugin.get()->channel_.get();
  channel_pointer->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

KeyboardEventPlugin::KeyboardEventPlugin(
    std::unique_ptr<flutter::MethodChannel<>> channel) {
  HMODULE hInstance = GetModuleHandle(nullptr);
  kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInstance, NULL);
  channel_ = std::move(channel);
#ifdef _DEBUG
  TCHAR iniFile[MAX_PATH];
  GetModuleFileName(NULL, iniFile, MAX_PATH);
  iniFile[wcslen(iniFile) - 4] = '\0';
  wcscat_s(iniFile, MAX_PATH, L".ini");
  wcscpy_s(logFile, MAX_PATH, iniFile);
  logFile[wcslen(logFile) - 4] = '\0';
  wcscat_s(logFile, MAX_PATH, L".txt");
  errno_t err = _wfopen_s(&logStream, logFile, L"a");
  if (err) {
    MessageBox(NULL, TEXT("can't open log file"), TEXT("Warnning!"), MB_OK);
  }
#endif
}

KeyboardEventPlugin::~KeyboardEventPlugin() {}

void KeyboardEventPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    } else if (IsWindows8OrGreater()) {
      version_stream << "8";
    } else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
    // } else if (method_call.method_name().compare("onLog") == 0) {
    //   channel_->InvokeMethod(kOnLogCallbackMethod)；

  } else {
    result->NotImplemented();
  }
}

void KeyboardEventPlugin::showText(LPCTSTR text, int behavior = 0) {
// _tprintf(_T("%s\n"), text);
// MessageBox(NULL, text, TEXT("message"), MB_OK);
#ifdef _DEBUG
  tostringstream line;
  line << text << "\n";
  log(line);
  auto *channel_pointer = channel_.get();
  if (channel_pointer)
    channel_pointer->InvokeMethod(kOnLogCallbackMethod,
        std::make_unique<flutter::EncodableValue>(LPCTSTR_To_string(text)));
#endif
}
}  // namespace

void showText(LPCTSTR text, int behavior) {
  KeyboardEventPlugin::get_instance(NULL)->showText(text, behavior);
}
void KeyboardEventPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  KeyboardEventPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
LPCTSTR GetSymbolFromVK(UINT vk, UINT sc, BOOL mod, HKL hklLayout) {
  static TCHAR symbol[32];
  BYTE btKeyState[256];
  // WORD Symbol = 0;
  TCHAR cc[2];
  if (mod) {
    ZeroMemory(btKeyState, sizeof(btKeyState));
  } else {
    for (int i = 0; i < 256; i++) {
      btKeyState[i] = (BYTE)GetKeyState(i);
    }
  }
  int rr = ToUnicodeEx(vk, sc, btKeyState, cc, 2, 0, hklLayout);
#ifdef _DEBUG
  TCHAR ss[KL_NAMELENGTH];
  GetKeyboardLayoutName(ss);
  tostringstream line;
  line << vk << ":" << rr << ":" << sc << "\n";
  log(line);
#endif
  if (rr > 0) {
    if (!visibleShift && mod && GetKeyState(VK_SHIFT) < 0) {
      // prefix "Shift - " only when Ctrl or Alt is hold (mod as TRUE)
      swprintf(symbol, 32, L"Shift %c %s", comboChars[1], cc);
    } else {
      swprintf(symbol, 32, L"%s", cc);
      symbol[rr] = L'\0';
    }
    return symbol;
  }
  return NULL;
}

LPCTSTR getSpecialKey(UINT vk) {
  static TCHAR unknown[32];
  for (size_t i = 0; i < nSpecialKeys; ++i) {
    if (specialKeys[i].val == vk) {
      return specialKeys[i].label;
    }
  }
  swprintf(unknown, 32, L"0x%02x", vk);
  return unknown;
}
void addBracket(LPWSTR str) {
  TCHAR tmp[64];
  if (wcslen(comboChars) > 2) {
    swprintf(tmp, 64, L"%s", str);
    swprintf(str, 64, L"%c%s%c", comboChars[0], tmp, comboChars[2]);
  }
}

LPCTSTR getModSpecialKey(UINT vk, BOOL mod = FALSE) {
  static TCHAR modsk[64];
  if (vk == 0xA0 || vk == 0xA1) {
    if (!mod) {
      // show nothing if press SHIFT only
      return NULL;
    } else {
      wcscpy_s(modsk, 64, L"Shift");
    }
  } else {
    TCHAR tmp[64];
    LPCTSTR sk = getSpecialKey(vk);
    if (!visibleShift && GetKeyState(VK_SHIFT) < 0) {
      // prefix "Shift - "
      swprintf(tmp, 64, L"Shift %c %s", comboChars[1], sk);
      sk = tmp;
    }
    if (!mod && HIBYTE(sk[0]) == 0) {
      // if the special key is not used with modifierkey, and has not been
      // replaced with visible symbol then surround it with <>
      swprintf(modsk, 64, L"%s", sk);
      addBracket(modsk);
    } else {
      swprintf(modsk, 64, L"%s", sk);
    }
  }

  return modsk;
}

// remove a modifier vk from modifierkeys
// for example, remove "Alt" from "Ctrl - Alt"
void cleanModifier(UINT vk, LPWSTR modifierkeys) {
  TCHAR tmp[64];
  LPCTSTR ck = getSpecialKey(vk);
  if (modifierkeys == NULL) return;
  LPWSTR p = wcsstr(modifierkeys, ck);
  if (p == modifierkeys) {
    if (wcslen(modifierkeys) == wcslen(p)) {
      // current key is the only modifier
      modifierkeys[0] = '\0';
    } else {
      // remove current key and the " - " after it
      // sizeof(" - ") == 4
      wcscpy_s(tmp, 64, modifierkeys + wcslen(ck) + 4);
      wcscpy_s(modifierkeys, 64, tmp);
    }
  } else if (p) {
    // get rid of all after current key including the delimiter
    *(p - 3) = '\0';
  }
}
static TCHAR modifierkey[64] = L"\0";
static BOOL modifierUsed = FALSE;
LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wp, LPARAM lp) {
  KBDLLHOOKSTRUCT k = *(KBDLLHOOKSTRUCT *)lp;
  TCHAR c[64] = L"\0";
  TCHAR tmp[64] = L"\0";
  const TCHAR *theKey = NULL;

  if (nCode < 0) return CallNextHookEx(kbdhook, nCode, wp, lp);

  static DWORD lastvk = 0;
  UINT spk = visibleShift ? 0xA0 : 0xA2;
  GUITHREADINFO Gti;
  ::ZeroMemory(&Gti, sizeof(GUITHREADINFO));
  Gti.cbSize = sizeof(GUITHREADINFO);
  ::GetGUIThreadInfo(0, &Gti);
  DWORD dwThread = ::GetWindowThreadProcessId(Gti.hwndActive, 0);
  HKL hklLayout = ::GetKeyboardLayout(dwThread);
  UINT isDeadKey =
      ((MapVirtualKeyEx(k.vkCode, MAPVK_VK_TO_CHAR, hklLayout) & 0x80000000) >>
       31);
  if (isDeadKey) {
    // GetKeyboardState(btKeyState);
    // for(int i = 0; i < 256; i++) {
    // btKeyState[i] = (BYTE)GetKeyState(i);
    // }
    // deadKeyPressed = TRUE;
  } else if (wp == WM_KEYUP || wp == WM_SYSKEYUP) {
    lastvk = 0;
    // fadeLastLabel(TRUE);
    if (k.vkCode >= spk && k.vkCode <= 0xA5 || k.vkCode == 0x5B ||
        k.vkCode == 0x5C) {
      // cleanModifier(k.vkCode, modifierkey);
      modifierUsed = FALSE;
    }
  } else if (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN) {
    if (!keyAutoRepeat && lastvk == k.vkCode) {
      // fadeLastLabel(FALSE);
      return TRUE;
    }
    int fin = 0;
    if (k.vkCode >= spk && k.vkCode <= 0xA5 ||   // ctrl / alt
        k.vkCode == 0x5B || k.vkCode == 0x5C) {  // win
      LPCTSTR ck = getSpecialKey(k.vkCode);
      if (modifierkey[0] == '\0') {
        wcscpy_s(modifierkey, 64, ck);
      } else if (!wcsstr(modifierkey, ck)) {
        wcscpy_s(tmp, 64, modifierkey);
        swprintf(modifierkey, 64, L"%s %c %s", tmp, comboChars[1], ck);
      }
      if (!modifierUsed && visibleModifier) {
        swprintf(c, 64, L"%s", modifierkey);
        addBracket(c);
        if (lastvk == k.vkCode) {
          showText(c, 2);
        } else {
          showText(c, 1);
        }
      }
    } else {
      // WORD a = 0;
      BOOL mod = modifierkey[0] != '\0';
      if (k.vkCode == 0x08 || k.vkCode == 0x09 || k.vkCode == 0x0D ||
          k.vkCode == 0x1B || k.vkCode == 0x20) {
        // for <BS>/<Tab>/<ENTER>/<ESC>/<SPACE>, treat them as specialKeys
        theKey = getModSpecialKey(k.vkCode, mod);
        fin = 1;
      } else {
        theKey = GetSymbolFromVK(k.vkCode, k.scanCode, mod, hklLayout);
        if (!theKey) {
          // otherwise try to translate with ToAsciiEx
          // if fails to translate with ToAsciiEx, then treat it as specialKeys
          theKey = getModSpecialKey(k.vkCode, mod);
          fin = 1;
        }
      }

      if (theKey) {
        if (mod) {
          fin = 1;
          swprintf(tmp, 64, L"%s %c %s", modifierkey, comboChars[1], theKey);
          addBracket(tmp);
          theKey = tmp;
        }
        if (fin || !onlyCommandKeys) {
          showText(theKey, fin);
        }
      }
    }
    lastvk = k.vkCode;
  }

  return CallNextHookEx(kbdhook, nCode, wp, lp);
}