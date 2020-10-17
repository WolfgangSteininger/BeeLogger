/* BEE - LOGGER Mastermodul
   Arduino UNO mit Wireless Communication  HC-12
   Anzeige Display 4x20
   Echtzeituhr

   Version 2.0:     14.10.2020
    Sendeaufforderung jede Stunde (dzt 10sec) durch Senden von Stocknummer
    dzt. nur #1
    Daten über erstes Zeichen identifiziert
    Datenspeicherung auf SD-Karte
    Alarm bei deutlichem Gewichtsverlust -> Schwarm
*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2C address
#include <SoftwareSerial.h>
//#define rxPin 5
//#define txPin 4
SoftwareSerial HC12(4, 5); // HC-12 TX Pin, HC-12 RX Pin
#include <RTClib.h>
#include <SPI.h> // SPI für SD Card
#include <SD.h> // 
const String LogFileName = "Daten.csv";
const int chipSelect = 10; // Für Datenlogger Modul am Arduino Uno so lassen
RTC_DS1307 rtc;
//rtc.adjust(DateTime(2020,16,10,14,39,00));

//#define TasterPin 8

char SerialBytein;
char HC12Bytein;
String HC12ReadBuffer = "";
String SerialReadBuffer = "";
String Stocknummer = "";
String Datum = "";
String Zeit = "";
String hum_out = "";
String tmp_out = "";
String hum_in = "";
String tmp_in = "";
String Gewicht = "";
String Batt = "";
String kennung = "";
String rest = "";

boolean HC12End = false;
int laenge;
int fall = 0;
unsigned long Millis;
unsigned long Millis_alt;
unsigned long Millis_alt_senden;
unsigned long time_dif;
unsigned long intervall = 10000;  // 30 sek
//************************************************************
void setup() {
  Serial.begin(9600);             // Serial port to computer
  lcd.begin(20, 4);
  lcd.clear();
  lcd.setBacklight(LOW);
  lcd.setBacklight(HIGH);
  lcd.setCursor(0, 0);
  lcd.print("Init SD-Card...");
  //if (!SD.begin(chipSelect)){lcd.setCursor(0,1);lcd.print("fehlgeschlagen!");while (1);}
  lcd.setCursor(0, 1); lcd.print("war erfolgreich!");
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Init RTC...");
  if (! rtc.begin()) {
    lcd.setCursor(0, 1);
    lcd.print("RTC nicht gefunden!");
    while (1);
  }
  //rtc.adjust(DateTime(2020,10,16,18,46,00)); //ACHTUNG: Nur zum Setzten der RTC
  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Abfrage RTC...");
  if (! rtc.isrunning()) {
    lcd.setCursor(0, 1); lcd.print("RTC laeuft nicht!  ");
    lcd.setCursor(0, 2); lcd.print("Bitte initalisieren"); while (1);
  }
  lcd.setCursor(0, 1); lcd.print("RTC funktionsbereit");
  delay(3000);

  if (!(SD.exists(LogFileName))) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("File oeffnen...");
    File LogFilePtr = SD.open(LogFileName, FILE_WRITE); // Datei zum Schreiben öffnen
    if (LogFilePtr) { // Wenn Datei zum schreiben bereit ist, schreibe Kopfzeile
      LogFilePtr.println("Tag;Monat;Jahr;Std;Min;");//Stock;temp_out;hum_out;temp_in;hum_in;Gewicht;Batt;");
      //LogFilePtr.println("Start");//Stock;temp_out;hum_out;temp_in;hum_in;Gewicht;Batt;");
      lcd.setCursor(0, 1);
      lcd.print("LogDatei erzeugt...");
      LogFilePtr.close(); // Datei schließen
    }
  }
  delay(3000);


  //pinMode(TasterPin, INPUT_PULLUP);
  HC12.begin(9600);               // Serial port to HC12
  if (HC12.isListening()) {
    Serial.println("OK");
  }
  else {
    Serial.println("NOK");
  }

  //test HC-12
  Serial.print("HC-12 available... ");
  HC12.write("AT+DEFAULT");
  delay(1000);
  while (HC12.available() > 0) {
    Serial.write(HC12.read());
  }
  Serial.println();
  Serial.println("initialization done");

  HC12ReadBuffer.reserve(64);
  //SerialReadBuffer.reserve(64);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    BEE - LOGGER");
  lcd.setCursor(0, 2);
  lcd.print("warte auf Daten....");

} //End SetUp
//***************************************************
void loop() {
  DateTime now = rtc.now(); // Hole aktuelle Uhrzeit aus RTC

  // HC12 Empfangen************************
  while (HC12.available()) {        // If HC-12 has data
    HC12Bytein = HC12.read();
    if ( char (HC12Bytein) == 10 || char (HC12Bytein == 13) ) {} //CR und LF nicht speichern
    else {
      HC12ReadBuffer += char(HC12Bytein);
    }
    //Serial.print(HC12ReadBuffer);
    if (HC12Bytein == '\n') {
      HC12End = true;
    }
  }
  if (HC12End) {
    Serial.println(HC12ReadBuffer);
    Serial.println("----------------");
    kennung = HC12ReadBuffer.substring(0, 1); // Erstes Zeichen ist Kennung
    rest = HC12ReadBuffer.substring(1);     // alle Zeichen ab dem Ersten  sind Daten

    if (kennung == "A") {
      Stocknummer = rest;
      lcd.setCursor(18, 3);
      lcd.print("#");
      lcd.print(Stocknummer);
    }
    if (kennung == "B") {
      Zeit = rest;
      lcd.setCursor(0, 0);
      lcd.print(Zeit);
    }
    if (kennung == "C") {
      Datum = rest;
      lcd.setCursor(10, 0);
      lcd.print(Datum);
    }
    if (kennung == "D") {
      tmp_out = rest;
      lcd.setCursor(0, 1);
      lcd.print(tmp_in);
      lcd.print(" C");
    }
    if (kennung == "E") {
      hum_out = rest;
      lcd.setCursor(10, 1);
      lcd.print(hum_in);
      lcd.print(" %");
    }
    if (kennung == "F") {
      tmp_in = rest;
      lcd.setCursor(0, 2);
      lcd.print(tmp_out);
      lcd.print(" C");
    }
    if (kennung == "G") {
      hum_in = rest;
      lcd.setCursor(10, 2);
      lcd.print(hum_out);
      lcd.print(" %");
    }
    if (kennung == "H") {
      Gewicht = rest;
      lcd.setCursor(0, 3);
      lcd.print(Gewicht);
      lcd.print(" kg");
    }
    if (kennung == "I") {
      Batt = rest;
      lcd.setCursor(10, 3);
      lcd.print(Batt);
      lcd.print(" V");
    }
    /*
      String dataString = ""; // erzeuge neuen Eintrag für CSV Datei
      //dataString = String(now.day())+ ";" + String(now.month())+ ";" + String(now.year())+ ";";
      //dataString += String(now.hour()) + ";" + String(now.minute()) + ";" + String(now.second()) + ";";
      //dataString = Stocknummer +";" + tmp_in + ";" + hum_in + ";" + tmp_out + ";" + hum_out + ";" + Gewicht + ";" + Batt + ";";
      File LogFilePtr = SD.open(LogFileName, FILE_WRITE); // Datei zum Schreiben öffnen.
      if (LogFilePtr){ // Wenn Datei zum schreiben bereit ist, schreibe hinein
      //LogFilePtr.println(dataString);
      LogFilePtr.println("ABC");
      LogFilePtr.close(); // Datei schließen
      Serial.println(dataString); // Auf serieller Schnittstelle auch ausgeben
      dataString="";
      }
      else{ Serial.println("Fehler beim oeffnen");}
    */
    HC12ReadBuffer = "";
    HC12End = false;
  }
  if ((millis() - Millis_alt_senden) > intervall) { //  hier Anforderungsintervall eingeben
    lcd.clear();
    Millis_alt_senden = millis();
    HC12.println("1");
  }   // Daten von Station 1 anfordern




} //Ende loop
