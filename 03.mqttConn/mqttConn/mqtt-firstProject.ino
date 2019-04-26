#include "ESP8266WiFi.h"
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//USTAWIENIA >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//-------------NIE ZMIENIAJ-------------------------
#define PROJECT "MyFirstMqtt"
#define ONE_WIRE_BUS 2  //gdzie jest podpięty termometr - D4
#define MySSID "MDSNET"
#define MyPass "kumpel123"
const char MqttUrl[]="messaging.internetofthings.ibmcloud.com";

//-----------------ZMIEŃ-------------------------------
const char MyMqttOrg[]="AAA";              // ZMIEŃ
const char MyMqttDevType[]="AAA";     // ZMIEŃ
const char MyMqttDevName[]="AAA";           // ZMIEŃ
const char MqttToken[]="AAA";// ZMIEŃ

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<KONIEC

WiFiClient ethClient;
PubSubClient mqttclient;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned int iteracja; //do pętli loop.


void wifi_printError() {
  //-------------------------------------------------------------
  //Wyświetl informacje o błędzie w podłączeniu WIFI
  //-------------------------------------------------------------
  switch (WiFi.status()) {
    case WL_IDLE_STATUS:
        Serial.println("WL_IDLE_STATUS");
        break;
    case WL_NO_SSID_AVAIL:
        Serial.println("WL_NO_SSID_AVAIL");
        break;
    case WL_SCAN_COMPLETED:
        Serial.println("WL_SCAN_COMPLETED");
        break;
    case WL_CONNECTED:
        Serial.println("WL_CONNECTED");
        break;
    case WL_CONNECT_FAILED:
        Serial.println("WL_CONNECT_FAILED");
        break;
    case WL_CONNECTION_LOST:
        Serial.println("WL_CONNECTION_LOST");
        break;
    case WL_DISCONNECTED:
        Serial.println("WL_DISCONNECTED");
        break;
  }
}

void mqtt_printError() {
  //-------------------------------------------------------------
  //Wyświetl informacje o błędzie w podłączeniu MQTT
  //-------------------------------------------------------------
  switch (mqttclient.state()) {
      case MQTT_CONNECTION_TIMEOUT:
        Serial.println("MQTT_CONNECTION_TIMEOUT");
        break;
      case MQTT_CONNECTION_LOST:
        Serial.println("MQTT_CONNECTION_LOST");
        break;
      case MQTT_CONNECT_FAILED:
        Serial.println("MQTT_CONNECT_FAILED");
        break;
      case MQTT_DISCONNECTED:
        Serial.println("MQTT_DISCONNECTED");
        break;
      case MQTT_CONNECTED:
        Serial.println("MQTT_CONNECTED");
        break;
      case MQTT_CONNECT_BAD_PROTOCOL:
        Serial.println("MQTT_CONNECT_BAD_PROTOCOL");
        break;
      case MQTT_CONNECT_BAD_CLIENT_ID:
        Serial.println("MQTT_CONNECT_BAD_CLIENT_ID");
        break;
      case MQTT_CONNECT_UNAVAILABLE:
        Serial.println("MQTT_CONNECT_UNAVAILABLE");
        break;
      case MQTT_CONNECT_BAD_CREDENTIALS:
        Serial.println("MQTT_CONNECT_BAD_CREDENTIALS");
        break;
      case MQTT_CONNECT_UNAUTHORIZED:
        Serial.println("MQTT_CONNECT_UNAUTHORIZED");
        break;
  }
}


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  //------------------------------------------------------------------------------------------
  //Funkcja wołana wówczas, gdy platforma MQTT chce coś do nas wysłać, u nas nie jest używana
  //------------------------------------------------------------------------------------------
  Serial.print("Wiadomość z platformy w kanale: ");
  Serial.println(topic);
}

