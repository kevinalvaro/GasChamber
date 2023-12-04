#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <EEPROM.h>
#include <TimeLib.h>

//==================== Topic AWS ================================
#define AWS_IOT_PUBLISH_TOPIC   "kedaireka/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "kedaireka/sub"
 
 //==================== Wifi Secure Connection ====================
WiFiClientSecure net = WiFiClientSecure(); 
PubSubClient client(net);

//======================= Variable setup ===========================
char Kode_Chamber[] = "TEST09"; // kode untuk chammber 1-12
char Loc_Name[] = "Tani Martani"; // lokasi geo "taman martani"
char Field_Name[] = "TEST"; // petak yang mana
char Square[] = "10"; // detail lokasi sawah
int Phase = 2; //fase tanam 1-3
int Placing = 0; // 0:not set 1:intake 2:center 3:outake
float Lat = -7.74201301036 ; //latitude 110,2083742903
float Long = 110.48098238 ; //longtitude -7,27340273
int CO2 = 0 ; //hasil sensor carbon2
float concentration = 0;
int Gas = 0 ; //hasil sensor gas
int Lux = 0 ; //hasil sensor lux
float Temp = 0 ; //hasil sensor suhu
int Humidity = 0 ; //hasil sensor humidity
float Voltage = 0 ; //hasil pembacaan sensor voltage
const char* LocalTime = " "; //update waktu
char timeOutput[30];
char DateTime[]= " "; //waktu sekarang 
// time setting
const char* ntpServer = "pool.ntp.org"; // set server
const long  gmtOffset_sec = 25200; // GMT offset untuk WIB (GMT+7)
const int   daylightOffset_sec = 0; // Tidak ada pengaturan DST di Indonesia
// set pin sensor
int CO2_Pin = 25;
int Gas_Pin = 35;
int Lux_Pin = 34;
int Voltage_Pin = 26;
#define Relay_Pin 14
int DHT11_Pin = 0;
int adc_value = 0;
float R1 = 30000.0; // kalibrasi sensor voltage
float R2 = 7500.0; // kalibrasi sensor voltage
float ref_voltage = 4.5;


#define SENSOR_DATA_PIN   (CO2_Pin)   // Sensor PWM interface
#define INTERRUPT_NUMBER   digitalPinToInterrupt(SENSOR_DATA_PIN)   // interrupt number
// Used in interrupt, calculate pulse width variable
volatile unsigned long pwmHighStartTicks=0, pwmHighEndTicks=0;
volatile unsigned long pwmHighVal=0, pwmLowVal=0;
// interrupt flag
volatile uint8_t flag=0;
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 15000;
int keep_alive = 120;


void ConnectWifi()
{
  WiFi.hostname("Kode: " + String(Kode_Chamber));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    Serial.println("Connecting to Wi-Fi");
  }
  Serial.println("Connected to the WiFi network");
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); //show ip address when connected on serial monitor.
  delay(2000);
}

void ConnectAWS()
{
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // // set keep alive
  // client.setKeepAlive(keep_alive);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}

void SetTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String GetLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Failed to obtain time";
  }
  // strftime(timeOutput, sizeof(ti meOutput), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  strftime(timeOutput, sizeof(timeOutput), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  LocalTime = timeOutput; 
  return String(timeOutput); 
} 

void publishMessage() 
{
  StaticJsonDocument<256> doc;
  doc["kode_chamber"] = Kode_Chamber ;
  doc["loc_name"] = Loc_Name;
  doc["field_name"] = Field_Name;
  doc["square"] = Square ;
  doc["phase"] = Phase;
  doc["placing"] = Placing;
  doc["lat"] = Lat;
  doc["long"] = Long;
  doc["co2"] = concentration;
  doc["gas"] = Gas ;
  doc["lux"] = Lux;
  doc["voltage"] = Voltage;
  doc["datetime"] = LocalTime;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}
