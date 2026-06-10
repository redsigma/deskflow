/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025-2026 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "CoreIpcServer.h"

#include "base/Log.h"
#include "common/Constants.h"
#include "deskflow/App.h"
#include "deskflow/AppUtil.h"
#include "deskflow/KeyMap.h"
#include "deskflow/KeyTypes.h"
#include "deskflow/ServerApp.h"
#include "server/Server.h"

#include <QLocalSocket>

namespace deskflow::core::ipc {

static CoreIpcServer *s_instance = nullptr;

CoreIpcServer::CoreIpcServer(QObject *parent) : IpcServer(parent, kCoreIpcName, QStringLiteral("core"))
{
  assert(s_instance == nullptr);
  s_instance = this;
}

CoreIpcServer &CoreIpcServer::instance()
{
  assert(s_instance != nullptr);
  return *s_instance;
}

void CoreIpcServer::processCommand(QLocalSocket *clientSocket, const QString &command, const QStringList &parts)
{
  if (command == QStringLiteral("stop")) {
    LOG_DEBUG("core ipc server got stop message");
    writeToClientSocket(clientSocket, QStringLiteral("ok"));
    broadcastCommand(QStringLiteral("bye"));
    Q_EMIT stopProcessRequested();
    return;
  }

  if (command == QStringLiteral("probeKeyDown")) {
    if (parts.size() < 2) {
      LOG_ERR("core ipc server got probeKeyDown command with missing args");
      return;
    }
    auto *server = dynamic_cast<::ServerApp *>(&::App::instance());
    if (server == nullptr || server->getServerPtr() == nullptr) {
      LOG_ERR("core ipc server can't run probeKeyDown, server not running");
      return;
    }
    std::string keySpec = parts.at(1).toStdString();
    KeyModifierMask mask = 0;
    KeyID key = kKeyNone;
    if (!KeyMap::parseModifiers(keySpec, mask) || !KeyMap::parseKey(keySpec, key)) {
      LOG_ERR("core ipc server failed to parse probeKeyDown key \"%s\"", qPrintable(parts.at(1)));
      return;
    }
    server->getServerPtr()->injectKeyDown(key, mask, 0, AppUtil::instance().getCurrentLanguageCode(), nullptr);
    writeToClientSocket(clientSocket, QStringLiteral("ok"));
    return;
  }

  if (command == QStringLiteral("probeKeyUp")) {
    if (parts.size() < 2) {
      LOG_ERR("core ipc server got probeKeyUp command with missing args");
      return;
    }
    auto *server = dynamic_cast<::ServerApp *>(&::App::instance());
    if (server == nullptr || server->getServerPtr() == nullptr) {
      LOG_ERR("core ipc server can't run probeKeyUp, server not running");
      return;
    }
    std::string keySpec = parts.at(1).toStdString();
    KeyModifierMask mask = 0;
    KeyID key = kKeyNone;
    if (!KeyMap::parseModifiers(keySpec, mask) || !KeyMap::parseKey(keySpec, key)) {
      LOG_ERR("core ipc server failed to parse probeKeyUp key \"%s\"", qPrintable(parts.at(1)));
      return;
    }
    server->getServerPtr()->injectKeyUp(key, mask, 0, nullptr);
    writeToClientSocket(clientSocket, QStringLiteral("ok"));
    return;
  }

  if (command == QStringLiteral("probeKeyRepeat")) {
    if (parts.size() < 2) {
      LOG_ERR("core ipc server got probeKeyRepeat command with missing args");
      return;
    }
    const auto argParts = parts.at(1).split(QChar('|'));
    if (argParts.isEmpty()) {
      LOG_ERR("core ipc server got probeKeyRepeat command with malformed args");
      return;
    }
    auto *server = dynamic_cast<::ServerApp *>(&::App::instance());
    if (server == nullptr || server->getServerPtr() == nullptr) {
      LOG_ERR("core ipc server can't run probeKeyRepeat, server not running");
      return;
    }
    std::string keySpec = argParts[0].toStdString();
    KeyModifierMask mask = 0;
    KeyID key = kKeyNone;
    if (!KeyMap::parseModifiers(keySpec, mask) || !KeyMap::parseKey(keySpec, key)) {
      LOG_ERR("core ipc server failed to parse probeKeyRepeat key \"%s\"", qPrintable(argParts[0]));
      return;
    }

    auto count = 1;
    if (argParts.size() > 1) {
      bool ok = false;
      count = argParts[1].toInt(&ok);
      if (!ok || count < 1) {
        LOG_WARN(
            "core ipc server got invalid probeKeyRepeat count, using 1 (key=%s, count=%s)", qPrintable(argParts[0]),
            qPrintable(argParts[1])
        );
        count = 1;
      }
    }

    server->getServerPtr()->injectKeyRepeat(
        key, mask, static_cast<int32_t>(count), 0, AppUtil::instance().getCurrentLanguageCode()
    );
    writeToClientSocket(clientSocket, QStringLiteral("ok"));
    return;
  }

  LOG_WARN("core ipc server got unknown command: %s", command.toUtf8().constData());
}

} // namespace deskflow::core::ipc
