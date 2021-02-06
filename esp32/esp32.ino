#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "Adafruit_CCS811.h"
#include <Wire.h> 
#include "FS.h"
#include "SD.h"
#include <U8g2lib.h>


//------------Zugangsdaten WLAN ESP32----------------//
const char *ssid = "MyESP32AP";
const char *password = "123456789";


//------------globale Variablen----------------//
float co2;
float tvoc;
float temperatur;
String co2ywert;
int ywert[121];
unsigned long previousMillis = 0;
const long interval = 60000;
String inputMessage1;


//------------Display----------------//
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);


//------------Initalisierung Webserver---------------- //
AsyncWebServer server(80);


//------------Initalisierung Umweltsensor---------------- //
Adafruit_CCS811 ccs;

//------------Initalisierung SD Karte---------------- //
#define SD_CS 5


// ------------Initalisierung LEDs---------------- //
int ledGruen = 14;
int ledGelb = 27;
int ledRot = 26;


// ------------Werte auf SD Karte schreiben SD Karte---------------- //
void writeFile(fs::FS &fs, const char * path, const char * message) {

  Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Error es konnte nicht auf die SD Karte geschrieben werden");
    u8x8.clearDisplay();
    u8x8.setFont(u8x8_font_8x13B_1x2_f);
    u8x8.setCursor(1, 0);
    u8x8.print("Fehler");
    u8x8.setCursor(1, 3);
    u8x8.print("SD Karte");
    
    return;
  }
  if (file.print(message)) {
    Serial.println("Auf die SD Karte geschrieben");
  } else {
    Serial.println("Error es konnte nicht auf die SD Karte geschrieben werden");
    u8x8.clearDisplay();
    u8x8.setFont(u8x8_font_8x13B_1x2_f);
    u8x8.setCursor(1, 0);
    u8x8.print("Fehler");
    u8x8.setCursor(1, 3);
    u8x8.print("SD Karte");
    
  }
  file.close();
}



// ------------Werte an SD Karte anhängen SD Karte---------------- //
void appendFile(fs::FS &fs, const char * path, const char * message) {

  Serial.printf("Appending to file: %s\n", path);
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("File konnte nicht geöffnet werden");
    u8x8.clearDisplay();
    u8x8.setFont(u8x8_font_8x13B_1x2_f);
    u8x8.setCursor(1, 0);
    u8x8.print("Fehler");
    u8x8.setCursor(1, 3);
    u8x8.print("SD Karte");
    return;
  }
  if (file.print(message)) {
    Serial.println("Nachricht angekommen");
  } else {
    Serial.println("Error Nachricht nicht angekommen");
    u8x8.clearDisplay();
    u8x8.setFont(u8x8_font_8x13B_1x2_f);
    u8x8.setCursor(1, 0);
    u8x8.print("Fehler");
    u8x8.setCursor(1, 3);
    u8x8.print("SD Karte");
    
  }
  file.close();
}


// ------------Werte auf SD Karte schreiben---------------- //
void SDCard() {

  //SD Karte Werte schreiben
  String dataMessage = String(temperatur) + ";" + String(co2) + ";" + String(tvoc) + "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/data.csv", dataMessage.c_str());
}

// ------------wird benötigt, damit beim laden der Webseite ein Wert steht---------------- //
String processor(const String& var){
  if(var == "XXXX"){

    return String(co2).c_str();
  }
  else if(var == "XXXXX"){
  
    return String(tvoc).c_str();
  }
  else if(var == "XXXXXX"){
  
    return String(temperatur).c_str();
  }
  return String();
}


// ------------Umweltsensor einlesen/LEDs ansteuern---------------- //
void ledCo2() {

      //Daten zum Testen ausgeben
     // Serial.print("CO2: ");
      //Serial.print(co2);
      //Serial.print("ppm, TVOC: ");
      //Serial.print(tvoc);
      //Serial.print("ppb Temp:");
      //Serial.print(temperatur);
      Serial.print("Nachricht:");
      Serial.println(inputMessage1);

      //LED Ansteuerung
      if (co2 < 700) {
        digitalWrite(ledGruen, HIGH);
        digitalWrite(ledGelb, LOW);
        digitalWrite(ledRot, LOW);
      }

      else if ((co2 >= 700) and (co2 < 1000)) {
        digitalWrite(ledGruen, HIGH);
        digitalWrite(ledGelb, HIGH);
        digitalWrite(ledRot, LOW);
      }

      else if ((co2 >= 1000) and (co2 < 1300)) {
        digitalWrite(ledGruen, LOW);
        digitalWrite(ledGelb, HIGH);
        digitalWrite(ledRot, LOW);
      }

      else if ((co2 >= 1300) and (co2 < 1500)) {
        digitalWrite(ledGruen, LOW);
        digitalWrite(ledGelb, HIGH);
        digitalWrite(ledRot, HIGH);
      }

      else if (co2 >= 1500) {
        digitalWrite(ledGruen, LOW);
        digitalWrite(ledGelb, LOW);
        digitalWrite(ledRot, HIGH);
      }
}


