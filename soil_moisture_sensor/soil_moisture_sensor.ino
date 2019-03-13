#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "task_scheduler.h"

#define OLED_ADDR   0x3C

#define SCREEN_WIDTH        128 // OLED display width, in pixels
#define SCREEN_HEIGHT       64  // OLED display height, in pixels
#define OLED_RESET          -1  // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SENSOR_DELAY_READ         20
#define SENSOR_OFF_DELAY          100
#define DISPLAY_OFF_DELAY         5000
#define SENSOR_CONTROL_PIN        8
#define SENSOR_READ_PIN           0
#define BUTTON_INTERRUPT_PIN      3
#define BUTTON_DISABLE_DURATION   500

struct Task * turnOffDisplayTask = (Task *)malloc(sizeof(Task));
struct Task * doubleClickTimeout = (Task *)malloc(sizeof(Task));

int previousMoisture = 0;
int previousDisplayFrequency = 0;
byte offset = 155;
byte level = 1;
byte previousLevel = 0;
volatile boolean buttonPressed = false;
volatile boolean buttonDoublePressed = false;
boolean isLevelDisplayed = false;
unsigned long lastInterruptTime = 0;
float moisture = 0;

void setup() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT_PIN), &button1Interrupt, CHANGE);

  turnOffDisplayTask->isDone = true;
  turnOffDisplayTask->time = 0;

  doubleClickTimeout->isDone = true;
  doubleClickTimeout->time = 0;
  
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.display();

  previousDisplayFrequency = analogRead(1);

  scheduleTask(1000, &readMoisture, true);
}

void button1Interrupt() {
  if(millis() - lastInterruptTime < BUTTON_DISABLE_DURATION) {
    return;
  }

  lastInterruptTime = millis();
  
  Serial.println("Button Pressed");

  if(!doubleClickTimeout->isDone){
    buttonDoublePressed = true;
  }
  
  buttonPressed = true;

  scheduleTurnOffDisplay();

  doubleClickTimeout = scheduleTask(1000, NULL, false);
  
  scheduleTask(SENSOR_OFF_DELAY, &turnOffSensor, false);
}

void loop() {
  handleKnob();
  handleDoubleButton();
  handleSingleButton();
  
  executeTasks();
}

void handleDoubleButton(){
  if(buttonDoublePressed){
    Serial.println(F("Button double pressed !"));
    buttonDoublePressed = false;
    buttonPressed = false;
    displaySettings();
  }
}

void handleSingleButton(){
  if(buttonPressed){
    turnOnSensor();
    readMoisture();

    displayMoisture(moisture);
    previousMoisture = moisture;
  
    buttonPressed = false;
  }
}

void handleKnob() {
  int value = analogRead(1);
  
  if (abs(previousDisplayFrequency - value) > 80) {
    scheduleTurnOffDisplay();
    
    level = (value+40)/100;

    if(isLevelDisplayed){
      changeDisplayLevel();
    } else {
      displayLevel();
      isLevelDisplayed = true;  
    }

    previousDisplayFrequency = value;
  }
}

void readMoisture(){
  turnOnSensor();
  float currentMoisture = analogRead(SENSOR_READ_PIN);
  Serial.print(F("Moisture level - "));
  Serial.println(currentMoisture);
  moisture = currentMoisture - offset;
  turnOffSensor();
}

void scheduleTurnOffDisplay(){
  if (turnOffDisplayTask->isDone) {
    turnOffDisplayTask = scheduleTask(DISPLAY_OFF_DELAY, &turnOffDisplay, false);
  } else {
    turnOffDisplayTask = resetTask(turnOffDisplayTask);
  }
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
  delay(20);
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

void displaySettings() {
  isLevelDisplayed = false;
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);
  display.print(F("Settings :"));

  display.setCursor(0,16);
  display.print(F("Sensitivity :"));

  display.setCursor(0,24);
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
