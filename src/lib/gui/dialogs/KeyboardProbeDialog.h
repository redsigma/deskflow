/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include <QDialog>
#include <QStringList>
#include <cstdint>

class QLabel;
class QPlainTextEdit;
class QCloseEvent;
class QShowEvent;
class QKeyEvent;

class KeyboardProbeDialog : public QDialog
{
  Q_OBJECT

public:
  explicit KeyboardProbeDialog(QWidget *parent = nullptr);
  ~KeyboardProbeDialog() override = default;

Q_SIGNALS:
  void probeKeyDown(const QString &keyStroke);
  void probeKeyUp(const QString &keyStroke);
  void probeKeyRepeat(const QString &keyStroke, int32_t repeatCount);

protected:
  void closeEvent(QCloseEvent *event) override;
  void showEvent(QShowEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

private:
  void appendKeyEvent(const QString &eventType, const QKeyEvent *event);
  void appendLine(const QString &line);
  QString describeModifiers(Qt::KeyboardModifiers modifiers) const;
  QString toKeyStroke(const QKeyEvent *event) const;
  QString keyToDeskflowName(int key) const;
  static Qt::KeyboardModifier modifierForKey(int key);
  static bool isModifierKey(int key);

  QLabel *m_statusLabel = nullptr;
  QPlainTextEdit *m_eventLog = nullptr;
  QStringList m_eventLines;
};
