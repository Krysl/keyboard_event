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
#include "keyboard_event.h"
#include "keyboard_event_plugin.h"

HHOOK kbdhook;

namespace {
std::unique_ptr<flutter::MethodChannel<>> channel = NULL;

class KeyboardEventPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);
  static void showText(LPCTSTR text);

  KeyboardEventPlugin();
  virtual ~KeyboardEventPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  static const char kOnLogCallbackMethod[];
};

const char KeyboardEventPlugin::kOnLogCallbackMethod[] = "onLog";

// static
void KeyboardEventPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  channel = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar->messenger(), "keyboard_event",
      &flutter::StandardMethodCodec::GetInstance());
  auto plugin = std::make_unique<KeyboardEventPlugin>();
  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

KeyboardEventPlugin::KeyboardEventPlugin() {
  HMODULE hInstance = GetModuleHandle(nullptr);
  kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInstance, NULL);
#ifdef _DEBUG
  log_init(NULL);
#endif
}

KeyboardEventPlugin::~KeyboardEventPlugin() {
  UnhookWindowsHookEx(kbdhook);
#ifdef _DEBUG
  if (logStream) fclose(logStream);
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
    // debug("%s Not Implemented\n", method_call.method_name().c_str());
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
  // #ifdef _DEBUG
  //   tostringstream line;
  //   line << text << "\n";
  //   log(line);
  // #endif
  if (channel == NULL) return;
  auto *channel_pointer = channel.get();
  if (channel_pointer) {
    auto result =
        std::make_unique<flutter::EngineMethodResult<flutter::EncodableValue>>(
            [](const uint8_t *reply, size_t reply_size) {
              debug("get result={reply}, size={size}", "reply"_a=reply,
                    "size"_a=reply_size, "\n");
            },
            &flutter::StandardMethodCodec::GetInstance());
    // auto resultPtr = result.get();
    channel_pointer->InvokeMethod(
        kOnLogCallbackMethod,
        std::make_unique<flutter::EncodableValue>(LPCTSTR_To_string(text)),
        std::move(result));
    // resultPtr->get().
  }
}
}  // namespace

/**
 * @brief 使用 Dart端 class KeyboardEvent 的 onLog(String str)
 * 回调函数来显示C++端信息
 *
 * @param text 需要显示的字符串
 */
void showText(LPCTSTR text, int behavior) {
  KeyboardEventPlugin::showText(text);
}

void KeyboardEventPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  KeyboardEventPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
