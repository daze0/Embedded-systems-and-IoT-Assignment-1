/*
 * TODOs:
 * - Check repeated block of code inside the super-loop.
 * - Add flags in order to delete repetition, allowing for increased performance.
 */
#include <EnableInterrupt.h>
#include <avr/sleep.h>
#define RED_LED_PIN 9
#define GREEN_LED1_PIN 7
#define GREEN_LED2_PIN 8
#define GREEN_LED3_PIN 10
#define GREEN_LED4_PIN 11
#define T1_BTN_PIN 2
#define T2_BTN_PIN 3
#define T3_BTN_PIN 12
#define T4_BTN_PIN 13
#define POT_PIN A0
#define FADE_AMOUNT 0.25
#define FADING_DELAY 1
#define DEBOUNCE_TIME 200

/*
 * Factor by which T1 and T2 timeouts are shortened.
 */
unsigned int factor;
const unsigned int lenDifficulties = 8;
const unsigned int difficulties[lenDifficulties] = {50, 100, 200, 400, 800, 1600, 3200, 6400};
unsigned int currentDifficulty = 0;
/*
 * Number of leds and buttons.
 */
const unsigned int len = 4;
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
 * T1 button rebound management.
 */
unsigned long btnLastTime = 0;  //DEFAULT
unsigned long btnCurrentTime;
unsigned long btnElapsed;
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
const unsigned int greenLedPins[len] = {GREEN_LED1_PIN, GREEN_LED2_PIN, GREEN_LED3_PIN, GREEN_LED4_PIN};
unsigned int currentGreenLed = 0;
boolean bounceForward = true;
boolean hasStoppedBouncing = false;
unsigned long t1;
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
/*
 * Score and game buttons management.
 */
const unsigned int gameBtnPins[len] = {T1_BTN_PIN, T2_BTN_PIN, T3_BTN_PIN, T4_BTN_PIN};
int currentBtnPressed = -1;
unsigned int score = 0;
boolean gameOver = false;

void setup() {
  randomSeed(analogRead(1));
  t1 = random(10000, 20000); // between 10 s and 20 s 
  redLedCurrIntensity = 0;
  redLedFadeAmount = FADE_AMOUNT;
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED1_PIN, OUTPUT);
  pinMode(GREEN_LED2_PIN, OUTPUT);
  pinMode(GREEN_LED3_PIN, OUTPUT);
  pinMode(GREEN_LED4_PIN, OUTPUT);
  pinMode(T1_BTN_PIN, INPUT);
  pinMode(T2_BTN_PIN, INPUT);
  pinMode(T3_BTN_PIN, INPUT);
  pinMode(T4_BTN_PIN, INPUT);
  enableInterrupt(T1_BTN_PIN, startBouncingLedGame, RISING);
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
  disableInterrupt(T1_BTN_PIN);
  enableInterrupt(T1_BTN_PIN, wakeUp, RISING);
  enableInterrupt(T2_BTN_PIN, wakeUp, RISING);
  enableInterrupt(T3_BTN_PIN, wakeUp, RISING);
  enableInterrupt(T4_BTN_PIN, wakeUp, RISING);
  sleep_mode();
}

/*
 * Disables sleep state and wakes up the system.
 */
void wakeUp() {
  Serial.println("Waking up..");
  sleep_disable();
  disableInterrupt(T1_BTN_PIN);
  btnLastTime = millis();
  enableInterrupt(T1_BTN_PIN, startBouncingLedGame, RISING);
}

// Simple utility method used to convert millis to secs.
double getCurrentTimeInSeconds() {
  return (double) millis() / 1000;
}

void startBouncingLedGame() {
  Serial.println(String("Difficulty: ") + factor);
  hasStoppedBouncing = false;
  analogWrite(RED_LED_PIN, 0);
  digitalWrite(GREEN_LED1_PIN, 0);
  digitalWrite(GREEN_LED2_PIN, 0);
  digitalWrite(GREEN_LED3_PIN, 0);
  digitalWrite(GREEN_LED4_PIN, 0);
  if (btnLastTime != 0) {
    btnCurrentTime = millis();
    btnElapsed = btnCurrentTime - btnLastTime;
  }
  if ((btnElapsed > DEBOUNCE_TIME) || (btnLastTime == 0)) {
    Serial.println(String("btnElapsed: ") + btnElapsed);
    Serial.println(String("btnLastTime: ") + btnLastTime);
    gameStarted = true;
    Serial.println("Go!");
    delay(1000);
    lastBounceTime = millis(); 
    startTime = lastBounceTime;
    digitalWrite(greenLedPins[currentGreenLed], 255);
    disableInterrupt(T1_BTN_PIN);
    disableInterrupt(T2_BTN_PIN);
    disableInterrupt(T3_BTN_PIN);
    disableInterrupt(T4_BTN_PIN);
  }
}

void stopBall() {
  Serial.println("STOPPED BALL");
  hasStoppedBouncing = true;
  digitalWrite(greenLedPins[currentGreenLed], 0);
  lastRecordedTime = millis();
}

void pressGameButton(int index) {
  currentBtnPressed = index;
  if (currentBtnPressed == currentGreenLed) {
    score += 1;
    hasStoppedBouncing = false;
    lastBounceTime = millis();
    startTime = lastBounceTime;  //Each new level timeout1 is reset.
    currentGreenLed = 0;
    digitalWrite(greenLedPins[currentGreenLed], 255);
    t1 -= (t1 > factor) ? factor : 0;
    t2 -= (t2 > factor) ? factor : 0;
    Serial.println(String("t1: ") + t1);
    Serial.println(String("t2: ") + t2);
  }
  else {
    gameOver = true;
  }
}

void pressGameButton1() {
  pressGameButton(0);
  delay(DEBOUNCE_TIME);
}

void pressGameButton2() {
  pressGameButton(1);
  delay(DEBOUNCE_TIME);
}

void pressGameButton3() {
  pressGameButton(2);
  delay(DEBOUNCE_TIME);
}

void pressGameButton4() {
  pressGameButton(3);
  delay(DEBOUNCE_TIME);
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
    unsigned int potValue = analogRead(POT_PIN);
    if (potValue != currentDifficulty) {
      currentDifficulty = map(potValue, 0, 1023, 0, 8);
      factor = difficulties[currentDifficulty];
    }
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
          
        }
      }
      else {
        stopBall();
        enableInterrupt(T1_BTN_PIN, pressGameButton1, RISING);
        enableInterrupt(T2_BTN_PIN, pressGameButton2, RISING);
        enableInterrupt(T3_BTN_PIN, pressGameButton3, RISING);
        enableInterrupt(T4_BTN_PIN, pressGameButton4, RISING);
      }
    }
    else {
      Serial.println("User has to press btn");
      currentTime2 = millis();
      elapsedFromLastRecorded = currentTime2 - lastRecordedTime;
      if (elapsedFromLastRecorded >= t2 || gameOver) {
        //TODO: Game Over
        Serial.println(String("Game Over. Final Score: ") + score);
        score = 0;
        gameStarted = false;
        gameOver = false;
        time0 = getCurrentTimeInSeconds();
        currentGreenLed = 0;
        currentBtnPressed = -1;
        disableInterrupt(T1_BTN_PIN);
        disableInterrupt(T2_BTN_PIN);
        disableInterrupt(T3_BTN_PIN);
        disableInterrupt(T4_BTN_PIN);
        enableInterrupt(T1_BTN_PIN, startBouncingLedGame, RISING);
        randomSeed(analogRead(1));
        t1 = random(10000, 20000);
        t2 = 10000;
      }
      else {}
    }
  }
}
