## 0.3.4
- fix [issues/4 "timestamp.h error C2220"](https://github.com/Krysl/keyboard_event/issues/4) 

## 0.3.2
- fix [issues/3](https://github.com/Krysl/keyboard_event/issues/3) by detect `LLKHF_UP` bit in `flags` of `KBDLLHOOKSTRUCT` structure 

## 0.3.2
- fix `KEYEVENT_DEBUG` enabled build problem when user use pub instead of a git repo

## 0.3.1
- remove spdlog depedence for normal user
  - developer can set variable `KEYEVENT_DEBUG` to `ON` 
    in `windows\CMakeLists.txt` to enable it
    - spdlog will be auto downlowd and compile when you compile this plugin

## 0.3.0-0
- make `_virtualKeyString2CodeMap`,`_virtualKeyCode2StringMap`
 public (remove`_`) for the names
- remove async get functions `virtualKeyString2CodeMap` & 
`virtualKeyCode2StringMap`, the virtual-key map will init 
in the static fun `init()` of `class KeyboardEvent`
- add isLetter, isNumber

## 0.2.0-0
- Migrate example to null-safety;
- add class KeyBoardState to record all pressed keys;

## 0.1.0-0

Use EventChannel to get events
