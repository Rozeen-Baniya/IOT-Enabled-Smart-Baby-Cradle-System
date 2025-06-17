#include <LiquidCrystal.h>
#include <Servo.h>

// Pin assignments
const int buzz = 10;
const int led = 9;
const int c_crit = 40;
const int TMP36 = A0;
const int soilMoisturePin = A2;
const int gasSensorPin = A1;
const int pirPin = 13;

// LCD and Servo
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Servo myServo;

float temp;
int motionState = 0;

void setup() {
  pinMode(buzz, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(pirPin, INPUT);
  myServo.attach(6);
  lcd.begin(16, 2);

  Serial.begin(9600);   // Send data to Arduino 2
}

void loop() {
  // Sensor Readings
  temp = analogRead(TMP36);
  temp = ((temp * 5.0 / 1023.0) - 0.5) * 100;

  int soilRaw = analogRead(soilMoisturePin);
  int soilMoisture = constrain(map(soilRaw, 300, 1023, 0, 100), 0, 100);

  int gasRaw = analogRead(gasSensorPin);
  int gasLevel = constrain(map(gasRaw, 0, 1023, 0, 100), 0, 100);

  motionState = digitalRead(pirPin);

  // Local LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: "); lcd.print(temp); lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Wet:"); lcd.print(soilMoisture); lcd.print("% ");
  lcd.print("G:"); lcd.print(gasLevel); lcd.print("%");
  delay(2000);

  // Send data to Arduino 2 via TX (pin 1)
  Serial.print("T:"); Serial.print(temp);
  Serial.print(",S:"); Serial.print(soilMoisture);
  Serial.print(",G:"); Serial.print(gasLevel);
  Serial.print(",M:"); Serial.print(motionState);
  Serial.print('\n');  // Message terminator

  // Alerts
  if (temp >= c_crit) {
    digitalWrite(led, HIGH);
    playHappySleepSound();
    moveServoFor10Sec();  // Servo moves for high temperature
  } else {
    digitalWrite(led, LOW);
  }

  if (soilMoisture > 25) beepBuzzer(3);   // Wet diaper
  if (gasLevel > 10) beepBuzzer(2);       // Gas alert

  if (motionState == HIGH) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Baby is Moving!");
    digitalWrite(led, HIGH);
    beepBuzzer(1);
    moveServoFor10Sec();  // Servo also moves when baby moves
    delay(2000);
  } else {
    digitalWrite(led, LOW);
  }
}

// Functions
void moveServoFor10Sec() {
  unsigned long start = millis();
  while (millis() - start < 10000) {
    myServo.write(180);
    delay(500);
    myServo.write(0);
    delay(500);
  }
}

void beepBuzzer(int seconds) {
  unsigned long start = millis();
  while (millis() - start < seconds * 1000) {
    tone(buzz, 1000, 200);
    delay(400);
  }
}

void playHappySleepSound() {
  int melody[] = {262, 294, 330, 349, 392};
  int duration[] = {300, 300, 300, 300, 300};

  for (int i = 0; i < 5; i++) {
    tone(buzz, melody[i], duration[i]);
    delay(duration[i] + 50);
  }
  noTone(buzz);
}
