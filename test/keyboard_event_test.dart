import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:keyboard_event/keyboard_event.dart';

void main() {
  const MethodChannel channel = MethodChannel('keyboard_event');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect(await KeyboardEvent.platformVersion, '42');
  });
}
