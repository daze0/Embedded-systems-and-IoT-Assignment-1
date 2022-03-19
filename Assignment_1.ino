#include <avr/sleep.h>
#define RED_LED_PIN 9
#define T1_BTN_PIN 2
#define FADE_AMOUNT 0.25
#define FADING_DELAY 1

const long start_timeout = 10000000;

unsigned long time0;
unsigned long time1;
unsigned long elapsed;
float redLedFadeAmount;
float redLedCurrIntensity;
boolean gameStarted = false;

void setup() {
  // put your setup code here, to run once:
  redLedCurrIntensity = 0;
  redLedFadeAmount = FADE_AMOUNT;
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(T1_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(T1_BTN_PIN), doNothing, RISING);
  Serial.begin(9600);
  Serial.println("Welcome to the Catch the Bouncing Led Ball Game. Press Key T1 to Start");  //TODO: fix duplicate message.
  Serial.flush();
  time0 = getCurrentTimeInSeconds();
}

void fadingRedLed() {
  analogWrite(RED_LED_PIN, redLedCurrIntensity); //between 0 and 255
  redLedCurrIntensity += redLedFadeAmount;
  if (redLedCurrIntensity >= 255 || redLedCurrIntensity <= 0) {
    redLedFadeAmount = -redLedFadeAmount;
  }
  delay(FADING_DELAY);
}

void goToDeepSleep() {
  Serial.println("Going to sleep..");
  Serial.flush();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  detachInterrupt(digitalPinToInterrupt(T1_BTN_PIN));
  attachInterrupt(digitalPinToInterrupt(T1_BTN_PIN), wakeUp, RISING);
  sleep_mode();
}

void wakeUp() {
  Serial.println("Waking up..");
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(T1_BTN_PIN));
  attachInterrupt(digitalPinToInterrupt(T1_BTN_PIN), doNothing, RISING);
}

unsigned long getCurrentTimeInSeconds() {
  return millis() / 1000;
}

void doNothing() {}

void loop() {
  // put your main code here, to run repeatedly:
  if (!gameStarted) {
    time1 = getCurrentTimeInSeconds();
    elapsed = time1 - time0;
    if (elapsed >= 10) {
      goToDeepSleep();
      elapsed = 0;
      time0 = getCurrentTimeInSeconds();
    }
    fadingRedLed();
  }
  else {
    //TODO: program game.
  }
}
