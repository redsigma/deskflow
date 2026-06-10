/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025-2026 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "IpcClient.h"

#include <QObject>
#include <cstdint>

namespace deskflow::gui::ipc {

class CoreIpcClient : public IpcClient
{
  Q_OBJECT

public:
  explicit CoreIpcClient(QObject *parent = nullptr);

  void sendStop();
  void sendProbeKeyDown(const QString &keyStroke);
  void sendProbeKeyUp(const QString &keyStroke);
  void sendProbeKeyRepeat(const QString &keyStroke, int32_t repeatCount);

Q_SIGNALS:
  void commandReceived(const QString &command, const QString &args);

protected:
  void processCommand(const QString &command, const QStringList &parts) override;
};

} // namespace deskflow::gui::ipc
