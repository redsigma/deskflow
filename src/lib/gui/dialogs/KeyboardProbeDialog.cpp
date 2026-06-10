/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Deskflow Developers
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "KeyboardProbeDialog.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QFont>
#include <QKeyEvent>
#include <QLabel>
#include <QPlainTextEdit>
#include <QShowEvent>
#include <QVBoxLayout>

KeyboardProbeDialog::KeyboardProbeDialog(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("Keyboard Probe"));
  setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
  setFixedSize(720, 420);
  setFocusPolicy(Qt::StrongFocus);

  auto *layout = new QVBoxLayout(this);
  m_statusLabel = new QLabel(tr("Focus this window and press/release keys."), this);
  layout->addWidget(m_statusLabel);

  m_eventLog = new QPlainTextEdit(this);
  m_eventLog->setReadOnly(true);
  m_eventLog->setLineWrapMode(QPlainTextEdit::NoWrap);
  m_eventLog->setFont(QFont(QStringLiteral("Monospace"), 9));
  layout->addWidget(m_eventLog);
}

void KeyboardProbeDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);
  setFocus();
  grabKeyboard();
}

void KeyboardProbeDialog::closeEvent(QCloseEvent *event)
{
  releaseKeyboard();
  QDialog::closeEvent(event);
}

void KeyboardProbeDialog::appendLine(const QString &line)
{
  m_eventLines.push_front(line);
  if (m_eventLines.size() > 400) {
    m_eventLines.removeLast();
  }

  m_eventLog->setPlainText(m_eventLines.join(QStringLiteral("\n")));
}

QString KeyboardProbeDialog::describeModifiers(Qt::KeyboardModifiers modifiers) const
{
  QStringList activeModifiers;
  if (modifiers.testFlag(Qt::ShiftModifier))
    activeModifiers.push_back(QStringLiteral("Shift"));
  if (modifiers.testFlag(Qt::ControlModifier))
    activeModifiers.push_back(QStringLiteral("Ctrl"));
  if (modifiers.testFlag(Qt::AltModifier))
    activeModifiers.push_back(QStringLiteral("Alt"));
  if (modifiers.testFlag(Qt::MetaModifier))
    activeModifiers.push_back(QStringLiteral("Meta"));
  if (modifiers.testFlag(Qt::KeypadModifier))
    activeModifiers.push_back(QStringLiteral("Keypad"));

  if (activeModifiers.isEmpty()) {
    return QStringLiteral("none");
  }

  return activeModifiers.join(QStringLiteral(", "));
}

QString KeyboardProbeDialog::toKeyStroke(const QKeyEvent *event) const
{
  const auto key = event->key();
  const int keyValue = key & ~static_cast<int>(Qt::KeypadModifier);
  auto modifiers = event->modifiers();
  auto includeAltGr = isModifierKey(key) && key == Qt::Key_AltGr;
  if (!includeAltGr &&
      ((modifiers & (Qt::ControlModifier | Qt::AltModifier)) == (Qt::ControlModifier | Qt::AltModifier)) &&
      (key != Qt::Key_Control) && (key != Qt::Key_Alt)) {
    includeAltGr = true;
    modifiers &= ~(Qt::ControlModifier | Qt::AltModifier);
  }

  if (isModifierKey(key)) {
    modifiers &= ~modifierForKey(key);
  }
  if (includeAltGr) {
    modifiers &= ~(Qt::ControlModifier | Qt::AltModifier);
  }

  QStringList modifierParts;
  if (modifiers & Qt::ShiftModifier) {
    modifierParts.push_back(QStringLiteral("Shift"));
  }
  if (modifiers & Qt::ControlModifier) {
    modifierParts.push_back(QStringLiteral("Control"));
  }
  if (modifiers & Qt::AltModifier) {
    modifierParts.push_back(QStringLiteral("Alt"));
  }
  if (modifiers & Qt::MetaModifier) {
    modifierParts.push_back(QStringLiteral("Meta"));
  }
  if (includeAltGr) {
    modifierParts.push_back(QStringLiteral("AltGr"));
  }

  const QString keyName = keyToDeskflowName(keyValue);
  if (keyName.isEmpty()) {
    return {};
  }

  if (modifierParts.isEmpty()) {
    return keyName;
  }
  return modifierParts.join(QStringLiteral("+")) + QStringLiteral("+") + keyName;
}

