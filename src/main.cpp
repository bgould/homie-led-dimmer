#include <Homie.h>
#include <ESPRotary.h>
#include <Button2.h>

#define firmwareVersion "1.0.0"

#define PIN_LED     2
#define ROTARY_PIN1 D5	
#define ROTARY_PIN2	D6
#define BUTTON_PIN  D7

HomieNode lightNode("light", "Light", "switch");
ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2);
Button2 button = Button2(BUTTON_PIN, INPUT_PULLUP, true, 1);

int level = 100;
bool on = false;

void rotate(ESPRotary& r);
void showDirection(ESPRotary& r);
void buttonPressed(Button2& b);

void setLight(bool newState, int newLevel) {
  bool stateChanged = (newState != on);
  bool levelChanged = (newLevel != level);
  on = newState;
  level = newLevel;
  analogWrite(PIN_LED, on ? 1023 - ((level * 1023) / 100) : 1023);
  if (stateChanged) {
    lightNode.setProperty("on").send(on ? "true" : "false");
  }
  if (levelChanged) {
    lightNode.setProperty("level").send(String(level));
  }
  Homie.getLogger() << "Light is " << (on ? "on" : "off") << "; Level: " << level << endl;
}

bool lightOnHandler(const HomieRange& range, const String& value) {
  if (value != "true" && value != "false") return false;
  setLight(value == "true", level);
  return true;
}

bool lightLevelHandler(const HomieRange& range, const String& value) {
  for (byte i = 0; i < value.length(); i++) {
    if (isDigit(value.charAt(i)) == false) return false;
  }

  const unsigned long numericValue = value.toInt();
  if ((numericValue > 100) || (numericValue < 1)) return false;

  setLight(on, numericValue);
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial << endl << endl;

  pinMode(PIN_LED, OUTPUT);
  analogWrite(PIN_LED, 1023);

  Homie_setFirmware("awesome-relay", firmwareVersion);

  lightNode.advertise("on").setName("On").setDatatype("boolean").settable(lightOnHandler);
  lightNode.advertise("level").setName("Level").setDatatype("integer")
           .setFormat("1:100").settable(lightLevelHandler);

  r.setChangedHandler(rotate);
  button.setReleasedHandler(buttonPressed);

  Homie.setup();
}

void loop() {
  r.loop();
  button.loop();
  Homie.loop();
}


// on change
void rotate(ESPRotary& r) {
  int newLevel = level;
  if (r.getDirection() == RE_LEFT) {
    newLevel -= 1;
  } else {
    newLevel += 1;
  }
  if (newLevel < 1) {
    newLevel = 1;
  }
  if (newLevel > 100) {
    newLevel = 100;
  }
  setLight(on, newLevel);
}

void buttonPressed(Button2& b) {
  setLight(!on, level);
}