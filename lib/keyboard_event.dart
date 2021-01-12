import 'dart:async';

import 'package:flutter/services.dart';

const String _kOnLogCallbackMethod = "onLog";
typedef OnLogCallback = Function(String str);

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

  Future<Null> _callbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kOnLogCallbackMethod) {
      if (this.onLog == null) return;
      final String str = methodCall.arguments;
      this.onLog(str);
    }
  }
}
