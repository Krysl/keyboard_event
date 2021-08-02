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
