import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
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
    this.onLog,
  }) {
    _channel.setMethodCallHandler(_callbackHandler);
  }
  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static Future<Map<String, int>> get getVirtualKeyMap async {
    final Map<String, int> virtualKeyMap =
        Map<String, int>.from(await _channel.invokeMethod('getVirtualKeyMap'));
    return virtualKeyMap;
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

  int nextListenerId = 1;
  CancelListening _cancelListening;
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
      _cancelListening();
      _cancelListening = null;
    } else {
      debugPrint("keyboard_event/event No Need");
    }
  }
}
