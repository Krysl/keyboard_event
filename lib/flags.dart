// ignore_for_file: constant_identifier_names

/// WM_KEYUP/DOWN/CHAR HIWORD(lParam) flags
const KF_EXTENDED = 0x0100;
const KF_DLGMODE = 0x0800;
const KF_MENUMODE = 0x1000;
const KF_ALTDOWN = 0x2000;
const KF_REPEAT = 0x4000;
const KF_UP = 0x8000;

/// Low level hook flags
const LLKHF_EXTENDED = (KF_EXTENDED >> 8); /* 0x00000001 */
const LLKHF_LOWER_IL_INJECTED = 0x00000002;
const LLKHF_INJECTED = 0x00000010;
const LLKHF_ALTDOWN = (KF_ALTDOWN >> 8); /* 0x00000020 */
const LLKHF_UP = (KF_UP >> 8); /* 0x00000080 */

const LLMHF_INJECTED = 0x00000001;
const LLMHF_LOWER_IL_INJECTED = 0x00000002;

