/* ====================== MQTT CONFIG ======================== */
#include <WiFi.h>

const char* ssid = "Hora De Estudiar-2.4GHz";
const char* password = "colosenses3:20";

WiFiClient client;
#include <PubSubClient.h>

PubSubClient mqtt(client);

/* settings MQTT */
//const char *broker = "broker.mqttdashboard.com";
const char *broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *mqtt_user = "load01--45";
const char *mqtt_pass = "less";

const String serial_number = "202112";

const char* topicData_pub = "iotW/data";
const char* topicInit_pub = "iotW/init";
const char* topic_sub = "deviceIotW/#";

String flag = "false";

/* ====================== VARIABLES CONFIG ======================== */
const int vcc_rs232 = 5;

String lastWeight = "";
String weightPayload = "";
String weight = "";
int id = 0;
#define pin_buzzer    25
int sleepTime = 10;
unsigned long lastreceived;

/* ******************* MQTT CALLBACK  ********************** */
void mqttCallback(char* topic, byte* payload, unsigned int length) {

    String message = "";
    for (int i = 0; i < length; i++)
    {
        message += String((char)payload[i]);
    }
    Serial.println(message);
    if(String(topic) ==  "deviceIotW/available"){
      
      lastWeight = "";
      flag = message;
      id = 0;

    }

}

/* ******************* RECONNECT  ********************** */
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    mqtt.publish(topicInit_pub,"reconnect");
    // Attempt to connect

    if (mqtt.connect(mqtt_user)) {
      Serial.println("connected");
      mqtt.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc="); 
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


/* ************************* SETUP WIFI ************************** */
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(2000);
}

void beep(){
  ledcWriteTone(0, 2000);
  delay(200);
  ledcWriteTone(0, 0);
  delay(100);
  ledcWriteTone(0, 2000);
  delay(200);
  ledcWriteTone(0, 0);
  delay(100);
}

void setup() {
  // initialize both serial ports:

  Serial.begin(9600);
  Serial2.begin(9600);

  pinMode(vcc_rs232, OUTPUT);
  digitalWrite(vcc_rs232,HIGH);

  ledcSetup(2000, 0, 8);
  ledcAttachPin(pin_buzzer, 0);
  
  setup_wifi();
  
  mqtt.setServer(broker, mqtt_port);
  mqtt.setCallback(mqttCallback);

/* **************** INIT AND SET RTC ********************* */
  if (mqtt.connect(mqtt_user)) {
      //mqtt.publish(topicInit_pub,"start");
      Serial.println(topic_sub);
      mqtt.subscribe(topic_sub);
      beep();
      beep();
      delay(2000);
  }

  delay(1000);
  
}

void loop() {
  // read from port 1, send to port 0:
  if (!mqtt.connected()) {
      reconnect();
  }
  
  mqtt.loop();
  
  if (Serial2.available())
  {
    while (Serial2.available()) {
      
      char inByte = Serial2.read();
      if(inByte=='\n')
      {
        Serial.println(weightPayload);
        weight = weightPayload;
        weightPayload = "";

        if(flag == "true" && weight.toFloat() > 5 && millis() > lastreceived + sleepTime * 1000){ // weight != lastWeight && 
          id += 1;
          String payload = String(id) + "," + weight;
          
          char data[15];
          payload.toCharArray(data, payload.length() + 1);
          mqtt.publish(topicData_pub, data);
          lastreceived = millis();
          //flag == "false";
          lastWeight = weight;
          Serial.print("send data");
          Serial.println(payload);
          beep();
          //delay(5000);
        }
        
        break;
        
      }else if(inByte == 'w' || inByte == 'n' || inByte == 'k'|| inByte == 'g' || inByte =='\n'){
      }
      else{
        weightPayload += String(inByte);
      }
    }
  }
  
}
