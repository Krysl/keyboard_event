import 'dart:collection';

import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter/services.dart';
import 'package:keyboard_event/keyboard_event.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  Map<String, int> _virtualKeyMap;
  Map<int, String> _virtualKey2StrMap;
  List<String> _err = [];
  List<String> _log = [];
  List<String> _event = [];
  KeyboardEvent keyboardEvent;
  bool listenIsOn = false;

  @override
  void initState() {
    super.initState();
    initPlatformState();
    keyboardEvent = KeyboardEvent(
      onLog: (str) => setState(() {
        if (str == '<Enter>')
          _event.last += '\n';
        else if (str == '<Backspace>') //
          _event.removeLast();
        if (str == '<F5>')
          _event.clear();
        else
          _event.add(str);
      }),
    );
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    Map<String, int> virtualKeyMap;
    List<String> err = [];
    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      platformVersion = await KeyboardEvent.platformVersion;
    } on PlatformException {
      err.add('Failed to get platform version.');
    }

    try {
      virtualKeyMap = await KeyboardEvent.getVirtualKeyMap;
    } on PlatformException {
      err.add('Failed to get Virtual-Key Map.');
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      if (platformVersion != null) _platformVersion = platformVersion;
      if (virtualKeyMap != null) {
        _virtualKeyMap = virtualKeyMap;
        for (var item in _virtualKeyMap.entries) {
          _log.add('${_log.length}    ${item.key} : ${item.value}, \n');
        }
        _virtualKey2StrMap = {};
        _virtualKeyMap.forEach((key, value) {
          _virtualKey2StrMap[value] = key;
        });
      }
      if (err.isNotEmpty) _err.addAll(err);
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: LayoutBuilder(
          builder: (BuildContext context, BoxConstraints viewportConstraints) {
            return SingleChildScrollView(
              child: Column(
                children: [
                  Text('运行于: $_platformVersion\n}'),
                  Text('event: $_event'),
                  Switch(
                    value: listenIsOn,
                    onChanged: (bool newValue) {
                      setState(() {
                        listenIsOn = newValue;
                        if (listenIsOn == true) {
                          keyboardEvent.startListening((msg) {
                            if (_virtualKey2StrMap != null) {
                              setState(() {
                                var list = List<int>.from(msg);
                                _event.add(_virtualKey2StrMap[list[1]]);
                              });
                            }
                            debugPrint(msg.toString());
                          });
                        } else {
                          keyboardEvent.cancelListening();
                        }
                      });
                    },
                  ),
                  Text(_log.join()),
                ],
              ),
            );
          },
        ),
        floatingActionButton: FloatingActionButton(
          onPressed: () {
            initPlatformState();
          },
        ),
      ),
    );
  }
}
