#include "keyboard_event.h"
#include "debug.h"
#include "key.h"
#include "keyboard_event_plugin.h"

TCHAR comboChars[4] = _T("<->");

BOOL visibleShift = FALSE;
BOOL visibleModifier = TRUE;
BOOL onlyCommandKeys = FALSE;
BOOL keyAutoRepeat = TRUE;

/**
 * @brief Get the Symbol From virtual-key code
 *
 * @param vk The virtual-key code. See [Virtual-Key
 * Codes](https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes)
 * @param sc The hardware scan code of the key.
 * @param mod
 * @param hklLayout The input locale identifier used to translate the specified
 * code. This parameter can be any input locale identifier previously returned
 * by the
 * [LoadKeyboardLayout](https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-loadkeyboardlayouta)
 * function.
 * @return LPCTSTR
 */
LPCTSTR GetSymbolFromVK(UINT vk, UINT sc, BOOL mod, HKL hklLayout) {
  static TCHAR symbol[32];
  BYTE btKeyState[256];
  // WORD Symbol = 0;
  TCHAR cc[2];
  if (mod) {
    ZeroMemory(btKeyState, sizeof(btKeyState));
  } else {
    for (int i = 0; i < 256; i++) {
      btKeyState[i] = (BYTE)GetKeyState(i);
    }
  }
  // 将指定的虚拟键代码和键盘状态转换为相应的一个或多个Unicode字符。https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-tounicodeex
  int rr = ToUnicodeEx(vk, sc, btKeyState, cc, 2, 0, hklLayout);
#ifdef _DEBUG
  TCHAR ss[KL_NAMELENGTH];
  GetKeyboardLayoutName(ss);
  tostringstream line;
  line << vk << ":" << rr << ":" << sc << "\n";
  log(line);
#endif
  if (rr > 0) {
    if (!visibleShift && mod && GetKeyState(VK_SHIFT) < 0) {
      // prefix "Shift - " only when Ctrl or Alt is hold (mod as TRUE)
      swprintf(symbol, 32, L"Shift %c %s", comboChars[1], cc);
    } else {
      swprintf(symbol, 32, L"%s", cc);
      symbol[rr] = L'\0';
    }
    return symbol;
  }
  return NULL;
}

/**
 * @brief 获取 virtual-key code 对应的字符串名称
 *
 * @param vk virtual-key code
 * @return LPCTSTR 字符串名称
 */
LPCTSTR getSpecialKey(UINT vk) {
  static TCHAR unknown[32];
  for (size_t i = 0; i < nSpecialKeys; ++i) {
    if (specialKeys[i].val == vk) {
      return specialKeys[i].label;
    }
  }
  swprintf(unknown, 32, L"0x%02x", vk);
  return unknown;
}

void addBracket(LPWSTR str) {
  TCHAR tmp[64];
  if (wcslen(comboChars) > 2) {
    swprintf(tmp, 64, L"%s", str);
    swprintf(str, 64, L"%c%s%c", comboChars[0], tmp, comboChars[2]);
  }
}

LPCTSTR getModSpecialKey(UINT vk, BOOL mod = FALSE) {
  static TCHAR modsk[64];
  if (vk == 0xA0 || vk == 0xA1) {
    if (!mod) {
      // show nothing if press SHIFT only
      return NULL;
    } else {
      wcscpy_s(modsk, 64, L"Shift");
    }
  } else {
    TCHAR tmp[64];
    LPCTSTR sk = getSpecialKey(vk);
    if (!visibleShift && GetKeyState(VK_SHIFT) < 0) {
      // prefix "Shift - "
      swprintf(tmp, 64, L"Shift %c %s", comboChars[1], sk);
      sk = tmp;
    }
    if (!mod && HIBYTE(sk[0]) == 0) {
      // if the special key is not used with modifierkey, and has not been
      // replaced with visible symbol then surround it with <>
      swprintf(modsk, 64, L"%s", sk);
      addBracket(modsk);
    } else {
      swprintf(modsk, 64, L"%s", sk);
    }
  }

  return modsk;
}

// remove a modifier vk from modifierkeys
// for example, remove "Alt" from "Ctrl - Alt"
void cleanModifier(UINT vk, LPWSTR modifierkeys) {
  TCHAR tmp[64];
  LPCTSTR ck = getSpecialKey(vk);
  if (modifierkeys == NULL) return;
  LPWSTR p = wcsstr(modifierkeys, ck);
  if (p == modifierkeys) {
    if (wcslen(modifierkeys) == wcslen(p)) {
      // current key is the only modifier
      modifierkeys[0] = '\0';
    } else {
      // remove current key and the " - " after it
      // sizeof(" - ") == 4
      wcscpy_s(tmp, 64, modifierkeys + wcslen(ck) + 4);
      wcscpy_s(modifierkeys, 64, tmp);
    }
  } else if (p) {
    // get rid of all after current key including the delimiter
    *(p - 3) = '\0';
  }
}
static TCHAR modifierkey[64] = L"\0";
static BOOL modifierUsed = FALSE;

