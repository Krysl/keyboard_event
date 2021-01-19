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
  List<String> _log = [];
  KeyboardEvent keyboardEvent;

  @override
  void initState() {
    super.initState();
    initPlatformState();
    keyboardEvent = KeyboardEvent(
      onLog: (str) => setState(() {
        if (str == '<Enter>')
          _log.last += '\n';
        else if (str == '<Backspace>') //
          _log.removeLast();
        if (str == '<F5>')
          _log.clear();
        else
          _log.add(str);
      }),
    );
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    try {
      platformVersion = await KeyboardEvent.platformVersion;
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Text('运行于: $_platformVersion\n$_log'),
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
