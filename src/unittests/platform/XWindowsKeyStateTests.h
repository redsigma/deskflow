/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QTest>

class XWindowsKeyStateTests : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void shouldReleaseModifierBit_nonToggle_returnsTrue();
  void shouldReleaseModifierBit_toggle_returnsFalse();
  void shouldReleaseModifierBit_unknown_returnsTrue();
};
