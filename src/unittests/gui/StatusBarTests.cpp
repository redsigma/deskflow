/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "StatusBarTests.h"

#include "common/Constants.h"
#include "gui/widgets/StatusBar.h"

#include <QLabel>

namespace {

QString statusText(StatusBar &statusBar)
{
  for (const auto *label : statusBar.findChildren<QLabel *>()) {
    if (label->text().contains(kAppName)) {
      return label->text();
    }
  }
  return {};
}

} // namespace

void StatusBarTests::setStatus_serverConnected_replacesConnectingText()
{
  StatusBar statusBar;

  statusBar.setStatus(ConnectionState::Connecting, ProcessState::Started, true);
  QVERIFY(statusText(statusBar).contains(QStringLiteral("connecting")));

  statusBar.setStatus(ConnectionState::Connected, ProcessState::Started, true);

  QCOMPARE(statusText(statusBar), QStringLiteral("%1 is waiting for clients").arg(kAppName));
}

QTEST_MAIN(StatusBarTests)
