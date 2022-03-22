/*
 * TODOs:
 * - Check repeated block of code inside the super-loop.
 * - Add flags in order to delete repetition, allowing for increased performance.
 */
#include <avr/sleep.h>
#define RED_LED_PIN 9
#define GREEN_LED1_PIN 7
#define GREEN_LED2_PIN 8
#define GREEN_LED3_PIN 10
#define GREEN_LED4_PIN 11
#define T1_BTN_PIN 2
#define FADE_AMOUNT 0.25
#define FADING_DELAY 1

/*
 * time0, time1, elapsed are used
 * to measure time(ms) elapsed between
 * two end-points.
 */
double time0;
double time1;
double elapsed;
/*
 * For red led fading purposes.
 */
float redLedFadeAmount;
float redLedCurrIntensity;
/*
 * Specifies if game has started or not.
 */
boolean gameStarted = false;
/*
 * Speed of the bouncing ball.
 */
const unsigned long ballSpeed = 500;
unsigned long lastBounceTime;
unsigned long currentTime;
unsigned long elapsedFromLastBounce;
/*
 * Green leds pins bouncing management.
 */
const unsigned int len = 4;
const unsigned int greenLedPins[len] = {GREEN_LED1_PIN, GREEN_LED2_PIN, GREEN_LED3_PIN, GREEN_LED4_PIN};
unsigned int currentGreenLed = 0;
boolean bounceForward = true;
boolean hasStoppedBouncing = false;
unsigned long t1 = random(10000, 20000); // between 10 s and 20 s 
unsigned long startTime;
unsigned long possibleFinalTime;
unsigned long totalElapsed = 0;
/*
 * T2 management.
 */
 unsigned long lastRecordedTime;
 unsigned long currentTime2;
 unsigned long elapsedFromLastRecorded;
 unsigned long t2 = 10000; // 10 s

void setup() {
  redLedCurrIntensity = 0;
  redLedFadeAmount = FADE_AMOUNT;
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED1_PIN, OUTPUT);
  pinMode(GREEN_LED2_PIN, OUTPUT);
  pinMode(GREEN_LED3_PIN, OUTPUT);
  pinMode(GREEN_LED4_PIN, OUTPUT);
  pinMode(T1_BTN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(T1_BTN_PIN), startBouncingLedGame, RISING);
  Serial.begin(9600);
  Serial.println("Welcome to the Catch the Bouncing Led Ball Game. Press Key T1 to Start");  //TODO: fix duplicate message.
  Serial.flush();
  time0 = getCurrentTimeInSeconds();
}

/*
 * Handles the fading of the red led in 
 * the initial part of the program.
 */
void fadingRedLed() {
  analogWrite(RED_LED_PIN, redLedCurrIntensity); //between 0 and 255
  redLedCurrIntensity += redLedFadeAmount;
  if (redLedCurrIntensity >= 255 || redLedCurrIntensity <= 0) {
    redLedFadeAmount = -redLedFadeAmount;
  }
  delay(FADING_DELAY);
}

/*
 * Sets system sleep mode to SLEEP_MODE_PWR_DOWN.
 */
void goToDeepSleep() {
  analogWrite(RED_LED_PIN, 0);
  Serial.println("Going to sleep..");
  Serial.flush();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  detachInterrupt(digitalPinToInterrupt(T1_BTN_PIN));
  attachInterrupt(digitalPinToInterrupt(T1_BTN_PIN), wakeUp, RISING);
  sleep_mode();
}

/*
 * Disables sleep state and wakes up the system.
 */
void wakeUp() {
  Serial.println("Waking up..");
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(T1_BTN_PIN));
  attachInterrupt(digitalPinToInterrupt(T1_BTN_PIN), startBouncingLedGame, RISING);
}

// Simple utility method used to convert millis to secs.
double getCurrentTimeInSeconds() {
  return (double) millis() / 1000;
}

void startBouncingLedGame() {
  gameStarted = true;
  analogWrite(RED_LED_PIN, 0);
  digitalWrite(GREEN_LED1_PIN, 0);
  digitalWrite(GREEN_LED2_PIN, 0);
  digitalWrite(GREEN_LED3_PIN, 0);
  digitalWrite(GREEN_LED4_PIN, 0);
  Serial.println("Go!");
  delay(1000);
  lastBounceTime = millis(); 
  startTime = lastBounceTime;
  digitalWrite(greenLedPins[currentGreenLed], 255);
  detachInterrupt(digitalPinToInterrupt(T1_BTN_PIN));
}

void stopBall() {
  Serial.println("STOPPED BALL");
  hasStoppedBouncing = true;
  digitalWrite(greenLedPins[currentGreenLed], 0);
  lastRecordedTime = millis();
}

// Super loop.
void loop() {
  // FIRST-STAGE
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
  //GAME-STAGE
  else {
    if (!hasStoppedBouncing) {
      currentTime = millis();
      possibleFinalTime = currentTime;
      elapsedFromLastBounce = currentTime - lastBounceTime;
      totalElapsed = possibleFinalTime - startTime;
      Serial.println(totalElapsed);
      if (totalElapsed < t1) {
        if (elapsedFromLastBounce >= ballSpeed) {
          Serial.println("BOUNCING..");
          Serial.println(String("elapsed: ") + elapsedFromLastBounce);
          digitalWrite(greenLedPins[currentGreenLed], 0);
          if (currentGreenLed == 3) {
            bounceForward = false;
          }
          else if (currentGreenLed == 0) {
            bounceForward = true;
          }
          currentGreenLed += bounceForward ? 1 : -1;  
          digitalWrite(greenLedPins[currentGreenLed], 255);
          lastBounceTime = millis();
          elapsedFromLastBounce = 0;
        } 
        else {
          currentTime2 = millis();
          elapsedFromLastRecorded = currentTime - lastBounceTime;
          if (elapsedFromLastRecorded >= t2) {
            //TODO: Game Over
            Serial.println("Game Over. Final Score: XXX");
            gameStarted = false;
          }
          else {
            //TODO: do nothing, waiting for button click.
            //TODO: button interrupt handler has to reset bouncing and change level (F).
          }
        }
      }
      else {
        stopBall();
      }
    }
    else {
    }
  }
}
