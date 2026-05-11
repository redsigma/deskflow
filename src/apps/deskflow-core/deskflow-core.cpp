/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2012 - 2016, 2025 - 2026 Symless Ltd.
 * SPDX-FileCopyrightText: (C) 2002 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "CoreArgParser.h"

#include "arch/Arch.h"
#include "base/EventQueue.h"
#include "base/Log.h"
#include "common/Constants.h"
#include "common/ExitCodes.h"
#include "common/Settings.h"
#include "deskflow/ClientApp.h"
#include "deskflow/ServerApp.h"
#include "deskflow/ipc/CoreIpcServer.h"

#if defined(Q_OS_WIN)
#include "arch/win32/ArchMiscWindows.h"
#endif

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QSharedMemory>
#include <QTextStream>
#include <QThread>

void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
  const auto utf8 = message.toUtf8();
  switch (type) {
  case QtDebugMsg:
    CLOG->print(context.file, context.line, CLOG_TAG_DEBUG "%s", utf8.constData());
    break;
  case QtInfoMsg:
    CLOG->print(context.file, context.line, CLOG_TAG_INFO "%s", utf8.constData());
    break;
  case QtWarningMsg:
    CLOG->print(context.file, context.line, CLOG_TAG_WARN "%s", utf8.constData());
    break;
  case QtCriticalMsg:
    CLOG->print(context.file, context.line, CLOG_TAG_ERR "%s", utf8.constData());
    break;
  case QtFatalMsg:
    CLOG->print(context.file, context.line, CLOG_TAG_CRIT "%s", utf8.constData());
    break;
  }
  if (type == QtFatalMsg) {
    abort();
  }
}

void showHelp(const CoreArgParser &parser)
{
  QTextStream(stdout) << parser.helpText();
}

App *createApp(const bool serverMode, const bool clientMode, EventQueue &events, const QString &processName)
{
  if (serverMode) {
    return new ServerApp(&events, processName);
  }

  if (clientMode) {
    return new ClientApp(&events, processName);
  }

  return nullptr;
}

int main(int argc, char **argv)
{
  Arch arch;
  arch.init();

  Log log;
  qInstallMessageHandler(qtMessageHandler);

  bool serverMode = false;
  bool clientMode = false;

#if defined(Q_OS_WIN)
  ArchMiscWindows::setInstanceWin32(GetModuleHandle(nullptr));

  // Keep a Qt app object alive while parsing args and initializing settings so
  // portableSettingsFile() can safely use applicationDirPath().
  int bootstrapArgc = argc;
  QCoreApplication bootstrapApp(bootstrapArgc, argv);
#endif
  {
    QStringList args;
    for (int i = 0; i < argc; i++)
      args.append(argv[i]);

    CoreArgParser parser(args);

    // Print any parser errors
    if (!parser.errorText().isEmpty()) {
      QTextStream(stdout) << parser.errorText() << "\n";
    }

    if (parser.help()) {
      showHelp(parser);
      return s_exitSuccess;
    }

    if (parser.version()) {
      QTextStream(stdout) << parser.versionText();
      return s_exitSuccess;
    }

    // Before we check any more args we need to check for a duplicate process.
    // Create a shared memory segment with a unique key
    // This is to prevent a new instance from running if one is already running
    QSharedMemory sharedMemory(kCoreBinName);

    // Attempt to attach first and detach in order to clean up stale shm chunks
    // This can happen if the previous instance was killed or crashed
    if (sharedMemory.attach())
      sharedMemory.detach();

    if (!sharedMemory.create(1) && parser.singleInstanceOnly()) {
      LOG_WARN("an instance of deskflow core is already running");
      return s_exitDuplicate;
    }

    parser.parse();
    serverMode = parser.serverMode();
    clientMode = parser.clientMode();

    LOG_INFO("tls trusted servers db: %s", qPrintable(Settings::tlsTrustedServersDb()));
  }

  QApplication app(argc, argv);
  QApplication::setApplicationName(QStringLiteral("%1 Core").arg(kAppName));

  EventQueue events;
  const auto processName = QFileInfo(argv[0]).fileName();

  App *coreApp = createApp(serverMode, clientMode, events, processName);
  if (coreApp == nullptr) {
    qFatal("failed to determine core mode");
    return s_exitFailed;
  }

  const auto ipcServer = new deskflow::core::ipc::CoreIpcServer(&app); // NOSONAR - Qt managed
  QObject::connect(
      ipcServer, &deskflow::core::ipc::IpcServer::stopProcessRequested, coreApp, &App::quit, Qt::DirectConnection
  );
  ipcServer->listen();

  QThread coreThread;
  QObject::connect(&coreThread, &QThread::finished, &app, &QApplication::quit);
  coreApp->run(coreThread);

  int exitCode = QApplication::exec();
  coreThread.wait();

  if (exitCode == s_exitSuccess) {
    exitCode = coreApp->getExitCode();
  }

  LOG_DEBUG("core exited, code: %d", exitCode);
  return exitCode;
}
