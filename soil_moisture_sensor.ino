#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "task_scheduler.h"

#define OLED_ADDR   0x3C

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SENSOR_DELAY_READ         20
#define SENSOR_OFF_DELAY          100
#define DISPLAY_OFF_DELAY         5000
#define SENSOR_CONTROL_PIN        8
#define SENSOR_READ_PIN           0
#define BUTTON_INTERRUPT_PIN      3
#define BUTTON_DISABLE_DURATION   2000

struct Task * turnOffDisplayTask = (Task *)malloc(sizeof(Task));

void setup() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN), &handleButtonPress, CHANGE);

  turnOffDisplayTask->isDone = true;
  turnOffDisplayTask->time = 0;
  
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();
}

int previousMoisture = 0;
int previousDisplayFrequency = 0;
byte offset = 155;
int level = 1;
int previousLevel = 0;
volatile boolean buttonPressed = false;
boolean isLevelDisplayed = false;
unsigned int lastInterruptTime = 0;

void handleButtonPress() {
  if(millis() - lastInterruptTime < BUTTON_DISABLE_DURATION) {
    return;
  }

  lastInterruptTime = millis();
  
  Serial.println("Button Pressed");
  buttonPressed = true;

  if (turnOffDisplayTask->isDone) {
    turnOffDisplayTask = scheduleTask(DISPLAY_OFF_DELAY, &turnOffDisplay);
  } else {
    turnOffDisplayTask = resetTask(turnOffDisplayTask);
  }
  
  scheduleTask(SENSOR_OFF_DELAY, &turnOffSensor);
}

void loop() {
  readDisplayFrequency();
  
  if(buttonPressed){
    turnOnSensor();
    float moisture = readMoisture();

    displayMoisture(moisture);
    previousMoisture = moisture;
  
    buttonPressed = false;
  }

  executeTasks();
}

void readDisplayFrequency() {
  int value = analogRead(1);
  
  if (abs(previousDisplayFrequency - value) > 20) {
    level = value/100;

    if(isLevelDisplayed){
      changeDisplayLevel();
    } else {
      displayLevel();
      isLevelDisplayed = true;  
    }

    previousDisplayFrequency = value;
  }
}

int readMoisture(){
  float moisture = analogRead(SENSOR_READ_PIN);
  Serial.println(moisture);
  moisture = moisture - offset;

  return moisture;
}

void turnOffDisplay() {
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  isLevelDisplayed = false;
}

void turnOffSensor() {
  digitalWrite(SENSOR_CONTROL_PIN, LOW);
}

void turnOnSensor(){
  digitalWrite(SENSOR_CONTROL_PIN, HIGH);
  delay(SENSOR_DELAY_READ);
}

void changeDisplayLevel() {
  display.setTextColor(WHITE, BLACK);
  display.setTextSize(2);

  display.setCursor(27,30);

  String toPrint = String(level);
  
  if(level<10){
    toPrint = toPrint+" ";
  }

  display.print(toPrint);
  display.display();
}

void displayLevel() {
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);
  display.print(F("Frequency level:"));

  display.setTextSize(2);
  display.setCursor(27,30);
  
  display.print(level);
  display.print(F(" / 10"));

  display.display();
}

void displayMoisture(int moisture){
  isLevelDisplayed = false;
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);
  display.print(F("Moisture level:"));

  display.setTextSize(2);
  display.setCursor(27,30);

  char formatted[6];
  dtostrf(100.0 - (moisture/(1023.0-offset))*100, 4, 2, formatted);
  
  display.print(formatted);
  display.print(F(" %"));

  display.display();
}

