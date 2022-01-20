#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

bool isMeasurmetInProgress = false;
int buttonPin = A1;
int buttonNew;
int buttonOld = 1;
// ECG sensor
bool wasElectrodeDisconneted = false;
int l0MPin = A2;
int l0PPin = A3;
int signalPin = A5;
// LCD
const int rs = 8, en = 7, d4 = 2, d5 = 3, d6 = 5, d7 = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
// SD card
File signalFile;
String fileName = "EKG";

void setup() {
  Serial.begin(9600);
  pinMode(l0MPin, INPUT);
  pinMode(l0PPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(signalPin, INPUT);
  
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
      measurmentInProgressMessage();
      beginMeasurement();

    }else if (isMeasurmetInProgress == true)
    {
      isMeasurmetInProgress = false;
      startMessage();
    }
  }
}

bool beginMeasurement() {
  if (!SD.begin(4)) {
    SDissueMessage();
    delay(1000);
    startMessage();
    isMeasurmetInProgress = !isMeasurmetInProgress;
    return false;
  }

  String fullFileName = getNextFileName();  

  signalFile = SD.open(fullFileName, FILE_WRITE);
  signalFile = SD.open(fullFileName, FILE_WRITE);
  
  unsigned long measurmentStartTime = millis();
  
  while (isMeasurmetInProgress){
    saveSample();
  }
    
  unsigned long measurmentTime = millis() - measurmentStartTime;

  String header = createHeader(measurmentTime, fullFileName);
  signalFile.println(header);
  signalFile.close();
  wasElectrodeDisconneted = false;
  return true;
}

void saveSample() {
  int newL0P = digitalRead(l0PPin);
  int newL0M = digitalRead(l0MPin);
  
  if (newL0M == 1 || newL0P == 1) {
    disconnectedElectrodeMessage();
    wasElectrodeDisconneted = true;

  }else {
    measurmentInProgressMessage();
    float sample = analogRead(signalPin);
    Serial.println(sample);   
    signalFile.println(sample);
  }

  checkButtonPush();
}

// Pomocnicze

String createHeader(long measurmentTime, String fileName){
  String header;
  float timeInSec = float(measurmentTime) / 1000;
  header = "Pomiar: " + fileName + " Czas badania: " + timeInSec + " sekund";

  if (wasElectrodeDisconneted)
  {
    header = header + " (Podczas badania odpięto elektrodę ! Sygnał może być niepełny)";
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

// void test() {
//   SD.begin(4);
  
//   String fullFileName = getNextFileName();  
//   signalFile = SD.open(fullFileName, FILE_WRITE);
//   signalFile = SD.open(fullFileName, FILE_WRITE);

//   unsigned long measurmentStartTime = millis();
//   unsigned long koniecBadania = 0;
//   while (koniecBadania <= 10000)
//   {
//     saveSample();
//     koniecBadania = millis() - measurmentStartTime;
//   }
  
//   String header = createHeader(koniecBadania);
//   signalFile.println(header);
//   signalFile.close();
//   wasElectrodeDisconneted = false;

// }