// ------------Display CO2 und Temperatur anzeigen---------------- //
void display() {
 
  //Text CO2 Gehalt
  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  u8x8.setCursor(1, 0);
  u8x8.print("CO2 Gehalt:");

  //Aktueller CO2 Wert
  u8x8.setFont(u8x8_font_courB18_2x3_f);
  u8x8.setCursor(1, 2);
  u8x8.print(String(co2)); 

  //Text Temp.: und aktueller Temperaturwert
  u8x8.setCursor(1, 6);
  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  u8x8.print("Temp.: "+ String(temperatur));
  
}

// ------------Void Setup---------------- //
void setup(){
  
  Serial.begin(115200);
  
  //LEDs definieren
  pinMode(ledGruen, OUTPUT);
  pinMode(ledGelb, OUTPUT);
  pinMode(ledRot, OUTPUT);

  digitalWrite(ledGruen, LOW);
  digitalWrite(ledGelb, LOW);
  digitalWrite(ledRot, LOW);

  //Display
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.clearDisplay();
  
  //Umweltsensor starten
  while (!ccs.begin()) {
    Serial.println("Failed to start sensor! Please check your wiring.");
   
    u8x8.setFont(u8x8_font_8x13B_1x2_f);
    u8x8.setCursor(1, 0);
    u8x8.print("Fehler");
    u8x8.setCursor(1, 3);
    u8x8.print("Umweltsensor");
    
  }

  //Temperatur kalibrieren
  while (!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

  //SD Karte starten
  delay(1000);

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Keine SD Karte erkannt");
  }

  Serial.println("Initialisieren der SD Karte...");
  SD.begin(SD_CS);
  while (!SD.begin(SD_CS)) {
    Serial.println("ERROR - SD Karte  konnte nicht initalisiert werden!");
  
    u8x8.setFont(u8x8_font_8x13B_1x2_f);
    u8x8.setCursor(1, 0);
    u8x8.print("Fehler");
    u8x8.setCursor(1, 3);
    u8x8.print("SD-Karte");
  }


  File file = SD.open("/data.csv");
  if (!file) {
    Serial.println("File existiert nicht");
    Serial.println("Creating file...");
    writeFile(SD, "/data.csv", "Temperatur; CO2; TVOC \r\n");
  } else {
    Serial.println("File exisitiert bereits");
  }
  file.close();

 
  
  //SPIFFS starten
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    
    u8x8.setFont(u8x8_font_8x13B_1x2_f);
    u8x8.setCursor(1, 0);
    u8x8.print("Fehler");
    u8x8.setCursor(1, 3);
    u8x8.print("SPIFFS");
    return;
  }

  //Array für das Aufzeichnen des CO2 Gehalts der letzten zwei Stunden
  for (int i = 0; i<=120; i++){
      ywert[i] = 0;
    }

  //WLAN initalisieren und starten
  WiFi.softAP(ssid, password);
 
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  u8x8.setCursor(1, 0);
  u8x8.print("IP Adresse:");
  u8x8.setCursor(1, 3);
  u8x8.print(WiFi.softAPIP());


  //Einzelnen Abfragen an den Webserver abrufen
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
     if(request->hasParam("co2")){  
        request->send_P(200, "text/plain", String(co2).c_str());
     
     } else if(request->hasParam("yco2")){  
        request->send_P(200, "text/plain", co2ywert.c_str());
     
     } else if(request->hasParam("temp")){  
        request->send_P(200, "text/plain", String(temperatur).c_str());
     
     } else if(request->hasParam("tvoc")){  
        request->send_P(200, "text/plain", String(tvoc).c_str());
     
     } else {
        request->send(SPIFFS, "/esp32.html", String(), false, processor);
     }
    });
 // server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
 //   inputMessage1 = request->getParam("time")->value();
  //});
 //Zu highchart.js navigieren
  server.on("/highcharts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/highcharts.js", "text/javascript");
  });    
  server.begin();

  
  delay(2000);
  u8x8.clearDisplay();
}




// ------------Void loop---------------- //
void loop(){
  //Millisekunden seit dem Systemstart holen
  unsigned long currentMillis = millis();
  
  //Daten aus dem CO2 Sensor einlesen
  if (ccs.available()) {
    if (!ccs.readData()) {
        co2 = ccs.geteCO2();
        tvoc =  ccs.getTVOC();
        temperatur = ccs.calculateTemperature();
    }
  }
 
  //Unterprogramm aufrufen, LED Ansteuerung
  ledCo2();

  //Unterprogramm für die Anzeige des Displays aufrufen
  display();

  //Jede Minute auf die SD Karte schreiben und in das y Array für den Webserver
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    //Unterprogramm für Schreiben auf SD Karte aufrufen
    SDCard();
    
    //Schleife für den y Wert
    //Aufbau Co2ywert = 400 | 300 | 520 |...
    for (int i = 0; i<=120; i++){
      
      if(i == 0){
        co2ywert = "";
      }
      if( i<120){
        ywert[i] = ywert[i+1];
      } else {
        ywert[i]=co2;
      }
      co2ywert = co2ywert + ywert[i] + " | ";
    }
  }

  delay(1000);
  }