void interruptChange()
{
  if (digitalRead(SENSOR_DATA_PIN)) {
    pwmHighStartTicks = micros();    // store the current micros() value
    if(2 == flag){
      flag = 4;
      if(pwmHighStartTicks > pwmHighEndTicks) {
        pwmLowVal = pwmHighStartTicks - pwmHighEndTicks;
      }
    }else{
      flag = 1;
    }
  } else {
    pwmHighEndTicks = micros();    // store the current micros() value
    if(1 == flag){
      flag = 2;
      if(pwmHighEndTicks > pwmHighStartTicks){
        pwmHighVal = pwmHighEndTicks - pwmHighStartTicks;
      }
    }
  }
}
void SensorCO2(){
  if(flag == 4){
    flag = 1;
    float pwmHighVal_ms = (pwmHighVal * 1000.0) / (pwmLowVal + pwmHighVal);
    if (pwmHighVal_ms < 0.01){
      Serial.println("Fault");
    }
    else if (pwmHighVal_ms < 80.00){
      Serial.println("preheating");
    }
    else if (pwmHighVal_ms < 998.00){
      concentration = (pwmHighVal_ms - 2) * 5;

    }
  }
}
void ReadSensor(){
  Gas=analogRead(Gas_Pin);
  Lux=analogRead(Lux_Pin);
  // adc_value = analogRead(Voltage_Pin);
  // Voltage  = (adc_value * ref_voltage) / 1024.0;
}
void PrintValue(){
  Serial.println(LocalTime);
  Serial.print("Co2 Value: ");
  Serial.println(concentration);
  Serial.print("CH4 (Gas) Value: ");
  Serial.println(Gas);
  Serial.print("Lux Value: ");
  Serial.println(Lux);
  Serial.print("Voltage Value: ");
  Serial.println(Voltage);
  Serial.println(adc_value);
  Serial.println();
}

void DelayPreheating(){
  int counter = 30;
  delay(1000);
  while(counter > 1){
    counter --;
    float pwmHighVal_ms = (pwmHighVal * 1000.0) / (pwmLowVal + pwmHighVal);
    Gas=analogRead(Gas_Pin);
    Lux=analogRead(Lux_Pin);
    Serial.print(counter);
    Serial.print(" CO2,CH4,Lux,Id Value: ");
    Serial.print(concentration = (pwmHighVal_ms - 2) * 5);
    Serial.print("|");
    Serial.print(Gas);
    Serial.print("|");
    Serial.print(Lux);
    Serial.print("|");
    Serial.print(Kode_Chamber);
    Serial.println();
    delay(1000);
  }
}
void ConnectionCheck(){
  if (!client.connected())
  {
    Serial.println("Connecting AWS IoT ");
    Serial.print(".");
    ConnectAWS();
    delay(100);
    
  }
}


void setup()
{
  pinMode(SENSOR_DATA_PIN, INPUT);
  attachInterrupt(INTERRUPT_NUMBER, interruptChange, CHANGE);
  Serial.begin(115200);
  ConnectWifi();
  ConnectAWS();
  SetTime();
  // DelayPreheating();
  startMillis = millis();
  if(WiFi.getSleep() == true) {
    WiFi.setSleep(false);
    Serial.println("WiFi Sleep is now deactivated.");
  }
  WiFi.setAutoReconnect(true);
  Serial.print(" Starting System ");
}
 
void loop()
{
  if ( WiFi.status() ==  WL_CONNECTED ){ //Conectado ao WiFi
    currentMillis = millis();
    if (currentMillis - startMillis >= period){
      ConnectAWS();
      publishMessage();
      Serial.println("========== PUBLISH MESSAGE =========");
      startMillis = currentMillis;
    }
    ConnectionCheck();

  }else{//Desconectado do WiFi, volta a tentar
    WiFi.disconnect();
    WiFi.reconnect();
    int UpCount = 0;
    int WLcount = 0;
    while (WiFi.status() != WL_CONNECTED && WLcount < 200 ){
      delay( 100 );
      Serial.printf(".");
      if (UpCount >= 60){
        UpCount = 0;
        Serial.printf("\n");
      }
      ++UpCount;
      ++WLcount;
    }
    if(WiFi.status() != WL_CONNECTED){
      Serial.println("Tidak dapat menyambung perangkat, Merestart ESP");
      ESP.restart();
    }
  }
  delay(1500);
  GetLocalTime();
  ReadSensor();
  SensorCO2();
  PrintValue();
}