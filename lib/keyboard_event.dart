import 'dart:async';

import 'package:flutter/services.dart';
import 'package:logger/logger.dart';

const String _kOnLogCallbackMethod = "onLog";
typedef OnLogCallback = void Function(String str);

var log = Logger();

class KeyboardEvent {
  static const MethodChannel _channel = const MethodChannel('keyboard_event');
  OnLogCallback onLog;

  KeyboardEvent({
    this.onLog,
  }) {
    _channel.setMethodCallHandler(_callbackHandler);
  }
  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  Future<dynamic> _callbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kOnLogCallbackMethod) {
      if (this.onLog == null) return;
      final String str = methodCall.arguments;
      this.onLog(str);
      // log.i(str);
      return '_callbackHandler $str';
    }
  }
}
