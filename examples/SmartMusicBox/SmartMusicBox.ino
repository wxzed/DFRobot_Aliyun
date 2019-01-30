#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DFRobot_Aliyun.h"
#include "Tone32.h"
#include "music_joy.h"

#define SPEARKER_PIN  A4

/*配置WIFI名和密码*/
const char * WIFI_SSID     = "hitest";
const char * WIFI_PASSWORD = "12345678";

/*配置设备证书信息*/
String ProductKey = "a1HNPgTewi3";
String ClientId = "12345";
String DeviceName = "SmartSpeaker";
String DeviceSecret = "JVNPREpLjwo3g3A33ic9nlAD6BtD6RAH";

/*配置域名和端口号*/
String ALIYUN_SERVER = "iot-as-mqtt.cn-shanghai.aliyuncs.com";
uint16_t PORT = 1883;

uint8_t beginPlay = 0;
uint8_t playWitchOne = 0;

/*需要操作的产品标识符*/
String Identifier = "PlayWitch";

/*需要订阅的TOPIC*/
const char * subTopic = "/sys/a1HNPgTewi3/SmartSpeaker/thing/service/property/set";

DFRobot_Aliyun myAliyun;
WiFiClient espClient;
PubSubClient client(espClient);

/*播放凉凉*/
static void playLiangLiang(){
  for (int i = 0; i < sizeof(liangliangData)/sizeof(liangliangData[0]); i++) { 
    int noteDuration = liangliangDurations[i] *600;
    tone(SPEARKER_PIN, liangliangData[i],noteDuration); 
  }
  noTone(SPEARKER_PIN);
} 

/*播放天空之城*/
static void playCastleInTheSky(){
  for (int i = 0; i < sizeof(CastleInTheSkyData)/sizeof(CastleInTheSkyData[0]); i++) { 
    int noteDuration = CastleInTheSkyDurations[i] *600;
    tone(SPEARKER_PIN, CastleInTheSkyData[i],noteDuration); 
  }
  noTone(SPEARKER_PIN);
}

static void playMusic(){
  if(playWitchOne == 0){
    playLiangLiang();
  }else{
    playCastleInTheSky();
  }
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
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject((const char *)payload);
  if(!root.success()){
    Serial.println("parseObject() failed");
    return;
  }
  playWitchOne = root["params"][Identifier];
  Serial.print("playWitchOne=");
  Serial.println(playWitchOne);
  beginPlay = 1;
}

void ConnectAliyun(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    /*根据自动计算的用户名和密码连接到Alinyun的设备，不需要更改*/
    if(client.connect(myAliyun.client_id,myAliyun.username,myAliyun.password)){
      Serial.println("connected");
      client.subscribe(subTopic);
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
  if(beginPlay == 1){
    playMusic();
    beginPlay =0;
  }
  client.loop();
}