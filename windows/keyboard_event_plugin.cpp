#include "include/keyboard_event/keyboard_event_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/method_result_functions.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>

#include <tchar.h>
#include <initializer_list>
#include <map>
#include <memory>
#include <sstream>
#include "spdlog/spdlog.h"

#include <fmt/format.h>
#include "codeconvert.h"
#include "keyboard_event.h"
#include "keyboard_event_plugin.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "timestamp.h"

using namespace spdlog;
using namespace fmt::literals;

HHOOK kbdhook;

void log_init() {
#ifdef _DEBUG
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::debug);
  console_sink->set_pattern("[keyboard_event] [%^%l%$] %v");

  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
  file_sink->set_level(spdlog::level::trace);

  set_default_logger(std::make_shared<spdlog::logger>(
      "multi_sink",
      std::initializer_list<spdlog::sink_ptr>{console_sink, file_sink}));
  set_level(spdlog::level::debug);
  warn("this should appear in both console and file");
  info("this message should not appear in the console, only in the file");
#endif
}

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
#ifdef _DEBUG
  UINT _codePage;
#endif
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
#ifdef _DEBUG
  _codePage = getCodePage();
#endif
  HMODULE hInstance = GetModuleHandle(nullptr);
  kbdhook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardProc, hInstance, NULL);
  log_init();
}

KeyboardEventPlugin::~KeyboardEventPlugin() { UnhookWindowsHookEx(kbdhook); }

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
    version_stream << "  [" << TIMESTAMP << "]";
    result->Success(flutter::EncodableValue(version_stream.str()));
    debug(version_stream.str());
  } else {
    debug("%s Not Implemented\n", method_call.method_name().c_str());
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
  if (channel == NULL) return;
  auto *channel_pointer = channel.get();
  if (channel_pointer) {
    auto result =
        std::make_unique<flutter::EngineMethodResult<flutter::EncodableValue>>(
            [](const uint8_t *reply, size_t reply_size) {
#pragma warning(disable : 4189)
              // const char *buf = (const char *)reply;
              auto buf = std::string((char *)reply, reply_size);
              // auto result = flutter::MethodResult<flutter::EncodableValue>();
              static auto result =
                  flutter::MethodResultFunctions<flutter::EncodableValue>(
                      [](const flutter::EncodableValue *result) {
                        if (std::holds_alternative<std::string>(*result)) {
                          std::string some_string =
                              std::get<std::string>(*result);
                          debug("onLog: ret = {}", some_string);
                        }
                      },
                      [](const std::string &error_code,
                         const std::string &error_message,
                         const flutter::EncodableValue *error_details) {
                        if (std::holds_alternative<std::string>(
                                *error_details)) {
                          std::string err_string =
                              std::get<std::string>(*error_details);
                          error(
                              "onLog ERROR! errCode={}, msg={}, details={}",  //
                              error_code, error_message, err_string);
                        } else {
                          error("onLog ERROR! errCode={}, msg={}",  //
                                error_code, error_message);
                        }
                      },
                      []() { error("onLog ERROR! NotImplemented!"); });
              flutter::StandardMethodCodec::GetInstance()
                  .DecodeAndProcessResponseEnvelope(reply, reply_size, &result);
            },
            &flutter::StandardMethodCodec::GetInstance());
    channel_pointer->InvokeMethod(
        kOnLogCallbackMethod,
        std::make_unique<flutter::EncodableValue>(LPCTSTR_To_string(text)),
        std::move(result));
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
