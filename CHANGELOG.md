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
- add isLeter, isNumber

## 0.2.0-0
- Migrate example to null-safety;
- add class KeyBoardState to record all pressed keys;

## 0.1.0-0

Use EventChannel to get events
