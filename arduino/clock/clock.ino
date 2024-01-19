#define TOGGLE_SWITCH  8
#define SIGNAL_LED    12
#define POWER_LED     11

void blink(uint8_t times) {
  for (uint8_t i = 0; i < times; ++i) {
    digitalWrite(SIGNAL_LED, HIGH);
    delay(250);
    digitalWrite(SIGNAL_LED, LOW);
    delay(250);
  }
  delay(500);
}

void updateTime(const String& time) {
  const uint8_t hour = time.substring(0, 2).toInt();
  const uint8_t minute = time.substring(3, 5).toInt();
  const bool amPm = time[5] == 'P'; // AM = false, PM = true
  blink(hour);
  blink(minute / 10);
  blink(minute % 10);
  blink(amPm ? 2 : 1);
}

void setup() {
  pinMode(TOGGLE_SWITCH, INPUT);
  pinMode(SIGNAL_LED, OUTPUT);
  pinMode(POWER_LED, OUTPUT);
  Serial.begin(9600);
  digitalWrite(POWER_LED, HIGH);
}

void loop() {
  const String data = Serial.readStringUntil('\n');
  if (data.length() > 0) {
    updateTime(data);
    Serial.println("OK1\n");
  }
}