void wifi_connect() {
  //-----------------------------------------
  //Podłącz się do Wifi
  //-----------------------------------------
  Serial.println("Podłączam się do Wifi.");
  WiFi.mode(WIFI_STA);
  WiFi.hostname((String)PROJECT);
  WiFi.begin(MySSID, MyPass);

  //Sprawdź czy się udało
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print ("Brak podłączenia do "+(String)MySSID)+" powód:";
    wifi_printError();
    delay(5000);
  };

  //Pokaż informacje o sieci
  IPAddress ip = WiFi.localIP();
  long rssi = WiFi.RSSI();
  Serial.print ("Podłączony do: "+(String)MySSID+", IP:");
  Serial.print ((String)ip[0]+"."+(String)ip[1]+"."+(String)ip[2]+"."+(String)ip[3]+" RSSI:");
  Serial.print (rssi);
  Serial.println ("dBm");
}

void mqtt_connect() {
  //-----------------------------------------
  //Podłącz się do platformy MQTT
  //-----------------------------------------

  //ustaw identyfikatory
  char MqttID[100];
  snprintf(MqttID, 100, "d:%s:%s:%s",MyMqttOrg,   MyMqttDevType, MyMqttDevName);
  char MqttServer[100];
  snprintf(MqttServer, 100, "%s.%s", MyMqttOrg,MqttUrl);

  //popdłącz
  mqttclient.setClient(ethClient);
  mqttclient.setServer(MqttServer, 1883);
  mqttclient.setCallback(mqtt_callback);
  Serial.print("Podłączam się do: ");
  Serial.print(MqttServer);
  Serial.print(" jako ");
  Serial.println(MqttID);
  while (! mqttclient.connect(MqttID, "use-token-auth", MqttToken)) {
            yield();
            delay(1000);
            Serial.print("...błąd, sprawdź parametry usługi MQTT: ");
            mqtt_printError();
  }
  Serial.println("Podłączyłem się do mqtt.");
}


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

void mqtt_sendTemp(float tempC) {
  //-------------------------------------------------
  //Wyślij temperaturę do platformy
  //-------------------------------------------------

  char packet_buff[MQTT_MAX_PACKET_SIZE];

  //dla czytelności posługujemy się biblioteką JSON, ale znacznie efektywniej byłoby używać zwykłych buforów char[]
  //użyta biblioteka PubSub Client wymaga podania bufora char[], zatem niezbędna będzie konwersja - co dodatkowo konsumuje zasoby procesora
  //lepiej zatem tak nie robić.

  StaticJsonDocument<100> jsonDoc;
  JsonObject root = jsonDoc.to<JsonObject>();
  JsonObject d = root.createNestedObject("d");
  d["temp"]=tempC;
  d["stamp"]=millis()/1000;
  serializeJson(jsonDoc, packet_buff,MQTT_MAX_PACKET_SIZE);

  if (mqttclient.publish("iot-2/evt/status/fmt/json", packet_buff)) {
        Serial.print ("mqtt_pub OK dla: ");
      } else {
        Serial.print ("mqtt_pub FAIL dla: ");
      }
      Serial.println (packet_buff);
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
  wifi_connect();
  mqtt_connect();
  sensors.begin();

}

void loop() {
  //----------------------------------------------
  // główna pętla kodu - wołana ciągle
  // ---------------------------------------------
  iteracja++;

  if (iteracja>30) {
    // wykonywane co około 15 sekund.
    iteracja=0;

    mqtt_sendTemp(getTemp()) ;
  }

  //wykonywane co ok. pół sekundy
  if (WiFi.status() != WL_CONNECTED){
    //Zgubiliśmy połączenie wifi?
    Serial.print("WIFI odłączone, powód: ");
    wifi_printError();
    wifi_connect();

  }
  if (! mqttclient.loop()) {
    //Zgubiliśmy połączenie mqtt?
    Serial.print("MQTT server odłączony, powód: ");
    mqtt_printError();
    mqtt_connect();

  }

  for (int i=0; i<50; i++) {
    //opóźnienie
    delay(10);
    yield();
  }

}