/**
 * @brief LowLevelKeyboardProc回调函数
 * https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644985(v=vs.85)
 * @param nCode
 * @param wp
 * 键盘消息的标识符。此参数可以是以下消息之一：WM_KEYDOWN，WM_KEYUP，WM_SYSKEYDOWN
 * 或 WM_SYSKEYUP。
 * @param lp 指向 KBDLLHOOKSTRUCT 结构的指针。
 * @return LRESULT
 */
LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wp, LPARAM lp) {
  KBDLLHOOKSTRUCT k = *(KBDLLHOOKSTRUCT *)lp;
  TCHAR c[64] = L"\0";
  TCHAR tmp[64] = L"\0";
  const TCHAR *theKey = NULL;

  if (nCode < 0) return CallNextHookEx(kbdhook, nCode, wp, lp);

  static DWORD lastvk = 0;
  UINT spk = visibleShift ? 0xA0 : 0xA2;
  GUITHREADINFO Gti;
  ::ZeroMemory(&Gti, sizeof(GUITHREADINFO));
  Gti.cbSize = sizeof(GUITHREADINFO);
  ::GetGUIThreadInfo(0, &Gti);
  DWORD dwThread = ::GetWindowThreadProcessId(Gti.hwndActive, 0);
  HKL hklLayout = ::GetKeyboardLayout(dwThread);
  UINT isDeadKey =
      ((MapVirtualKeyEx(k.vkCode, MAPVK_VK_TO_CHAR, hklLayout) & 0x80000000) >>
       31);
  if (isDeadKey) {
    // GetKeyboardState(btKeyState);
    // for(int i = 0; i < 256; i++) {
    // btKeyState[i] = (BYTE)GetKeyState(i);
    // }
    // deadKeyPressed = TRUE;
  } else if (wp == WM_KEYUP || wp == WM_SYSKEYUP) {
    lastvk = 0;
    // fadeLastLabel(TRUE);
    if (k.vkCode >= spk && k.vkCode <= 0xA5 || k.vkCode == 0x5B ||
        k.vkCode == 0x5C) {
      // cleanModifier(k.vkCode, modifierkey);
      modifierUsed = FALSE;
    }
  } else if (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN) {
    if (!keyAutoRepeat && lastvk == k.vkCode) {
      // fadeLastLabel(FALSE);
      return TRUE;
    }
    int fin = 0;
    if (k.vkCode >= spk && k.vkCode <= 0xA5 ||   // ctrl / alt
        k.vkCode == 0x5B || k.vkCode == 0x5C) {  // win
      LPCTSTR ck = getSpecialKey(k.vkCode);
      if (modifierkey[0] == '\0') {
        wcscpy_s(modifierkey, 64, ck);
      } else if (!wcsstr(modifierkey, ck)) {
        wcscpy_s(tmp, 64, modifierkey);
        swprintf(modifierkey, 64, L"%s %c %s", tmp, comboChars[1], ck);
      }
      if (!modifierUsed && visibleModifier) {
        swprintf(c, 64, L"%s", modifierkey);
        addBracket(c);
        if (lastvk == k.vkCode) {
          showText(c, 2);
        } else {
          showText(c, 1);
        }
      }
    } else {
      // WORD a = 0;
      BOOL mod = modifierkey[0] != '\0';
      if (k.vkCode == 0x08 || k.vkCode == 0x09 || k.vkCode == 0x0D ||
          k.vkCode == 0x1B || k.vkCode == 0x20) {
        // for <BS>/<Tab>/<ENTER>/<ESC>/<SPACE>, treat them as specialKeys
        theKey = getModSpecialKey(k.vkCode, mod);
        fin = 1;
      } else {
        theKey = GetSymbolFromVK(k.vkCode, k.scanCode, mod, hklLayout);
        if (!theKey) {
          // otherwise try to translate with ToAsciiEx
          // if fails to translate with ToAsciiEx, then treat it as specialKeys
          theKey = getModSpecialKey(k.vkCode, mod);
          fin = 1;
        }
      }

      if (theKey) {
        if (mod) {
          fin = 1;
          swprintf(tmp, 64, L"%s %c %s", modifierkey, comboChars[1], theKey);
          addBracket(tmp);
          theKey = tmp;
        }
        if (fin || !onlyCommandKeys) {
          showText(theKey, fin);
        }
      }
    }
    lastvk = k.vkCode;
  }

  return CallNextHookEx(kbdhook, nCode, wp, lp);
}