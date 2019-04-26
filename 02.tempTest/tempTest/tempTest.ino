#include "ESP8266WiFi.h"
#include <OneWire.h>
#include <DallasTemperature.h>

//USTAWIENIA >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//-------------NIE ZMIENIAJ-------------------------
#define ONE_WIRE_BUS 2  //gdzie jest podpięty termometr - D4
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<KONIEC


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


float getTemp() {
  //-------------------------------------------------
  //Pobierz aktualną temperaturę z sensora 1Wire
  //-------------------------------------------------
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == DEVICE_DISCONNECTED_C)  { 
    Serial.println("Błąd: Brak komunikacji z termometrem.");
    return(-1);
  } else {
    return(tempC);
  }
}

//--------------------------------------------------------------------------------


void setup() {
  //----------------------------------------------------------------------------
  // Ustawienie płytki - wołane jeden raz, bezpośrednio po uruchomieniu programu
  //----------------------------------------------------------------------------
  Serial.begin(115200);
  Serial.println("");

  //Opóźnienie startu - abyśmy zdążyli podłączyć się z terminalem
  for (int i=5; i>0; i--) {
    Serial.print("Start: ");
    Serial.println(i);
    delay(1000);
  }
  
  //setup płytki
  Serial.println ("Setup.");
  sensors.begin();

}

void loop() {
  //----------------------------------------------
  // główna pętla kodu - wołana ciągle
  // ---------------------------------------------

  Serial.print("Temperatura: ");
  Serial.println(getTemp()) ;
  delay(1000);
  
}
