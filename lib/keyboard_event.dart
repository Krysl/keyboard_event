import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'package:logger/logger.dart';

import 'flags.dart';

const String _kOnLogCallbackMethod = 'onLog';
typedef OnLogCallback = void Function(String str);

// ignore: constant_identifier_names
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

String toBinary(int num, [int len = 8]) {
  StringBuffer buf = StringBuffer();
  if (len < 0) throw ArgumentError('len() should >=0');
  while (--len >= 0) {
    buf.write((num & (0x1 << len)) > 0 ? '1' : '0');
  }
  return buf.toString();
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

  bool get isKeyUP => (keyMsg == KeyEventMsg.WM_KEYUP) || (keyMsg == KeyEventMsg.WM_SYSKEYUP) || isReleased;
  bool get isKeyDown => !isKeyUP;
  bool get isSysKey => (keyMsg == KeyEventMsg.WM_SYSKEYUP) || (keyMsg == KeyEventMsg.WM_SYSKEYDOWN);

  bool get isExtended => 0 != (flags & LLKHF_EXTENDED);
  bool get isLowerIlInjected => 0 != (flags & LLKHF_LOWER_IL_INJECTED);
  bool get isInjected => 0 != (flags & LLKHF_INJECTED);
  bool get isAltDown => 0 != (flags & LLKHF_ALTDOWN);
  bool get isReleased => 0 != (flags & LLKHF_UP);

  String? get vkName => KeyboardEvent.virtualKeyCode2StringMap?[vkCode]?[0];

  bool? get isLetter => KeyboardEvent.isLetter(vkCode);
  bool? get isNumber => KeyboardEvent.isNumber(vkCode);

  @override
  String toString() {
    var sb = StringBuffer();
    var map = KeyboardEvent.virtualKeyCode2StringMap;
    String? name;
    if (map != null) {
      name = map[vkCode]?[0];
    }
    name ??= '<Unknow Key $vkCode>';
    sb.write('KeyEvent ${toBinary(flags)} ${isKeyUP ? "↑" : "↓"}$name');
    return sb.toString();
  }
}

typedef Listener = void Function(KeyEvent keyEvent);
typedef CancelListening = void Function();

Logger log = Logger();

class KeyBoardState {
  Set<int> state = <int>{};
  KeyBoardState();
  @override
  String toString() {
    if (KeyboardEvent.virtualKeyCode2StringMap != null) {
      var sb = StringBuffer();
      bool isFirst = true;
      sb.write('[');
      for (final key in state) {
        if (isFirst) {
          isFirst = false;
        } else {
          sb.write(',');
        }
        var str = KeyboardEvent.virtualKeyCode2StringMap![key]?[0];
        sb.write(str ?? key.toString());
      }
      sb.write(']');
      return sb.toString();
    } else {
      return state.toString();
    }
  }
}

class KeyboardEvent {
  static const MethodChannel _channel = MethodChannel('keyboard_event');
  static const EventChannel _eventChannel = EventChannel('keyboard_event/event');
  KeyBoardState state = KeyBoardState();
  OnLogCallback? onLog;

  KeyboardEvent({
    this.onLog,
  }) {
    _channel.setMethodCallHandler(_callbackHandler);
  }
  static Future<void> init() async {
    virtualKeyString2CodeMap = await _getVirtualKeyString2CodeMap;
    virtualKeyCode2StringMap = await _getVirtualKeyCode2StringMap;
  }

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  /// Get Virturl-Key Map
  ///
  /// The `await KeyboardEvent.init()` **MUST** be ran before use this
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
  static Map<String, int>? virtualKeyString2CodeMap;

  static Future<Map<String, int>> get _getVirtualKeyString2CodeMap async {
    final Map<String, int> virtualKeyMap = Map<String, int>.from(await _channel.invokeMethod('getVirtualKeyMap', 0));
    return virtualKeyMap;
  }

  /// Virturl-Key Map
  ///
  /// The `await KeyboardEvent.init()` **MUST** be ran before use this
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
  static Map<int, List<String>>? virtualKeyCode2StringMap;

  static Future<Map<int, List<String>>> get _getVirtualKeyCode2StringMap async {
    Map<int, List<String>> ret = {};
    final Map<int, List<dynamic>> virtualKeyMap =
        Map<int, List<dynamic>>.from(await _channel.invokeMethod('getVirtualKeyMap', 1));
    virtualKeyMap.forEach((key, value) {
      ret[key] = List<String>.from(value);
    });
    return ret;
  }

  static bool? isLetter(int vk) {
    if (KeyboardEvent.virtualKeyCode2StringMap != null) {
      var A = KeyboardEvent.virtualKeyString2CodeMap!['A'];
      var Z = KeyboardEvent.virtualKeyString2CodeMap!['Z'];
      if ((vk >= A!) && (vk <= Z!)) {
        return true;
      } else {
        return false;
      }
    }
    return null;
  }

  static bool? isNumber(int vk) {
    if (KeyboardEvent.virtualKeyCode2StringMap != null) {
      var key_0 = KeyboardEvent.virtualKeyString2CodeMap!['0'];
      var key_9 = KeyboardEvent.virtualKeyString2CodeMap!['9'];
      if ((vk >= key_0!) && (vk <= key_9!)) {
        return true;
      } else {
        return false;
      }
    }
    return null;
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
  Future<void> startListening(Listener listener) async {
    var subscription = _eventChannel.receiveBroadcastStream(nextListenerId++).listen(//listener
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
    debugPrint('keyboard_event/event startListening');
    _cancelListening = () {
      subscription.cancel();
      debugPrint('keyboard_event/event canceled');
    };
  }

  Future<void> cancelListening() async {
    if (_cancelListening != null) {
      _cancelListening!();
      _cancelListening = null;
    } else {
      debugPrint('keyboard_event/event No Need');
    }
  }
}