QString KeyboardProbeDialog::keyToDeskflowName(int key) const
{
  switch (key) {
  case Qt::Key_Space:
    return QStringLiteral("Space");
  case Qt::Key_Escape:
    return QStringLiteral("Escape");
  case Qt::Key_Tab:
    return QStringLiteral("Tab");
  case Qt::Key_Backtab:
    return QStringLiteral("LeftTab");
  case Qt::Key_Backspace:
    return QStringLiteral("BackSpace");
  case Qt::Key_Return:
    return QStringLiteral("Return");
  case Qt::Key_Insert:
    return QStringLiteral("Insert");
  case Qt::Key_Delete:
    return QStringLiteral("Delete");
  case Qt::Key_Pause:
    return QStringLiteral("Pause");
  case Qt::Key_Print:
    return QStringLiteral("Print");
  case Qt::Key_SysReq:
    return QStringLiteral("SysReq");
  case Qt::Key_Home:
    return QStringLiteral("Home");
  case Qt::Key_End:
    return QStringLiteral("End");
  case Qt::Key_Left:
    return QStringLiteral("Left");
  case Qt::Key_Up:
    return QStringLiteral("Up");
  case Qt::Key_Right:
    return QStringLiteral("Right");
  case Qt::Key_Down:
    return QStringLiteral("Down");
  case Qt::Key_PageUp:
    return QStringLiteral("PageUp");
  case Qt::Key_PageDown:
    return QStringLiteral("PageDown");
  case Qt::Key_CapsLock:
    return QStringLiteral("CapsLock");
  case Qt::Key_NumLock:
    return QStringLiteral("NumLock");
  case Qt::Key_ScrollLock:
    return QStringLiteral("ScrollLock");
  case Qt::Key_Menu:
    return QStringLiteral("Menu");
  case Qt::Key_Help:
    return QStringLiteral("Help");
  case Qt::Key_Enter:
    return QStringLiteral("KP_Enter");
  case Qt::Key_Clear:
    return QStringLiteral("Clear");
  case Qt::Key_HomePage:
    return QStringLiteral("WWWHome");
  case Qt::Key_Favorites:
    return QStringLiteral("WWWFavorites");
  case Qt::Key_Search:
    return QStringLiteral("WWWSearch");
  case Qt::Key_Back:
    return QStringLiteral("WWWBack");
  case Qt::Key_Forward:
    return QStringLiteral("WWWForward");
  case Qt::Key_Stop:
    return QStringLiteral("WWWStop");
  case Qt::Key_Refresh:
    return QStringLiteral("WWWRefresh");
  case Qt::Key_VolumeDown:
    return QStringLiteral("AudioDown");
  case Qt::Key_VolumeMute:
    return QStringLiteral("AudioMute");
  case Qt::Key_VolumeUp:
    return QStringLiteral("AudioUp");
  case Qt::Key_MediaPlay:
    return QStringLiteral("AudioPlay");
  case Qt::Key_MediaStop:
    return QStringLiteral("AudioStop");
  case Qt::Key_MediaPrevious:
    return QStringLiteral("AudioPrev");
  case Qt::Key_MediaNext:
    return QStringLiteral("AudioNext");
  case Qt::Key_Standby:
    return QStringLiteral("Sleep");
  case Qt::Key_LaunchMail:
    return QStringLiteral("AppMail");
  case Qt::Key_LaunchMedia:
    return QStringLiteral("AppMedia");
  case Qt::Key_Launch0:
    return QStringLiteral("AppUser1");
  case Qt::Key_Launch1:
    return QStringLiteral("AppUser2");
  case Qt::Key_Select:
    return QStringLiteral("Select");
  case Qt::Key_Plus:
    return QStringLiteral("Plus");
  case Qt::Key_Comma:
    return QStringLiteral("Comma");
  case Qt::Key_Semicolon:
    return QStringLiteral("Semicolon");
  case Qt::Key_Control:
    return QStringLiteral("Control");
  case Qt::Key_Alt:
    return QStringLiteral("Alt");
  case Qt::Key_Shift:
    return QStringLiteral("Shift");
  case Qt::Key_Meta:
    return QStringLiteral("Meta");
  case Qt::Key_AltGr:
    return QStringLiteral("AltGr");
  default:
    break;
  }

  if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
    return QStringLiteral("F%1").arg(key - Qt::Key_F1 + 1);
  }

  if (key > 0x20 && key < 0x7f) {
    return QChar(key).toLower();
  }

  return QStringLiteral("\\u%1").arg(key & 0xffff, 4, 16, QChar('0')).toUpper();
}

bool KeyboardProbeDialog::isModifierKey(int key)
{
  return key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta ||
         key == Qt::Key_AltGr;
}

Qt::KeyboardModifier KeyboardProbeDialog::modifierForKey(int key)
{
  switch (key) {
  case Qt::Key_Shift:
    return Qt::ShiftModifier;
  case Qt::Key_Control:
    return Qt::ControlModifier;
  case Qt::Key_Alt:
    return Qt::AltModifier;
  case Qt::Key_Meta:
    return Qt::MetaModifier;
  case Qt::Key_AltGr:
    return Qt::AltModifier;
  default:
    return Qt::NoModifier;
  }
}

void KeyboardProbeDialog::appendKeyEvent(const QString &eventType, const QKeyEvent *event)
{
  const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
  const auto line =
      tr("[%1] %2 key=0x%3(%4) text=%5 scan=0x%6 nativeVk=0x%7 modifiers=%8 nativeModifiers=0x%9")
          .arg(
              timestamp, eventType, QString::number(event->key(), 16).toUpper(),
              event->text().isEmpty() ? QStringLiteral("(none)") : QString::fromLatin1(event->text().toUtf8()),
              QString::number(event->nativeScanCode(), 16).toUpper(),
              QString::number(event->nativeVirtualKey(), 16).toUpper(), describeModifiers(event->modifiers()),
              QString::number(event->nativeModifiers(), 16).toUpper()
          );

  appendLine(line);
  m_statusLabel->setText(tr("Captured %1 in %2").arg(eventType, event->text()));
}

void KeyboardProbeDialog::keyPressEvent(QKeyEvent *event)
{
  const QString stroke = toKeyStroke(event);
  if (!stroke.isEmpty()) {
    if (event->isAutoRepeat()) {
      const auto repeatCount = event->count() < 1 ? 1 : event->count();
      Q_EMIT probeKeyRepeat(stroke, static_cast<int32_t>(repeatCount));
    } else {
      Q_EMIT probeKeyDown(stroke);
    }
  }
  appendKeyEvent(tr("DOWN"), event);
  event->accept();
}

void KeyboardProbeDialog::keyReleaseEvent(QKeyEvent *event)
{
  const QString stroke = toKeyStroke(event);
  if (!stroke.isEmpty()) {
    Q_EMIT probeKeyUp(stroke);
  }
  appendKeyEvent(tr("UP"), event);
  event->accept();
}
