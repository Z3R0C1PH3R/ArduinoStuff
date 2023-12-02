#include "DigiKeyboard.h"

void setup() {
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.sendKeyStroke(KEY_R, MOD_GUI_LEFT);
  DigiKeyboard.delay(200);
  DigiKeyboard.println("cmd");
  DigiKeyboard.delay(1000);
  DigiKeyboard.println("color 0A");
  DigiKeyboard.delay(100);
}


void loop() {
  DigiKeyboard.println("ipconfig");
  DigiKeyboard.delay(500);
  DigiKeyboard.println("tree");
  DigiKeyboard.delay(1000);
  DigiKeyboard.println("help");
  DigiKeyboard.delay(200);
  DigiKeyboard.println("echo You are Hacked!?!?!?!?!");
  DigiKeyboard.delay(2000);
}
