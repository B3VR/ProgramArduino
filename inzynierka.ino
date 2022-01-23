#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include <TimerOne.h>

bool isMeasurmetInProgress = false;
int buttonPin = A1;
int buttonNew;
int buttonOld = 1;
// ECG sensor
bool wasElectrodeDisconnected = false;
bool isElectrodeDisconnected = false;
int l0MPin = A2;
int l0PPin = A3;
int signalPin = A5;
// LCD
const int rs = 8, en = 7, d4 = 2, d5 = 3, d6 = 5, d7 = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// SD card
File signalFile;
int csPin = 4;

String fileName = "EKG";

void setup() {
  Serial.begin(19200);
  pinMode(l0MPin, INPUT);
  pinMode(l0PPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(signalPin, INPUT);
  
  Timer1.initialize(4300);
  Timer1.attachInterrupt(saveSample);
  noInterrupts();

  while (!Serial) {
    
  }

  lcd.begin(16, 2);

  buttonNew = digitalRead(buttonPin);
  startMessage();
}

void loop() { 
  checkButtonPush();
}

void checkButtonPush() {
  buttonOld = buttonNew;
  buttonNew = digitalRead(buttonPin);
  
  if (buttonOld == HIGH && buttonNew == LOW)
  {
    if (isMeasurmetInProgress == false)
    {
      isMeasurmetInProgress = true;
      SDissueMessage();
      beginMeasurement();

    }else if (isMeasurmetInProgress == true)
    {
      isMeasurmetInProgress = false;
      startMessage();
    }
  }
}

bool beginMeasurement() {
  if (!SD.begin(csPin)) {
    SDissueMessage();
    delay(1000);
    startMessage();
    isMeasurmetInProgress = !isMeasurmetInProgress;
    return false;
  }

  measurmentInProgressMessage();
  String fullFileName = getNextFileName();  

  signalFile = SD.open(fullFileName, FILE_WRITE);
  
  unsigned long measurmentStartTime = millis();

  while (isMeasurmetInProgress){
    interrupts();
  }
  noInterrupts();

  unsigned long measurmentTime = millis() - measurmentStartTime;
  signalFile.close();

  signalFile = SD.open(fullFileName, FILE_WRITE);

  String header = createHeader(measurmentTime, fullFileName);
  signalFile.println(header);

  signalFile.close();

  wasElectrodeDisconnected = false;
  isElectrodeDisconnected = false;
  signalSavedMessage();
  delay(100000);
  startMessage();
  return true;
}

void saveSample()  {
  bool oldIsElectrodeDisconnected = isElectrodeDisconnected;

  int L0P = digitalRead(l0PPin);
  int L0M = digitalRead(l0MPin);
  
  if (L0M == 1 || L0P == 1) {
    wasElectrodeDisconnected = true;
    isElectrodeDisconnected = true;

  }else {
    isElectrodeDisconnected = false;
    unsigned int sample = analogRead(signalPin);
    Serial.println(sample);   
    signalFile.println(sample);
  }

  bool newIsElectrodeDisconnected = isElectrodeDisconnected;

  checkElectrodeMessage(oldIsElectrodeDisconnected, newIsElectrodeDisconnected);
  checkButtonPush();
}

// Pomocnicze

String createHeader(long measurmentTime, String fileName){
  String header;
  float timeInSec = float(measurmentTime) / 1000;
  header = "Pomiar: " + fileName + " Czas badania: " + timeInSec + " sekund";

  if (wasElectrodeDisconnected)
  {
    header = header + " (Podczas badania odpięto elektrodę! Sygnał może być niepełny)";
  }

  return header;
}

String getNextFileName(){
  int i = 1;
  while (true)
  {
    String fullFileName = fileName + i + ".txt";
    if (SD.exists(fullFileName))
    {
      i++;
    }else {
      return fullFileName;
    }
  } 
}

void checkElectrodeMessage(bool oldIsElectrodeDisconnected, bool newIsElectrodeDisconnected){

  if (oldIsElectrodeDisconnected != newIsElectrodeDisconnected)
  {
    if (newIsElectrodeDisconnected == true)
    {
      disconnectedElectrodeMessage();

    }else if (newIsElectrodeDisconnected == false)
    {
      measurmentInProgressMessage();
    }
  }
}

// Komunikaty wyświetlacza

void startMessage() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Wcisnij przycisk");
  lcd.setCursor(0,1);
  lcd.print("aby rozpoczac");
}

void measurmentInProgressMessage() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Wcisnij ponownie");
  lcd.setCursor(0,1);
  lcd.print("aby zakonczyc");
}

void disconnectedElectrodeMessage() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Elektroda");
  lcd.setCursor(0,1);
  lcd.print("odlaczona !");
}

void SDissueMessage() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Blad karty");
  lcd.setCursor(0,1);
  lcd.print("pamieci !");
}

void signalSavedMessage() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Zapisano badanie");
}