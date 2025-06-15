#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ------------------ PIN DEFINITIONS ------------------
#define Dir 14
#define Step 27
#define T_1 26
#define T_2 2
#define T_3 15
#define Buzzer 19

// ------------------ VARIABLES ------------------
int T_1time;
int T_2time;
int T_3time;

volatile int x1 = 0;
volatile int x2 = 0;
volatile int x3 = 0;
volatile int x4 = 0;

bool auto1Active = false;
bool auto2Active = false;
bool manual1Active = false;
bool manual2Active = false;

LiquidCrystal_I2C lcd(0x27, 20, 4);

TaskHandle_t countdownTaskHandle = NULL;
SemaphoreHandle_t lcdMutex;

// ------------------ FUNCTIONS ------------------

void Startup_Sound() {
  tone(Buzzer, 2000);
  delay(3000);
  noTone(Buzzer);
}

void Startup_Screen() {
  lcd.setCursor(6, 0); lcd.print("Welcome");
  lcd.setCursor(5, 1); lcd.print("To The Tap");
  lcd.setCursor(4, 2); lcd.print("Density Tester");
  lcd.setCursor(0, 3); lcd.print("");
}

void Idle_Screen() {
  lcd.setCursor(7, 0); lcd.print("Home");
  lcd.setCursor(0,0); lcd.print("       ");
  lcd.setCursor(11,0); lcd.print("          ");
  lcd.setCursor(2, 1); lcd.print("1. Auto Mode");
  lcd.setCursor(14,1); lcd.print("    ");
  lcd.setCursor(2, 2); lcd.print("2. Manual Mode 1");
  lcd.setCursor(2, 3); lcd.print("3. Manual Mode 2");
}

void Auto_Screen1() {
  lcd.clear();
  lcd.setCursor(5, 0); lcd.print("Auto Mode");
  lcd.setCursor(8, 1); lcd.print("In");
  lcd.setCursor(5, 2); lcd.print("Progress");
  lcd.setCursor(10,3); lcd.print("/");
  lcd.setCursor(11,3); lcd.print("500");
}

void Auto_Screen2() {
  lcd.clear();
  lcd.setCursor(5, 0); lcd.print("Auto Mode");
  lcd.setCursor(8, 1); lcd.print("In");
  lcd.setCursor(5, 2); lcd.print("Progress");
  lcd.setCursor(10,3); lcd.print("/");
  lcd.setCursor(11,3); lcd.print("1250");
}

void CoolDown() {
  lcd.clear();
  lcd.setCursor(5, 1); lcd.print("HOLD ON");
}

void Manual_Screen1() {
  lcd.clear();
  lcd.setCursor(3, 0); lcd.print("Manual Mode 1");
  lcd.setCursor(8, 1); lcd.print("In");
  lcd.setCursor(5, 2); lcd.print("Progress");
  lcd.setCursor(10,3); lcd.print("/");
  lcd.setCursor(11,3); lcd.print("500");
 
}

void Manual_Screen2() {
  lcd.clear();
  lcd.setCursor(3, 0); lcd.print("Manual Mode 2");
  lcd.setCursor(8, 1); lcd.print("In");
  lcd.setCursor(5, 2); lcd.print("Progress");
  lcd.setCursor(10,3); lcd.print("/");
  lcd.setCursor(11,3); lcd.print("1250");
}
void SessionComplete_Screen() {
  lcd.clear();
  lcd.setCursor(3, 1); lcd.print("Turn The Switch");
  lcd.setCursor(8,2); lcd.print("OFF");
}

void Motor_Auto() {
  digitalWrite(Step, HIGH);
  digitalWrite(Step, LOW);
  delayMicroseconds(1405);
}

void Motor_Starting_Torque() {
  digitalWrite(Step, HIGH);
  digitalWrite(Step, LOW);
  delayMicroseconds(1950);
}


void updateCounter(uint8_t col, uint8_t row, int val) {
  if (xSemaphoreTake(lcdMutex, portMAX_DELAY)) {
    lcd.setCursor(col, row);
    lcd.print("   ");
    lcd.setCursor(col, row);
    lcd.print(val);
    xSemaphoreGive(lcdMutex);
  }
}

// ------------------ TASK: Countdown Updater ------------------
void countdownTask(void *parameter) {
  while (true) {
    if (auto1Active) x1++;
    else if (auto2Active) x2++;
    else if (manual1Active) x3++;
    else if (manual2Active) x4++;
    vTaskDelay(240 / portTICK_PERIOD_MS);
  }
}

void lcdTask(void *parameter) {
  while (true) {
    if (auto1Active) updateCounter(7, 3, x1);
    else if (auto2Active) updateCounter(7, 3, x2);
    else if (manual1Active) updateCounter(7, 3, x3);
    else if (manual2Active) updateCounter(7, 3, x4);
    vTaskDelay(240 / portTICK_PERIOD_MS);
  }
}

