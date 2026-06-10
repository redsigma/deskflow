/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025-2026 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "CoreIpcClient.h"

#include "common/Constants.h"

#include <QDebug>
#include <QLocalSocket>
#include <QObject>
#include <QString>

namespace deskflow::gui::ipc {

CoreIpcClient::CoreIpcClient(QObject *parent) : IpcClient(parent, kCoreIpcName, QStringLiteral("core"))
{
  // do nothing
}

void CoreIpcClient::sendStop()
{
  sendMessage(QStringLiteral("stop"));
}

void CoreIpcClient::sendProbeKeyDown(const QString &keyStroke)
{
  sendMessage(QStringLiteral("probeKeyDown=%1").arg(keyStroke));
}

void CoreIpcClient::sendProbeKeyUp(const QString &keyStroke)
{
  sendMessage(QStringLiteral("probeKeyUp=%1").arg(keyStroke));
}

void CoreIpcClient::sendProbeKeyRepeat(const QString &keyStroke, int32_t repeatCount)
{
  const auto count = (repeatCount > 0) ? repeatCount : 1;
  sendMessage(QStringLiteral("probeKeyRepeat=%1|%2").arg(keyStroke, QString::number(count)));
}

void CoreIpcClient::processCommand(const QString &command, const QStringList &parts)
{
  const auto args = parts.size() >= 2 ? parts.at(1) : QString();
  Q_EMIT commandReceived(command, args);
}

} // namespace deskflow::gui::ipc
