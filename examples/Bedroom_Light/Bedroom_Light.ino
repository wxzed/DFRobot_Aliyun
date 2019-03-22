#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DFRobot_Aliyun.h"

#define BEDROOD_LIGHT  D2

/*配置WIFI名和密码*/
const char * WIFI_SSID     = "WIFI_SSID";
const char * WIFI_PASSWORD = "WIFI_PASSWORD";

/*配置设备证书信息*/
String ProductKey = "you Product Key";
String ClientId = "12345";/*自定义ID*/
String DeviceName = "you Device Name";
String DeviceSecret = "you Device Secret";

/*配置域名和端口号*/
String ALIYUN_SERVER = "iot-as-mqtt.cn-shanghai.aliyuncs.com";
uint16_t PORT = 1883;

/*需要操作的产品标识符*/
String Identifier = "you Identifier";

/*需要上报和订阅的两个TOPIC*/
const char * subTopic = "you sub Topic";//****set
const char * pubTopic = "you pub Topic";//******post

DFRobot_Aliyun myAliyun;
WiFiClient espClient;
PubSubClient client(espClient);

static void openLight(){
  digitalWrite(BEDROOD_LIGHT, HIGH);
}

static void closeLight(){
  digitalWrite(BEDROOD_LIGHT, LOW);
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
  const uint16_t LightStatus = root["params"][Identifier];
  if(LightStatus == 1){
    openLight();
  }else{
    closeLight();
  }
  String tempMseg = "{\"id\":"+ClientId+",\"params\":{\""+Identifier+"\":"+(String)LightStatus+"},\"method\":\"thing.event.property.post\"}";
  char sendMseg[tempMseg.length()];
  strcpy(sendMseg,tempMseg.c_str());
  client.publish(pubTopic,sendMseg);
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
  pinMode(BEDROOD_LIGHT,OUTPUT);
  
  /*连接WIFI*/
  connectWiFi();
  
  /*初始化Alinyun的配置，可自动计算用户名和密码*/
  myAliyun.init(ALIYUN_SERVER,ProductKey,ClientId,DeviceName,DeviceSecret);
  
  client.setServer(myAliyun.mqtt_server,PORT);
  
  /*设置回调函数，当收到订阅信息时会执行回调函数*/
  client.setCallback(callback);
  
  /*连接到Aliyun*/
  ConnectAliyun();
  
  /*开机先关灯*/
  closeLight();
  
  /*上报关灯信息*/
  client.publish(pubTopic,("{\"id\":"+ClientId+",\"params\":{\""+Identifier+"\":0},\"method\":\"thing.event.property.post\"}").c_str());
}

void loop(){
  if(!client.connected()){
    ConnectAliyun();
  }
  client.loop();
}