import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'package:logger/logger.dart';

const String _kOnLogCallbackMethod = "onLog";
typedef OnLogCallback = void Function(String str);

enum KeyEventMsg { WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, WM_SYSKEYUP, WM_UNKNOW }

KeyEventMsg toKeyEventMsg(int v) {
  KeyEventMsg keyMsg;
  switch (v) {
    case 0:
      keyMsg = KeyEventMsg.WM_KEYDOWN;
      break;
    case 1:
      keyMsg = KeyEventMsg.WM_KEYUP;
      break;
    case 2:
      keyMsg = KeyEventMsg.WM_SYSKEYDOWN;
      break;
    case 3:
      keyMsg = KeyEventMsg.WM_SYSKEYDOWN;
      break;
    default:
      keyMsg = KeyEventMsg.WM_UNKNOW;
      break;
  }
  return keyMsg;
}

class KeyEvent {
  late KeyEventMsg keyMsg;
  late int vkCode;
  late int scanCode;
  late int flags;
  late int time;
  late int dwExtraInfo;

  KeyEvent(List<int> list) {
    keyMsg = toKeyEventMsg(list[0]);
    vkCode = list[1];
    scanCode = list[2];
    flags = list[3];
    time = list[4];
    dwExtraInfo = list[5];
  }

  bool get isKeyUP =>
      (keyMsg == KeyEventMsg.WM_KEYUP) || (keyMsg == KeyEventMsg.WM_SYSKEYUP);
  bool get isKeyDown => !isKeyUP;
  bool get isSysKey =>
      (keyMsg == KeyEventMsg.WM_SYSKEYUP) ||
      (keyMsg == KeyEventMsg.WM_SYSKEYDOWN);

  String? get vkName =>
      KeyboardEvent._virtualKeyCode2StringMap?[this.vkCode]?[0];

  @override
  String toString() {
    var sb = StringBuffer();
    var map = KeyboardEvent._virtualKeyCode2StringMap;
    String? name;
    if (map != null) {
      name = map[vkCode]?[0];
    }
    if (name == null) name = '<Unknow Key $vkCode>';
    sb.write('KeyEvent ${isKeyUP ? "↑" : "↓"}$name');
    return sb.toString();
  }
}

typedef void Listener(KeyEvent keyEvent);
typedef void CancelListening();

var log = Logger();

class KeyBoardState {
  Set<int> state = Set<int>();
  KeyBoardState();
  @override
  String toString() {
    if (KeyboardEvent._virtualKeyCode2StringMap != null) {
      var sb = StringBuffer();
      bool isFirst = true;
      sb.write('[');
      for (var key in state) {
        if (isFirst) {
          isFirst = false;
        } else {
          sb.write(',');
        }
        var str = KeyboardEvent._virtualKeyCode2StringMap![key]?[0];
        sb.write(str != null ? str : key.toString());
      }
      sb.write(']');
      return sb.toString();
    } else {
      return state.toString();
    }
  }
}

class KeyboardEvent {
  static const MethodChannel _channel = const MethodChannel('keyboard_event');
  static const EventChannel _eventChannel =
      const EventChannel("keyboard_event/event");
  KeyBoardState state = KeyBoardState();
  OnLogCallback? onLog;

  KeyboardEvent({
    this.onLog,
  }) {
    _channel.setMethodCallHandler(_callbackHandler);
    virtualKeyString2CodeMap;
    virtualKeyCode2StringMap;
  }
  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static Map<String, int>? _virtualKeyString2CodeMap;

  static Future<Map<String, int>> get _getVirtualKeyString2CodeMap async {
    final Map<String, int> virtualKeyMap = Map<String, int>.from(
        await _channel.invokeMethod('getVirtualKeyMap', 0));
    return virtualKeyMap;
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
  static Future<Map<String, int>> get virtualKeyString2CodeMap async {
    if (_virtualKeyString2CodeMap == null) {
      _virtualKeyString2CodeMap = await _getVirtualKeyString2CodeMap;
    }
    return _virtualKeyString2CodeMap!;
  }

  static Map<int, List<String>>? _virtualKeyCode2StringMap;

  static Future<Map<int, List<String>>> get _getVirtualKeyCode2StringMap async {
    Map<int, List<String>> ret = {};
    final Map<int, List<dynamic>> virtualKeyMap = Map<int, List<dynamic>>.from(
        await _channel.invokeMethod('getVirtualKeyMap', 1));
    virtualKeyMap.forEach((key, value) {
      ret[key] = List<String>.from(value);
    });
    return ret;
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
  static Future<Map<int, List<String>>> get virtualKeyCode2StringMap async {
    if (_virtualKeyCode2StringMap == null) {
      _virtualKeyCode2StringMap = await _getVirtualKeyCode2StringMap;
    }
    return _virtualKeyCode2StringMap!;
  }

  Future<dynamic> _callbackHandler(MethodCall methodCall) async {
    if (methodCall.method == _kOnLogCallbackMethod) {
      if (onLog == null) return;
      final String str = methodCall.arguments;
      onLog!(str);
      // log.i(str);
      return '_callbackHandler $str';
    }
  }

  int nextListenerId = 1;
  CancelListening? _cancelListening;
  startListening(Listener listener) async {
    var subscription =
        _eventChannel.receiveBroadcastStream(nextListenerId++).listen(//listener
            (dynamic msg) {
      var list = List<int>.from(msg);
      var keyEvent = KeyEvent(list);
      if (keyEvent.isKeyDown) {
        if (!state.state.contains(keyEvent.vkCode)) {
          state.state.add(keyEvent.vkCode);
        }
      } else {
        if (state.state.contains(keyEvent.vkCode)) {
          state.state.remove(keyEvent.vkCode);
        }
      }
      listener(keyEvent);
    }, cancelOnError: true);
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
