/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2025 Chris Rizzitello <sithlord48@gmail.com>
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2011 Nick Bolton
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "KeyStateTests.h"
#include "base/EventQueue.h"
#include "deskflow/KeyMap.h"

#include "MockEventQueue.h"
#include "MockKeyMap.h"
#include "MockKeyState.h"

#include <vector>

namespace {
class TrackingKeyState : public KeyState
{
public:
  TrackingKeyState(IEventQueue *events, deskflow::KeyMap &keyMap) : KeyState(events, keyMap, {"en"}, true)
  {
  }

  bool fakeCtrlAltDel() override
  {
    return false;
  }

  KeyModifierMask pollActiveModifiers() const override
  {
    return m_polledMask;
  }

  int32_t pollActiveGroup() const override
  {
    return 0;
  }

  void pollPressedKeys(KeyButtonSet &) const override
  {
  }

  void getKeyMap(deskflow::KeyMap &) override
  {
  }

  void fakeKey(const Keystroke &keystroke) override
  {
    m_fakeKeys.push_back(keystroke);
  }

  bool fakeMediaKey(KeyID) override
  {
    return false;
  }

  void setPolledMask(KeyModifierMask mask)
  {
    m_polledMask = mask;
  }

  int releaseCount() const
  {
    int count = 0;
    for (const auto &key : m_fakeKeys) {
      if (key.m_type == Keystroke::KeyType::Button && !key.m_data.m_button.m_press) {
        ++count;
      }
    }
    return count;
  }

private:
  std::vector<Keystroke> m_fakeKeys;
  KeyModifierMask m_polledMask = 0;
};

deskflow::KeyMap::KeyItem makeKeyItem(KeyID id, KeyButton button)
{
  deskflow::KeyMap::KeyItem item;
  item.m_id = id;
  item.m_group = 0;
  item.m_button = button;
  item.m_required = 0;
  item.m_sensitive = 0;
  item.m_generates = 0;
  item.m_lock = false;
  item.m_client = 0;
  return item;
}
} // namespace

void KeyStateTests::initTestCase()
{
  m_arch.init();
}

void KeyStateTests::keyDown()
{
  deskflow::KeyMap keyMap;
  EventQueue eventQueue;
  MockKeyState keyState(eventQueue, keyMap);

  keyState.onKey(1, true, KeyModifierAlt);

  QVERIFY(keyState.getKeyState(1));
}

void KeyStateTests::keyUp()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);
  QVERIFY(!keyState.getKeyState(1));
}

void KeyStateTests::invalidKey()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  keyState.onKey(0, true, KeyModifierAlt);

  QVERIFY(!keyState.getKeyState(0));
}

void KeyStateTests::onKey_aKeyDown_keyStateOne()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  keyState.onKey(1, true, KeyModifierAlt);

  QVERIFY(keyState.getKeyState(1));
}

void KeyStateTests::onKey_aKeyUp_keyStateZero()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  keyState.onKey(1, false, KeyModifierAlt);

  QVERIFY(!keyState.getKeyState(1));
}

void KeyStateTests::onKey_invalidKey_keyStateZero()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  keyState.onKey(0, true, KeyModifierAlt);

  QVERIFY(!keyState.getKeyState(0));
}

void KeyStateTests::updateKeyState_pollDoesNothing_keyNotSet()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  keyState.updateKeyState();

  QVERIFY(!keyState.isKeyDown(1));
}

void KeyStateTests::updateKeyState_activeModifiers_maskNotSet()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  keyState.updateKeyState();

  QCOMPARE(0, keyState.getActiveModifiers());
}

void KeyStateTests::fakeKeyRepeat_invalidKey_returnsFalse()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  QVERIFY(!keyState.fakeKeyRepeat(0, 0, 0, 0, "en"));
}

void KeyStateTests::fakeKeyRepeat_zeroServerButton_returnsFalse()
{
  EventQueue eventQueue;
  deskflow::KeyMap keyMap;
  keyMap.addKeyEntry(makeKeyItem(0x41, 10));
  keyMap.finish();

  TrackingKeyState keyState(&eventQueue, keyMap);
  keyState.fakeKeyDown(0x41, 0, 10, "en");

  QVERIFY(!keyState.fakeKeyRepeat(0x41, 0, 1, 0, "en"));
}

void KeyStateTests::fakeKeyUp_buttonNotDown_returnsFalse()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  QVERIFY(!keyState.fakeKeyUp(0));
}

void KeyStateTests::fakeKeyDown_maskedServerButton_keyUpWorks()
{
  EventQueue eventQueue;
  deskflow::KeyMap keyMap;
  keyMap.addKeyEntry(makeKeyItem(0x41, 10));
  keyMap.finish();

  TrackingKeyState keyState(&eventQueue, keyMap);
  const KeyButton serverButton = IKeyState::s_numButtons + 10;
  keyState.fakeKeyDown(0x41, 0, serverButton, "en");

  QVERIFY(keyState.fakeKeyUp(serverButton));
}

void KeyStateTests::fakeKeyDown_zeroServerButton_keyUpIgnored()
{
  EventQueue eventQueue;
  deskflow::KeyMap keyMap;
  keyMap.addKeyEntry(makeKeyItem(0x41, 10));
  keyMap.finish();

  TrackingKeyState keyState(&eventQueue, keyMap);
  keyState.fakeKeyDown(0x41, 0, 0, "en");

  QVERIFY(!keyState.fakeKeyUp(0));
}

void KeyStateTests::fakeAllKeysUp_modifierKey_clearsStateAndMapping()
{
  EventQueue eventQueue;
  deskflow::KeyMap keyMap;
  auto modifierItem = makeKeyItem(kKeyAlt_L, 20);
  deskflow::KeyMap::initModifierKey(modifierItem);
  keyMap.addKeyEntry(modifierItem);
  keyMap.finish();

  TrackingKeyState keyState(&eventQueue, keyMap);
  keyState.setPolledMask(0);
  keyState.fakeKeyDown(kKeyAlt_L, 0, 20, "en");
  keyState.fakeAllKeysUp();

  QVERIFY(keyState.releaseCount() > 0);
  QCOMPARE(keyState.getActiveModifiers(), KeyModifierMask(0));
  QVERIFY(!keyState.fakeKeyUp(20));
}

void KeyStateTests::isKeyDown_noKeysDown_returnsFalse()
{
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, m_keymap);

  QVERIFY(!keyState.isKeyDown(1));
}

void KeyStateTests::isKeyDown_keyDown_retrunsTrue()
{
  MockKeyMap keyMap;
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, keyMap);

  deskflow::KeyMap::KeyItem key;
  key.m_button = 1;
  keyState.fakeKeyDown(1, 0, 1, "en");

  QVERIFY(keyState.isKeyDown(1));
}

void KeyStateTests::updateKeyState_pollInsertsSingleKey_keyIsDown()
{
  MockKeyMap keyMap;
  MockEventQueue eventQueue;
  MockKeyState keyState(eventQueue, keyMap);

  deskflow::KeyMap::KeyItem key;
  key.m_button = 1;
  keyState.fakeKeyDown(1, 0, 1, "en");

  keyState.updateKeyState();
  QVERIFY(keyState.isKeyDown(1));
}

QTEST_MAIN(KeyStateTests)
