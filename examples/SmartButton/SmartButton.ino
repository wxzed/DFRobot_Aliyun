#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DFRobot_Aliyun.h"
#include "Tone32.h"

#define BUTTON_PIN  D2

/*配置WIFI名和密码*/
const char * WIFI_SSID     = "hitest";
const char * WIFI_PASSWORD = "12345678";

/*配置设备证书信息*/
String ProductKey = "a1xxJLTVJ2J";
String ClientId = "12345";
String DeviceName = "SmartButton";
String DeviceSecret = "3HuUAfD6pYZ9UDmTsevJGBYZ1hU4piWJ";

/*配置域名和端口号*/
String ALIYUN_SERVER = "iot-as-mqtt.cn-shanghai.aliyuncs.com";
uint16_t PORT = 1883;

/*需要操作的产品标识符*/
String Identifier = "ButtonStatus";

/*需要上报的TOPIC*/
const char * pubTopic = "/sys/a1xxJLTVJ2J/SmartButton/thing/event/property/post";

DFRobot_Aliyun myAliyun;
WiFiClient espClient;
PubSubClient client(espClient);

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
  pinMode(BUTTON_PIN,INPUT);
  
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
  if(digitalRead(BUTTON_PIN) == 1){
    /*上报按钮的状态*/
    client.publish(pubTopic,("{\"id\":"+ClientId+",\"params\":{\""+Identifier+"\":1},\"method\":\"thing.event.property.post\"}").c_str());
    delay(1000);
  }else{
  }
  client.loop();
}