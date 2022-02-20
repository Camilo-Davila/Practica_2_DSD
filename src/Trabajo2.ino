#include <UbiConstants.h>
#include <UbidotsEsp32Mqtt.h>
#include <UbiTypes.h>

//*******Pantalla****/////
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

//*******Sensor de temperatura*****//
#include "DHT.h"
#include <string.h>

#define DHTPIN 27//definicion del pin al que se conecta el sensor 
#define DHTTYPE DHT11//definir el tipo de dht

#define LED1Pin  32
#define LED2Pin  33

DHT dht(DHTPIN,DHTTYPE); //constructor

/****************************************
 * Define Constants
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-j1TruHugbGjhYBFo9QxRdBRSET2lTo";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "UPBWiFi";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL = "Temperatura"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL2 = "Humedad"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL3 = "sw1"; //  Replace with your variable label to subscribe to
const char *VARIABLE_LABEL4 = "sw2"; //  Replace with your variable label to subscribe to
const char *VARIABLE_LABEL5 = "ConfSW1"; //  Replace with your variable label to subscribe to
const char *VARIABLE_LABEL6 = "ConfSW2"; //  Replace with your variable label to subscribe to
const char *CUSTOM_TOPIC = "/v2.0/devices/demo/+"; // This will subscribe to all the messages received by the Device labeled as "demo"

const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds

unsigned long timer;
//uint8_t analogPin = 33; // Pin used to read data from GPIO34 ADC_CH6.

int estado_sw1=0;
int estado_sw2=0, confsw1=0, confsw2=0;

char str1[]="/v2.0/devices/esp32/sw1/lv";
//strcpy(str1,);

char *buff = "this is a test string";

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  buff=topic;
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    //Serial.print((char)payload[i]);
    if ((char)payload[0] == '1')
    {
      //digitalWrite(LED, HIGH);
      if(strcmp(str1,topic)==0){
        estado_sw1=1;
        confsw1=1;
      }
      else{
        estado_sw2=1;
        confsw2=1;
      }
            
    }
    else
    {
      //digitalWrite(LED, LOW);
      //estado_sw1=0;
      if(strcmp(str1,topic)==0){
        estado_sw1=0;
        confsw1=0;
      }
      else{
        estado_sw2=0;
        confsw2=0;
      }
    }
  }
  Serial.println();
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
//Sensor init
  dht. begin();
  pinMode(LED1Pin, OUTPUT);
  pinMode(LED2Pin, OUTPUT);

//Pantalla init
  tft.init();

  //Código de pantalla de carga
  //Con esta función se rota la pantalla
  tft.setRotation(1);
  //Con esta función cambio el color del fondo de la pantalla
  tft.fillScreen(TFT_BLACK);
  //Código para imprimir un texto en la pantalla, el primer argumento es la posición x, el segundo es la posición y, el tercero es el tamaño de la fuente
  tft.setTextColor(TFT_BLUE);
  tft.drawString("TRABAJO 2",50,50,4);//X, Y, FONT
  //tft.drawString("Hola Mundo",10,120,2);//EL 2 es la fuente
  tft.setTextColor(TFT_DARKGREEN);
  tft.drawString("Connecting to server...",48,70,2);//X, Y, FONT
  
  // ubidots.setDebug(true);  // uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL3); // Insert the dataSource and Variable's Labels
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL4); // Insert the dataSource and Variable's Labels
  //ubidots.subscribe(CUSTOM_TOPIC); // Insert the topic to subscribe to

  timer = millis();
}

//int i=0;

void loop()
{
  // put your main code here, to run repeatedly:

  delay (1000); //reatrdo de 2seg
  float h= dht.readHumidity(); //lee la humedad en fnc read y la almacena en h 
  float t= dht.readTemperature(); //lee la temperatura en fnc read y la almacena en t 
  if (isnan(h) || isnan(t)){
    Serial.println(F("Failed to read from DHT sensor!")); //isnan nos devuelve un 1 en caso de ue exista un fallo o un error en la lectura de la vble
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Failed to read from DHT sensor",20,50,2);//X, Y, FONT
    return; 
  }
  
  //tft.drawString(String(i), 30, 50, 7);
  //i++;
  //delay(100);
  
  if (!ubidots.connected())
  {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL3); // Insert the device and variable's Labels, respectively
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL4); // Insert the device and variable's Labels, respectively
  }
  if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    //float value = analogRead(analogPin);
    //Serial.print(value);
    //Serial.print();
    //ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL); // Insert the dataSource and Variable's Labels
    
    ubidots.add(VARIABLE_LABEL, t); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL2, h); 
    ubidots.publish(DEVICE_LABEL);
    ubidots.add(VARIABLE_LABEL5, confsw1); // Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL6, confsw2); 
    timer = millis();
  }
  ubidots.loop();

  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_CYAN);
  tft.drawString("Humedad (%):",5,0,2);//X, Y, FONT
  tft.drawString(String(h), 90, 15, 7);

  tft.setTextColor(TFT_RED);
  tft.drawString("Temperatura(°C):",5,65,2);//X, Y, FONT
  tft.drawString(String(t), 90, 80, 7);

//Switch 1
  if (estado_sw1 == 1)
  {
    digitalWrite(LED1Pin, HIGH);
    tft.fillCircle(20,110,10,TFT_YELLOW);//(X,Y,radio,color)
  }
  else
  {
    digitalWrite(LED1Pin, LOW);
    tft.fillCircle(20,110,10,TFT_DARKGREY);//(X,Y,radio,color)
  }

//Switch 2
  if (estado_sw2 == 1)
  {
    digitalWrite(LED2Pin, HIGH);
    tft.fillCircle(50,110,10,TFT_GREEN);//(X,Y,radio,color)
  }
  else
  {
    digitalWrite(LED2Pin, LOW);
    tft.fillCircle(50,110,10,TFT_DARKGREY);//(X,Y,radio,color)
  }
  //tft.fillCircle(50,110,10,TFT_DARKGREY);//(X,Y,radio,color)
  //fillCircle(50,50, 10, TFT_BLACK);
  
  
}
