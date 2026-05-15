/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "XWindowsKeyStateTests.h"

#include "deskflow/KeyTypes.h"
#include "platform/XWindowsKeyState.h"

void XWindowsKeyStateTests::shouldReleaseModifierBit_nonToggle_returnsTrue()
{
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitShift));
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitControl));
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitAlt));
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitMeta));
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitSuper));
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitAltGr));
}

void XWindowsKeyStateTests::shouldReleaseModifierBit_toggle_returnsFalse()
{
  QVERIFY(!XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitCapsLock));
  QVERIFY(!XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitNumLock));
  QVERIFY(!XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitScrollLock));
}

void XWindowsKeyStateTests::shouldReleaseModifierBit_unknown_returnsTrue()
{
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(kKeyModifierBitNone));
  QVERIFY(XWindowsKeyState::shouldReleaseModifierBit(31));
}

QTEST_MAIN(XWindowsKeyStateTests)