// ------------------ MODE FUNCTIONS ------------------

void runAutoMode() {
  auto1Active = true;
  x1 = 0;
  Auto_Screen1();
  T_1time = millis() / 1000;

  while ((millis() / 1000) - T_1time <= 2 && digitalRead(T_1) == HIGH && digitalRead(T_2) == LOW && digitalRead(T_3) == HIGH) {
    Motor_Starting_Torque();
  }
   while ((millis() / 1000) - T_1time <= 118 && digitalRead(T_1) == HIGH && digitalRead(T_2) == LOW && digitalRead(T_3) == HIGH) {
    Motor_Auto();
  }
  auto1Active = false;

  CoolDown();
  delay(5000);
  
  auto2Active = true;
  x2 = 0;
  Auto_Screen2();
  int tStart = millis() / 1000;

  while ((millis() / 1000) - tStart <= 2 && digitalRead(T_1) == HIGH && digitalRead(T_2) == LOW && digitalRead(T_3) == HIGH) {
    Motor_Starting_Torque();
  }
  while ((millis() / 1000) - tStart <= 298 && digitalRead(T_1) == HIGH && digitalRead(T_2) == LOW && digitalRead(T_3) == HIGH) {
    Motor_Auto();
  }

  auto2Active = false;
}

void runManualMode1() {
  while (true) {
    if (digitalRead(T_2) == LOW && digitalRead(T_2) == LOW) {
      Manual_Screen1();
    }

    if (digitalRead(T_1) == LOW && digitalRead(T_2) == LOW && digitalRead(T_3) == LOW) {
      T_2time = millis() / 1000;
      break;
    }
  }

  manual1Active = true;
  x3 = 0;

  while ((millis() / 1000) - T_2time <= 2 && digitalRead(T_1) == LOW && digitalRead(T_2) == LOW && digitalRead(T_3) == LOW) {
    Motor_Starting_Torque();
  }
  while ((millis() / 1000) - T_2time <= 118 && digitalRead(T_1) == LOW && digitalRead(T_2) == LOW && digitalRead(T_3) == LOW) {
    Motor_Auto();
  }

  manual1Active = false;
}

void runManualMode2() {
  while (true) {
    if (digitalRead(T_3) == HIGH && digitalRead(T_2) == HIGH) {
      Manual_Screen2();
    }

    if (digitalRead(T_1) == LOW && digitalRead(T_2) == HIGH && digitalRead(T_3) == HIGH) {
      T_3time = millis() / 1000;
      break;
    }
  }

  manual2Active = true;
  x4 = 0;

  while ((millis() / 1000) - T_3time <= 2 && digitalRead(T_1) == LOW && digitalRead(T_2) == HIGH && digitalRead(T_3) == HIGH) {
    Motor_Starting_Torque();
  }
  while ((millis() / 1000) - T_3time <= 298 && digitalRead(T_1) == LOW && digitalRead(T_2) == HIGH && digitalRead(T_3) == HIGH) {
    Motor_Auto();
  }

  manual2Active = false;
}

// ------------------ SETUP & LOOP ------------------

void setup() {
  pinMode(Dir, OUTPUT);
  pinMode(Step, OUTPUT);
  digitalWrite(Dir, LOW);

  pinMode(T_1, INPUT_PULLUP);
  pinMode(T_2, INPUT_PULLUP);
  pinMode(T_3, INPUT_PULLUP);

  pinMode(Buzzer, OUTPUT);

  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  lcdMutex = xSemaphoreCreateMutex();

  Startup_Sound();
  Startup_Screen();
  delay(3000);

  xTaskCreatePinnedToCore(countdownTask, "CountdownTask", 2048, NULL, 1, &countdownTaskHandle, 0);
  xTaskCreatePinnedToCore(lcdTask, "LCDTask", 2048, NULL, 1, NULL, 0);
}

void loop() {
  lcd.clear();
  Idle_Screen();

  while (true) {
    int t1 = digitalRead(T_1);
    int t2 = digitalRead(T_2);
    int t3 = digitalRead(T_3);

    if (t1 == HIGH && t2 == LOW && t3 == HIGH) {
      runAutoMode();
      SessionComplete_Screen();
      delay(5000);
    } else if (t2 == LOW && t1 == LOW && t3 == LOW) {
      runManualMode1();
      SessionComplete_Screen();
      delay(5000);
    } else if (t3 == HIGH && t1 == LOW && t2 == HIGH) {
      runManualMode2();
      SessionComplete_Screen();
      delay(5000);
    } else if (t1 == LOW && t2 == LOW && t3 == HIGH) {
      Idle_Screen();
      }

    delay(200);
  }
}
