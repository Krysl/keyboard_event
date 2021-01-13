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
#include "keyboard_event_plugin.h"

HHOOK kbdhook;
const char kOnLogCallbackMethod[] = "onLog";

LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wp, LPARAM lp);

namespace {

class KeyboardEventPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);
  static std::unique_ptr<flutter::MethodChannel<>> channel_;
  void showText(LPCTSTR text);

  // KeyboardEventPlugin(const KeyboardEventPlugin &) = delete;
  // KeyboardEventPlugin &operator=(const KeyboardEventPlugin &) = delete;

  /**
   * @brief Get the instance object
   * Meyers' Singleton.
   * https://www.cnblogs.com/sunchaothu/p/10389842.html
   * @param channel
   * @return KeyboardEventPlugin&
   */
  static KeyboardEventPlugin &get_instance() {
    static auto instance = KeyboardEventPlugin();
    return instance;
  }

  virtual ~KeyboardEventPlugin();

 private:
  KeyboardEventPlugin();
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
};

std::unique_ptr<flutter::MethodChannel<>> KeyboardEventPlugin::channel_ = NULL;

// static
void KeyboardEventPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  // auto channel =
  //     std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
  //         registrar->messenger(), "keyboard_event",
  //         &flutter::StandardMethodCodec::GetInstance());

  KeyboardEventPlugin::channel_ =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "keyboard_event",
          &flutter::StandardMethodCodec::GetInstance());
  KeyboardEventPlugin *_plugin = &KeyboardEventPlugin::get_instance();
  auto plugin = std::unique_ptr<KeyboardEventPlugin>(_plugin);
  auto *channel_pointer = KeyboardEventPlugin::channel_.get();
  channel_pointer->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

KeyboardEventPlugin::KeyboardEventPlugin() {
  // HMODULE hInstance = GetModuleHandle(nullptr);
  // kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInstance, NULL);
  // KeyboardEventPlugin::channel_ = std::move(channel);
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

KeyboardEventPlugin::~KeyboardEventPlugin() {
  // UnhookWindowsHookEx(kbdhook);
  KeyboardEventPlugin::channel_.reset();
#ifdef _DEBUG
  fclose(logStream);
#endif
}

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

/**
 * @brief 使用 Dart端 class KeyboardEvent 的 onLog(String str)
 * 回调函数来显示C++端信息
 * @param text 需要显示的字符串
 */
void KeyboardEventPlugin::showText(LPCTSTR text) {
// _tprintf(_T("%s\n"), text);
// MessageBox(NULL, text, TEXT("message"), MB_OK);
#ifdef _DEBUG
  tostringstream line;
  line << text << "\n";
  log(line);
#endif
  auto *channel_pointer = KeyboardEventPlugin::channel_.get();
  if (channel_pointer)
    channel_pointer->InvokeMethod(
        kOnLogCallbackMethod,
        std::make_unique<flutter::EncodableValue>(LPCTSTR_To_string(text)));
}
}  // namespace

/**
 * @brief 使用 Dart端 class KeyboardEvent 的 onLog(String str)
 * 回调函数来显示C++端信息
 *
 * @param text 需要显示的字符串
 */
void showText(LPCTSTR text, int behavior) {
  KeyboardEventPlugin::get_instance().showText(text);
}

void KeyboardEventPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  KeyboardEventPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
