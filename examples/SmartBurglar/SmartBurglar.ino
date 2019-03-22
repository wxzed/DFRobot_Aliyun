#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DFRobot_Aliyun.h"
#include "Tone32.h"
#include "music_joy.h"

#define SPEARKER_PIN  A4
#define MOTION_SENSOR_PIN  D2

/*配置WIFI名和密码*/
const char * WIFI_SSID     = "WIFI_SSID";
const char * WIFI_PASSWORD = "WIFI_PASSWORD";

/*配置设备证书信息*/
String ProductKey = "you Product Key";
String ClientId = "12345";
String DeviceName = "you Device Name";
String DeviceSecret = "you Device Secret";

/*配置域名和端口号*/
String ALIYUN_SERVER = "iot-as-mqtt.cn-shanghai.aliyuncs.com";
uint16_t PORT = 1883;

/*需要操作的产品标识符*/
String Identifier = "you Identifier";

/*需要上报的TOPIC*/
const char * pubTopic = "you pub Topic";//******post

DFRobot_Aliyun myAliyun;
WiFiClient espClient;
PubSubClient client(espClient);

/*播放天空之城*/
static void playCastleInTheSky(){
  for (int i = 0; i < sizeof(CastleInTheSkyData)/sizeof(CastleInTheSkyData[0]); i++) { 
    int noteDuration = CastleInTheSkyDurations[i] *600;
    tone(SPEARKER_PIN, CastleInTheSkyData[i],noteDuration); 
  }
  noTone(SPEARKER_PIN);
}

void connectWiFi(){
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Adderss: ");
  Serial.println(WiFi.localIP());
}

void callback(char * topic, byte * payload, unsigned int len){
  Serial.print("Recevice [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < len; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void ConnectAliyun(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    /*根据自动计算的用户名和密码连接到Alinyun的设备，不需要更改*/
    if(client.connect(myAliyun.client_id,myAliyun.username,myAliyun.password)){
      Serial.println("connected");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void setup(){
  Serial.begin(115200);
  pinMode(MOTION_SENSOR_PIN,INPUT);
  
  /*连接WIFI*/
  connectWiFi();
  
  /*初始化Alinyun的配置，可自动计算用户名和密码*/
  myAliyun.init(ALIYUN_SERVER,ProductKey,ClientId,DeviceName,DeviceSecret);
  
  client.setServer(myAliyun.mqtt_server,PORT);
  
  /*设置回调函数，当收到订阅信息时会执行回调函数*/
  client.setCallback(callback);
  
  /*连接到Aliyun*/
  ConnectAliyun();
}

void loop(){
  if(!client.connected()){
    ConnectAliyun();
  }
  if(digitalRead(MOTION_SENSOR_PIN) == 1){
    /*上报防盗警告*/
    client.publish(pubTopic,("{\"id\":"+ClientId+",\"params\":{\""+Identifier+"\":1},\"method\":\"thing.event.property.post\"}").c_str());
    /*播放音乐*/
    playCastleInTheSky();
  }else{
  }
  client.loop();
}