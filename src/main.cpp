#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

LiquidCrystal lcd(0,2,4,5,6,7);
SoftwareSerial bluetooth(8,9);

#define sensor 23


int NUM_BLADES = 2; // Default number of blades
unsigned int RPM = 0;
unsigned int rpm = 0;

unsigned int RPM1 = 0;
unsigned int rpm1 = 0;

int delay1() {
  lcd.clear();
  lcd.print("reading rpm...");
  int i, j;
  unsigned int count = 0;
  for (i = 0; i < 100; i++) {
    for (j = 0; j < 100; j++) {
      if (digitalRead(sensor)) {
        count++;
        while (digitalRead(sensor));
      }
    }
  }
  return count;
  
}

unsigned long getTime() {
  unsigned long t = millis();
  for (int i = 0; i < 100; i++) {
    while (digitalRead(sensor) == LOW); 
    while (digitalRead(sensor) == HIGH);
  }
  return millis() - t;
}



void receiveBlades() {
  while (bluetooth.available()) {
    char input = bluetooth.read();
    if (input == 'B') {
      // Receive the number of blades from the app
      // read the number of blades from the Bluetooth module
      // and store it in the NUM_BLADES variable
      while (bluetooth.available() == 0); // Wait for data
      int NUM_BLADES = bluetooth.parseInt(); // Parse the received value
      Serial.println("Number of blades =" + NUM_BLADES);
      
    }
  }
}

void setup() {
  pinMode(sensor, INPUT);
  pinMode(2, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("Bluetooth");
  lcd.setCursor(0,1);
  lcd.print("Tachometer");
  delay(500);
  //digitalWrite(start, HIGH);

  bluetooth.begin(9600); // Initialize Bluetooth communication
  Serial.begin(9600);
  Serial.println("Serial monitor initiated");
}

void loop() {
  unsigned int time = 0;

  unsigned int time1 = 0;

  lcd.clear();
  
  lcd.clear();
  lcd.print("Enter no of");
  lcd.setCursor(0,1);
  lcd.print("blades");
  delay(200);
  lcd.clear();
  lcd.print("No. of blades=");
  lcd.setCursor(15,0);
  lcd.print(NUM_BLADES);
  delay(500);
  // Serial.println("Reading RPM.....");
  
  time = delay1();
  time1 = getTime();


  lcd.clear();
  lcd.print("reading complete");
  delay(100);

  lcd.clear();
  lcd.print("Please Wait.....");
  delay(2000);
  Serial.println("Please Wait.....");
  
  receiveBlades(); // Receive the number of blades from the app

  RPM = (60 * 100 * NUM_BLADES) / time;
  RPM1 = 100000/RPM;

  rpm =100*60/(NUM_BLADES*time1/1000);
  rpm1 = rpm/50;

  Serial.println("RPM calculated");
  lcd.clear();
  lcd.print("RPM calculated");

  delay(200);

  lcd.clear();
  lcd.print("RPM=");
  lcd.print(RPM1);
  // lcd.setCursor(0,1);
  // lcd.print(rpm1);
  delay(500);

  Serial.println("RPM=");
  Serial.println(RPM);

  // Send RPM data to the mobile app
  bluetooth.print("RPM=");
  bluetooth.println(RPM);
  delay(1000);
}