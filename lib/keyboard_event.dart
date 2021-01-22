import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

// ignore: import_of_legacy_library_into_null_safe
import 'package:logger/logger.dart';

const String _kOnLogCallbackMethod = "onLog";
typedef OnLogCallback = void Function(String str);
typedef void Listener(dynamic msg);
typedef void CancelListening();

var log = Logger();

class KeyboardEvent {
  static const MethodChannel _channel = const MethodChannel('keyboard_event');
  static const EventChannel _eventChannel =
      const EventChannel("keyboard_event/event");
  OnLogCallback onLog;

  KeyboardEvent({
    required this.onLog,
  }) {
    _channel.setMethodCallHandler(_callbackHandler);
  }
  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  /// Get Virturl-Key Map 
  /// 
  /// See details:
  /// "[Virtual-Key Codes](https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes)"
  /// 
  /// name -> vk
  /// ```
  /// {
  ///   'Virturl-Key Name': int Virturl-Key Code,
  /// }
  /// ```
  static Future<Map<String, int>> get getVirtualKeyString2CodeMap async {
    final Map<String, int> virtualKeyMap = Map<String, int>.from(
        await _channel.invokeMethod('getVirtualKeyMap', 0));
    return virtualKeyMap;
  }

  /// Get Virturl-Key Map   
  /// 
  /// See details:
  /// "[Virtual-Key Codes](https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes)"
  /// 
  /// vk -> [name...]
  /// ```
  /// {
  ///   int Virturl-Key Code: ['Virturl-Key Name 1','Virturl-Key Name 2',...],
  /// }
  /// ```
  static Future<Map<int, List<String>>> get getVirtualKeyCode2StringMap async {
    Map<int, List<String>> ret = {};
    final Map<int, List<dynamic>> virtualKeyMap = Map<int, List<dynamic>>.from(
        await _channel.invokeMethod('getVirtualKeyMap', 1));
    virtualKeyMap.forEach((key, value) {
      ret[key] = List<String>.from(value);
    });
    return ret;
  }

  Future<dynamic> _callbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kOnLogCallbackMethod) {
      final String str = methodCall.arguments;
      this.onLog(str);
      // log.i(str);
      return '_callbackHandler $str';
    }
  }

  int nextListenerId = 1;
  CancelListening? _cancelListening;
  startListening(Listener listener) async {
    var subscription = _eventChannel
        .receiveBroadcastStream(nextListenerId++)
        .listen(listener, cancelOnError: true);
    debugPrint("keyboard_event/event startListening");
    _cancelListening = () {
      subscription.cancel();
      debugPrint("keyboard_event/event canceled");
    };
  }

  cancelListening() async {
    if (_cancelListening != null) {
      _cancelListening!();
      _cancelListening = null;
    } else {
      debugPrint("keyboard_event/event No Need");
    }
  }
}
