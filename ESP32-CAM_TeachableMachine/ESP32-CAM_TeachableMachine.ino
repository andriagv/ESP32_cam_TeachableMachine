/*
Author : Andria Gvaramia  28-07-2025

Updated for Web Visualization Enhancement - 2024
- Static IP configuration: 172.20.10.4
- Improved modern UI with gradient backgrounds
- Enhanced user experience with emojis and better styling
- Better responsive design for mobile and desktop
- Improved camera controls and settings panel

http://172.20.10.4             
http://172.20.10.4:81/stream         <img src="http://172.20.10.4:81/stream">
http://172.20.10.4/capture              <img src="http://172.20.10.4/capture">
http://172.20.10.4/status      



 http://192.168.xxx.xxx/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://192.168.xxx.xxx/control?ip                      
http://192.168.xxx.xxx/control?mac                     
http://192.168.xxx.xxx/control?restart                
http://192.168.xxx.xxx/control?digitalwrite=pin;value  
http://192.168.xxx.xxx/control?analogwrite=pin;value  
http://192.168.xxx.xxx/control?digitalread=pin         
http://192.168.xxx.xxx/control?analogread=pin          
http://192.168.xxx.xxx/control?touchread=pin           
http://192.168.xxx.xxx/control?resetwifi=ssid;password   
http://192.168.xxx.xxx/control?flash=value             

 http://192.168.xxx.xxx/control?var=***&val=***
http://192.168.xxx.xxx/control?var=framesize&val=value    // value = 10->UXGA(1600x1200), 9->SXGA(1280x1024), 8->XGA(1024x768) ,7->SVGA(800x600), 6->VGA(640x480), 5 selected=selected->CIF(400x296), 4->QVGA(320x240), 3->HQVGA(240x176), 0->QQVGA(160x120)
http://192.168.xxx.xxx/control?var=quality&val=value      // value = 10 ~ 63
http://192.168.xxx.xxx/control?var=brightness&val=value   // value = -2 ~ 2
http://192.168.xxx.xxx/control?var=contrast&val=value     // value = -2 ~ 2
http://192.168.xxx.xxx/control?var=hmirror&val=value      // value = 0 or 1 
http://192.168.xxx.xxx/control?var=vflip&val=value        // value = 0 or 1 
http://192.168.xxx.xxx/control?var=flash&val=value        // value = 0 ~ 255   
      
IP：
IP：http://192.168.4.1/?ip
：http://192.168.4.1/?resetwifi=ssid;password
*/


const char* ssid     = "edurom";   //your network SSID
const char* password = "123456788";   //your network password

const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";         

IPAddress local_IP(172, 20, 10, 4);
IPAddress gateway(172, 20, 10, 1);
IPAddress subnet(255, 255, 255, 0);

#include <WiFi.h>
#include <esp32-hal-ledc.h>      
#include "soc/soc.h"             
#include "soc/rtc_cntl_reg.h"    


#include "esp_camera.h"          
#include "esp_http_server.h"     
#include "img_converters.h"      

String Feedback="";  


String Command="";
String cmd="";
String P1="";
String P2="";
String P3="";
String P4="";
String P5="";
String P6="";
String P7="";
String P8="";
String P9="";


byte ReceiveState=0;
byte cmdState=1;
byte strState=1;
byte questionstate=0;
byte equalstate=0;
byte semicolonstate=0;

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

//ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
    
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  Serial.println();

  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  //
  // WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
  //            Ensure ESP32 Wrover Module or other board with PSRAM is selected
  //            Partial images will be transmitted if image exceeds buffer size
  //   
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){  // PSRAM (Pseudo SRAM)
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }


  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);     // UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)

  //s->set_vflip(s, 1);  
  //s->set_hmirror(s, 1);  
  
  //(GPIO4) - Flash LED (GPIO4)
  pinMode(4, OUTPUT);
  analogWrite(4, 0);
  
  WiFi.mode(WIFI_AP_STA);  // WiFi.mode(WIFI_AP); WiFi.mode(WIFI_STA);

  //ClientIP - Static IP Configuration for 172.20.10.4
  WiFi.config(local_IP, gateway, subnet);

  for (int i=0;i<2;i++) {
    WiFi.begin(ssid, password);    
  
    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+10000) < millis()) break;    
    } 
  
    if (WiFi.status() == WL_CONNECTED) {    // Connected successfully
      WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);           
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
  
      for (int i=0;i<5;i++) {   
        analogWrite(4, 10);
        delay(200);
        analogWrite(4, 0);
        delay(200);    
      }
      break;
    }
  } 

  if (WiFi.status() != WL_CONNECTED) {    
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);         

    for (int i=0;i<2;i++) {    
      analogWrite(4, 10);
      delay(1000);
      analogWrite(4, 0);
      delay(1000);    
    }
  } 
  
  
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());  
  Serial.println("");
  
  startCameraServer(); 

  // - Set flash LED to low level
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 
  
  Serial.println("ESP32-CAM Web Interface Ready!");
  Serial.println("Access the interface at: http://172.20.10.4");
}

void loop() {

}

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

//影像截圖
static esp_err_t capture_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t fb_len = 0;
    if(fb->format == PIXFORMAT_JPEG){
        fb_len = fb->len;
        res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    } else {
        jpg_chunking_t jchunk = {req, 0};
        res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
        httpd_resp_send_chunk(req, NULL, 0);
        fb_len = jchunk.len;
    }
    esp_camera_fb_return(fb);
    return res;
}


static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
          if(fb->format != PIXFORMAT_JPEG){
              bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
              esp_camera_fb_return(fb);
              fb = NULL;
              if(!jpeg_converted){
                  Serial.println("JPEG compression failed");
                  res = ESP_FAIL;
              }
          } else {
              _jpg_buf_len = fb->len;
              _jpg_buf = fb->buf;
          }
        }

        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }                
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
            break;
        }
    }

    return res;
}

// - Command parameter control
static esp_err_t cmd_handler(httpd_req_t *req){
    char*  buf;    // - Access URL parameters
    size_t buf_len;
    char variable[128] = {0,};  // - Access var parameter value
    char value[128] = {0,};     // - Access val parameter value
    String myCmd = "";

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
          if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
            httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
          } 
          else {
            myCmd = String(buf);   //var, val，
          }
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
    ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;     
    if (myCmd.length()>0) {
      myCmd = "?"+myCmd;  
      for (int i=0;i<myCmd.length();i++) {
        getCommand(char(myCmd.charAt(i)));  
      }
    }

    if (cmd.length()>0) {
      Serial.println("");
      //Serial.println("Command: "+Command);
      Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
      Serial.println(""); 

       http://172.20.10.4/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
      // Custom command block - Control commands for ESP32-CAM
      if (cmd=="your cmd") {
        // You can do anything
        // Feedback="<font color=\"red\">Hello World</font>";   //可為一般文字或HTML語法
      }
      else if (cmd=="ip") {  // STAIP - Query APIP, STAIP
        Feedback="AP IP: "+WiFi.softAPIP().toString();    
        Feedback+="<br>";
        Feedback+="STA IP: "+WiFi.localIP().toString();
      }  
      else if (cmd=="mac") {  // Query MAC address
        Feedback="STA MAC: "+WiFi.macAddress();
      }  
      else if (cmd=="restart") {  //重ESP32-CAM - Restart ESP32-CAM
        ESP.restart();
      }  
      else if (cmd=="digitalwrite") {  //Digital output control
        pinMode(P1.toInt(), OUTPUT);
        digitalWrite(P1.toInt(), P2.toInt());
      }   
      else if (cmd=="digitalread") {  //Digital read
        Feedback=String(digitalRead(P1.toInt()));
      }
      else if (cmd=="analogwrite") {   //Analog output control
        if (P1=="4") {
          pinMode(4, OUTPUT);
          analogWrite(4, P2.toInt());     
        }
        else {
          pinMode(P1.toInt(), OUTPUT);
          analogWrite(P1.toInt(), P2.toInt());
        }
      }       
      else if (cmd=="analogread") {  //Analog read
        Feedback=String(analogRead(P1.toInt()));
      }
      else if (cmd=="touchread") {  //Touch read
        Feedback=String(touchRead(P1.toInt()));
      }
      else if (cmd=="resetwifi") {  //Reset WiFi connection
        for (int i=0;i<2;i++) {
          WiFi.begin(P1.c_str(), P2.c_str());
          Serial.print("Connecting to ");
          Serial.println(P1);
          long int StartTime=millis();
          while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              if ((StartTime+5000) < millis()) break;
          } 
          Serial.println("");
          Serial.println("STAIP: "+WiFi.localIP().toString());
          Feedback="STAIP: "+WiFi.localIP().toString();
  
          if (WiFi.status() == WL_CONNECTED) {
            WiFi.softAP((WiFi.localIP().toString()+"_"+P1).c_str(), P2.c_str());
            for (int i=0;i<2;i++) {    
              analogWrite(4, 10);
              delay(300);
              analogWrite(4, 0);
              delay(300);    
            }
            break;
          }
        }
      }   
      else if (cmd=="flash") { 
        pinMode(4, OUTPUT);
        int val = P1.toInt();
        analogWrite(4, val);  
      }
      else if (cmd=="serial") {
        if (P1!=""&P1!="stop") Serial.println(P1);
        if (P2!=""&P2!="stop") Serial.println(P2);
        Serial.println();
      }       
      else {
        Feedback="Command is not defined";
      }

      if (Feedback=="") Feedback=Command; 
    
      const char *resp = Feedback.c_str();
      httpd_resp_set_type(req, "text/html");  
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");  
      return httpd_resp_send(req, resp, strlen(resp));
    } 
    else {
      
      // Official command block - Camera control parameters
      int val = atoi(value);
      sensor_t * s = esp_camera_sensor_get();
      int res = 0;

      if(!strcmp(variable, "framesize")) {  //Set image resolution
        if(s->pixformat == PIXFORMAT_JPEG) 
          res = s->set_framesize(s, (framesize_t)val);
      }
      else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);  //Set JPEG quality
      else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);  //Set contrast
      else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);  //Set brightness
      else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, val);  //Set horizontal mirror
      else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, val);  //Set vertical flip
      else if(!strcmp(variable, "flash")) {  //Control flash
        pinMode(4, OUTPUT);
        analogWrite(4, val);
      } 
      else {
          res = -1;
      }
  
      if(res){
          return httpd_resp_send_500(req);
      }

      if (buf) {
        Feedback = String(buf);
        const char *resp = Feedback.c_str();
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        return httpd_resp_send(req, resp, strlen(resp)); 
      }
      else {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        return httpd_resp_send(req, NULL, 0);
      }
    }
}

//Display video parameters status (JSON format for initial settings)
static esp_err_t status_handler(httpd_req_t *req){
    static char json_response[1024];

    sensor_t * s = esp_camera_sensor_get();
    char * p = json_response;
    *p++ = '{';
    p+=sprintf(p, "\"flash\":%d,", 0);
    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror); 
    p+=sprintf(p, "\"vflip\":%u", s->status.vflip);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

//Custom web interface with enhanced visualization
// Features: Modern gradient design, responsive layout, improved UX
static const char PROGMEM INDEX_HTML[] = R"rawliteral(<!doctype html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <meta http-equiv="Access-Control-Allow-Headers" content="Origin, X-Requested-With, Content-Type, Accept">
        <meta http-equiv="Access-Control-Allow-Methods" content="GET,POST,PUT,DELETE,OPTIONS">
        <meta http-equiv="Access-Control-Allow-Origin" content="*">
        <title>ESP32-CAM Web Interface</title>
        <style>
          * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
          }
          
          body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(to right, #add8e6, #40e0d0, #3cb371, #add8e6);
            background-size: 400% 400%;
            animation: gradientShift 15s ease infinite;
            color: #ffffff;
            font-size: 16px;
            margin: 0;
            padding: 10px;
            min-height: 100vh;
            overflow-x: hidden;
            width: 100%;
            max-width: 100vw;
          }
          
          @keyframes gradientShift {
            0% { background-position: 0% 50%; }
            50% { background-position: 100% 50%; }
            100% { background-position: 0% 50%; }
          }
          
          h2 {
            font-size: 28px;
            margin-bottom: 15px;
            text-align: center;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
            color: #ffffff;
          }
          
          .main-container {
            max-width: 100%;
            width: 100%;
            margin: 0 auto;
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 15px;
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
            overflow: hidden;
            box-sizing: border-box;
          }
          
          .header {
            text-align: center;
            margin-bottom: 20px;
          }
          
          .header p {
            margin-bottom: 15px;
            font-size: 14px;
            opacity: 0.9;
          }
          
          .controls-section {
            display: flex;
            flex-direction: column;
            gap: 15px;
            margin-bottom: 20px;
          }
          
          .button-row {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            justify-content: center;
          }
          
          button {
            display: inline-block;
            padding: 12px 20px;
            border: 0;
            cursor: pointer;
            color: #fff;
            background: linear-gradient(45deg, #ff6b6b, #ee5a24);
            border-radius: 8px;
            font-size: 14px;
            outline: 0;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
            min-width: 120px;
            text-align: center;
          }
          
          button:hover {
            background: linear-gradient(45deg, #ee5a24, #ff6b6b);
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.3);
          }
          
          button:active {
            background: linear-gradient(45deg, #d63031, #e17055);
          }
          
          button.disabled {
            cursor: default;
            background: #a0a0a0;
          }
          
          .settings-panel {
            background: rgba(54,54,54,0.8);
            padding: 20px;
            border-radius: 10px;
            backdrop-filter: blur(10px);
            margin-bottom: 20px;
            width: 100%;
            box-sizing: border-box;
          }
          
          .settings-toggle {
            cursor: pointer;
            display: block;
            background: rgba(255,255,255,0.2);
            padding: 12px;
            border-radius: 8px;
            text-align: center;
            transition: all 0.3s ease;
            margin-bottom: 15px;
            font-weight: bold;
          }
          
          .settings-toggle:hover {
            background: rgba(255,255,255,0.3);
          }
          
          .settings-toggle:checked + .settings-content {
            display: none;
          }
          
          .settings-content {
            display: block;
          }
          
          .input-group {
            display: flex;
            flex-direction: column;
            margin-bottom: 15px;
            gap: 8px;
            width: 100%;
            min-width: 0;
          }
          
          .input-group label {
            font-weight: bold;
            font-size: 14px;
            color: #ffffff;
            margin-bottom: 5px;
            text-shadow: 1px 1px 2px rgba(0,0,0,0.5);
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
          }
          
          .input-group input,
          .input-group select {
            padding: 10px;
            border-radius: 8px;
            border: 1px solid rgba(255,255,255,0.3);
            background: rgba(255,255,255,0.1);
            color: #ffffff;
            font-size: 14px;
            width: 100%;
          }
          
          .input-group input:focus,
          .input-group select:focus {
            outline: none;
            border-color: #40e0d0;
            box-shadow: 0 0 0 2px rgba(64, 224, 208, 0.3);
          }
          
          .range-container {
            display: flex;
            align-items: center;
            gap: 10px;
            flex-wrap: wrap;
            width: 100%;
            min-width: 0;
          }
          
          .range-min,
          .range-max {
            font-weight: bold;
            font-size: 12px;
            color: #ffffff;
            min-width: 30px;
            text-align: center;
            text-shadow: 1px 1px 2px rgba(0,0,0,0.5);
            flex-shrink: 0;
          }
          
          input[type=range] {
            -webkit-appearance: none;
            flex: 1;
            min-width: 150px;
            height: 8px;
            background: rgba(255,255,255,0.2);
            cursor: pointer;
            border-radius: 4px;
            outline: none;
            margin: 0;
            padding: 0;
          }
          
          input[type=range]::-webkit-slider-thumb {
            -webkit-appearance: none;
            height: 20px;
            width: 20px;
            border-radius: 50%;
            background: linear-gradient(45deg, #ff6b6b, #ee5a24);
            cursor: pointer;
            box-shadow: 0 2px 8px rgba(0,0,0,0.3);
          }
          
          input[type=range]::-moz-range-thumb {
            height: 20px;
            width: 20px;
            border-radius: 50%;
            background: linear-gradient(45deg, #ff6b6b, #ee5a24);
            cursor: pointer;
            border: none;
            box-shadow: 0 2px 8px rgba(0,0,0,0.3);
          }
          
          .switch {
            display: flex;
            align-items: center;
            gap: 10px;
          }
          
          .switch input {
            outline: 0;
            opacity: 0;
            width: 0;
            height: 0;
          }
          
          .slider {
            width: 50px;
            height: 25px;
            border-radius: 25px;
            cursor: pointer;
            background-color: rgba(255,255,255,0.3);
            transition: .4s;
            position: relative;
            display: inline-block;
          }
          
          .slider:before {
            position: absolute;
            content: "";
            border-radius: 50%;
            height: 19px;
            width: 19px;
            left: 3px;
            top: 3px;
            background-color: #fff;
            box-shadow: 0 2px 8px rgba(0,0,0,0.3);
            transition: .4s;
          }
          
          input:checked + .slider {
            background: linear-gradient(45deg, #ff6b6b, #ee5a24);
          }
          
          input:checked + .slider:before {
            transform: translateX(25px);
          }
          
          .image-container {
            position: relative;
            width: 100%;
            max-width: 100%;
            border-radius: 10px;
            overflow: hidden;
            margin-bottom: 20px;
            background: rgba(0,0,0,0.1);
            padding: 15px;
            text-align: center;
            min-height: 300px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
          }
          
          .image-container img {
            width: 100%;
            max-width: 640px;
            height: auto;
            display: block;
            border-radius: 10px;
            margin: 0 auto;
            box-shadow: 0 4px 15px rgba(0,0,0,0.3);
            object-fit: contain;
          }
          
          .close {
            position: absolute;
            right: 10px;
            top: 10px;
            background: linear-gradient(45deg, #ff6b6b, #ee5a24);
            width: 30px;
            height: 30px;
            border-radius: 50%;
            color: #fff;
            text-align: center;
            line-height: 30px;
            cursor: pointer;
            font-size: 18px;
            font-weight: bold;
            box-shadow: 0 4px 15px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
          }
          
          .close:hover {
            transform: scale(1.1);
          }
          
          .hidden {
            /* Removed to keep elements visible */
          }
          
          #result {
            background: rgba(255,255,255,0.1);
            padding: 15px;
            border-radius: 10px;
            margin-top: 20px;
            backdrop-filter: blur(10px);
            border-left: 4px solid #ff6b6b;
            font-size: 14px;
            line-height: 1.5;
          }
          
          canvas {
            display: block;
            border-radius: 10px;
            margin: 10px auto;
            max-width: 100%;
            height: auto;
            box-shadow: 0 4px 15px rgba(0,0,0,0.3);
            object-fit: contain;
          }
          
          @media (max-width: 768px) {
            body {
              padding: 5px;
              font-size: 14px;
              overflow-x: hidden;
            }
            
            .main-container {
              padding: 10px;
              width: 100%;
              max-width: 100vw;
            }
            
            .button-row {
              flex-direction: column;
              align-items: center;
              gap: 8px;
            }
            
            button {
              width: 100%;
              max-width: 200px;
              font-size: 13px;
              padding: 10px 15px;
            }
            
            .input-group {
              margin-bottom: 12px;
              width: 100%;
            }
            
            .range-container {
              flex-direction: column;
              align-items: stretch;
              width: 100%;
            }
            
            input[type=range] {
              min-width: 100%;
              width: 100%;
            }
            
            .settings-panel {
              width: 100%;
              padding: 15px;
            }
            
            .image-container {
              width: 100%;
              padding: 10px;
              margin-bottom: 15px;
              min-height: 250px;
            }
            
            .image-container img {
              max-width: 100%;
              height: auto;
            }
            
            canvas {
              max-width: 100%;
              height: auto;
            }
          }
          
          @media (min-width: 769px) {
            .main-container {
              max-width: 1200px;
              width: 100%;
            }
            
            .controls-section {
              flex-direction: row;
              align-items: flex-start;
              gap: 20px;
              width: 100%;
            }
            
            .settings-panel {
              flex: 0 0 350px;
              max-width: 350px;
            }
            
            .image-container {
              flex: 1;
              max-width: calc(100% - 370px);
              padding: 15px;
            }
            
            .image-container img {
              max-width: 100%;
              height: auto;
            }
            
            canvas {
              max-width: 100%;
              height: auto;
            }
          }
        </style>
        <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
        <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow/tfjs@1.3.1/dist/tf.min.js"></script>
        <script src="https:\/\/cdn.jsdelivr.net/npm/@teachablemachine/image@0.8/dist/teachablemachine-image.min.js"></script>  
        <script src="https:\/\/cdn.jsdelivr.net/npm/@teachablemachine/pose@0.8/dist/teachablemachine-pose.min.js"></script>       
    </head>
    <body>
        <div class="main-container">
            <div class="header">
                <h2>📷 ESP32-CAM Web Interface</h2>
                <p>Access your camera at: http://172.20.10.4</p>
            </div>
            
            <div class="controls-section">
                <div class="button-row">
                    <button id="restart" onclick="try{fetch(document.location.origin+'/control?restart');}catch(e){}">🔄 Restart</button>
                    <button id="get-still">📷 Get Still</button>
                    <button id="toggle-stream">🎥 Start Stream</button>
                </div>
            </div>
            
            <div class="image-container" id="stream-container" style="display: block;">
                <div class="close" id="close-stream">×</div>
                <img id="stream" src="data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMzIwIiBoZWlnaHQ9IjI0MCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cmVjdCB3aWR0aD0iMTAwJSIgaGVpZ2h0PSIxMDAlIiBmaWxsPSIjNjY2NjY2Ii8+PHRleHQgeD0iNTAlIiB5PSI1MCUiIGZvbnQtZmFtaWx5PSJBcmlhbCwgc2Fucy1zZXJpZiIgZm9udC1zaXplPSIxNCIgZmlsbD0iI2ZmZmZmZiIgdGV4dC1hbmNob3I9Im1pZGRsZSIgZHk9Ii4zZW0iPkNhbWVyYSBQcmV2aWV3PC90ZXh0Pjwvc3ZnPg==" crossorigin="anonymous" style="display: block;">
                <canvas id="canvas" width="320" height="240" style="display: block;"></canvas>
            </div>
            
            <div class="settings-panel">
                <input type="checkbox" id="nav-toggle-cb" class="settings-toggle">
                <label for="nav-toggle-cb" class="settings-toggle">⚙️ Settings Panel</label>
                <div class="settings-content" id="menu">
                    <div class="input-group">
                        <label for="kind">Model Type</label>
                        <select id="kind">
                            <option value="image">Image Recognition</option>
                            <option value="pose">Pose Detection</option>
                        </select>
                    </div>
                    
                    <div class="input-group">
                        <label for="modelPath">Model URL</label>
                        <input type="text" id="modelPath" value="" placeholder="Enter model URL">
                    </div>
                    
                    <div class="input-group">
                        <button type="button" id="btnModel" onclick="LoadModel();">🚀 Start Recognition</button>
                    </div>
                    
                    <div class="input-group" id="flash-group">
                        <label for="flash">💡 Flash</label>
                        <div class="range-container">
                            <div class="range-min">0</div>
                            <input type="range" id="flash" min="0" max="255" value="0" class="default-action">
                            <div class="range-max">255</div>
                        </div>
                    </div>
                    
                    <div class="input-group" id="framesize-group">
                        <label for="framesize">📐 Resolution</label>
                        <select id="framesize" class="default-action">
                            <option value="10">UXGA(1600x1200)</option>
                            <option value="9">SXGA(1280x1024)</option>
                            <option value="8">XGA(1024x768)</option>
                            <option value="7">SVGA(800x600)</option>
                            <option value="6">VGA(640x480)</option>
                            <option value="5" selected="selected">CIF(400x296)</option>
                            <option value="4">QVGA(320x240)</option>
                            <option value="3">HQVGA(240x176)</option>
                            <option value="0">QQVGA(160x120)</option>
                        </select>
                    </div>
                    
                    <div class="input-group" id="quality-group">
                        <label for="quality">🎨 Quality</label>
                        <div class="range-container">
                            <div class="range-min">10</div>
                            <input type="range" id="quality" min="10" max="63" value="10" class="default-action">
                            <div class="range-max">63</div>
                        </div>
                    </div>
                    
                    <div class="input-group" id="brightness-group">
                        <label for="brightness">☀️ Brightness</label>
                        <div class="range-container">
                            <div class="range-min">-2</div>
                            <input type="range" id="brightness" min="-2" max="2" value="0" class="default-action">
                            <div class="range-max">2</div>
                        </div>
                    </div>
                    
                    <div class="input-group" id="contrast-group">
                        <label for="contrast">🔍 Contrast</label>
                        <div class="range-container">
                            <div class="range-min">-2</div>
                            <input type="range" id="contrast" min="-2" max="2" value="0" class="default-action">
                            <div class="range-max">2</div>
                        </div>
                    </div>
                    
                    <div class="input-group" id="hmirror-group">
                        <label for="hmirror">🔄 H-Mirror</label>
                        <div class="switch">
                            <input id="hmirror" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="hmirror"></label>
                        </div>
                    </div>
                    
                    <div class="input-group" id="vflip-group">
                        <label for="vflip">🔄 V-Flip</label>
                        <div class="switch">
                            <input id="vflip" type="checkbox" class="default-action" checked="checked">
                            <label class="slider" for="vflip"></label>
                        </div>
                    </div>
                </div>
            </div>
            
            <div id="result" style="color:#ff6b6b;font-weight:bold;"></div>
        </div>
        
        <script>
          document.addEventListener('DOMContentLoaded', function (event) {
            var baseHost = document.location.origin
            var streamUrl = baseHost + ':81'
            const hide = el => {
              // Keep elements visible but hide if needed
              el.style.display = 'none';
            }
            const show = el => {
              // Keep elements visible
              el.style.display = 'block';
            }
            const disable = el => {
              el.classList.add('disabled')
              el.disabled = true
            }
            const enable = el => {
              el.classList.remove('disabled')
              el.disabled = false
            }
            const updateValue = (el, value, updateRemote) => {
              updateRemote = updateRemote == null ? true : updateRemote
              let initialValue
              if (el.type === 'checkbox') {
                initialValue = el.checked
                value = !!value
                el.checked = value
              } else {
                initialValue = el.value
                el.value = value
              }
              if (updateRemote && initialValue !== value) {
                updateConfig(el);
              }
            }
            function updateConfig (el) {
              let value
              switch (el.type) {
                case 'checkbox':
                  value = el.checked ? 1 : 0
                  break
                case 'range':
                case 'select-one':
                  value = el.value
                  break
                case 'button':
                case 'submit':
                  value = '1'
                  break
                default:
                  return
              }
              const query = `${baseHost}/control?var=${el.id}&val=${value}`
              fetch(query)
                .then(response => {
                  console.log(`request to ${query} finished, status: ${response.status}`)
                })
            }
            document
              .querySelectorAll('.close')
              .forEach(el => {
                el.onclick = () => {
                  hide(el.parentNode)
                }
              })
            // read initial values
            fetch(`${baseHost}/status`)
              .then(function (response) {
                return response.json()
              })
              .then(function (state) {
                document
                  .querySelectorAll('.default-action')
                  .forEach(el => {
                    updateValue(el, state[el.id], false)
                  })
              })
            const view = document.getElementById('stream')
            const viewContainer = document.getElementById('stream-container')
            const stillButton = document.getElementById('get-still')
            const streamButton = document.getElementById('toggle-stream')
            const closeButton = document.getElementById('close-stream')
            const stopStream = () => {
              //window.stop();
              view.src="";
              streamButton.innerHTML = '🎥 Start Stream'
            }
            const startStream = () => {
              view.src = `${streamUrl}/stream`
              show(viewContainer)
              streamButton.innerHTML = '⏹️ Stop Stream'
            }
            // Attach actions to buttons
            stillButton.onclick = () => {
              stopStream()
              try{
                view.src = `${baseHost}/capture?_cb=${Date.now()}`
              }
              catch(e) {
                view.src = `${baseHost}/capture?_cb=${Date.now()}`  
              }
              show(viewContainer)
            }
            closeButton.onclick = () => {
              stopStream()
              hide(viewContainer)
            }
            streamButton.onclick = () => {
              const streamEnabled = streamButton.innerHTML === '⏹️ Stop Stream'
              if (streamEnabled) {
                stopStream()
              } else {
                startStream()
              }
            }
            // Attach default on change action
            document
              .querySelectorAll('.default-action')
              .forEach(el => {
                el.onchange = () => updateConfig(el)
              })
          })
        </script>
          
        <script>
        var getStill = document.getElementById('get-still');
        var ShowImage = document.getElementById('stream');
        var canvas = document.getElementById("canvas");
        var context = canvas.getContext("2d");  
        var modelPath = document.getElementById('modelPath');
        var result = document.getElementById('result');
        var kind = document.getElementById('kind');        
        let Model;
        
        async function LoadModel() {
          if (modelPath.value=="") {
            result.innerHTML = "⚠️ Please input model path.";
            return;
          }
    
          result.innerHTML = "⏳ Please wait for loading model...";
          
          const URL = modelPath.value;
          const modelURL = URL + "model.json";
          const metadataURL = URL + "metadata.json";
          if (kind.value=="image") {
            Model = await tmImage.load(modelURL, metadataURL);
          }
          else if (kind.value=="pose") {
            Model = await tmPose.load(modelURL, metadataURL);
          }
          maxPredictions = Model.getTotalClasses();
          result.innerHTML = "✅ Model loaded successfully!";
    
          getStill.style.display = "block";
          getStill.click();
        }
    
        async function predict() {
          var data = "";
          var maxClassName = "";
          var maxProbability = "";
    
          canvas.setAttribute("width", ShowImage.width);
          canvas.setAttribute("height", ShowImage.height);
          context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height); 
           
          if (kind.value=="image")
            var prediction = await Model.predict(canvas);
          else if (kind.value=="pose") {
            var { pose, posenetOutput } = await Model.estimatePose(canvas);
            var prediction = await Model.predict(posenetOutput);
          }
    
          if (maxPredictions>0) {
            for (let i = 0; i < maxPredictions; i++) {
              if (i==0) {
              maxClassName = prediction[i].className;
              maxProbability = prediction[i].probability;
              }
              else {
              if (prediction[i].probability>maxProbability) {
                maxClassName = prediction[i].className;
                maxProbability = prediction[i].probability;
              }
              }
              data += "🎯 " + prediction[i].className + ": " + (prediction[i].probability * 100).toFixed(1) + "%<br>";
            }
            result.innerHTML = data;
            result.innerHTML += "<br><strong>🏆 Result: " + maxClassName + " (" + (maxProbability * 100).toFixed(1) + "%)</strong>"; 

            $.ajax({url: document.location.origin+'/control?serial='+maxClassName+";"+maxProbability+';stop', async: false});
          }
          else
            result.innerHTML = "❌ Unrecognizable";
            
          getStill.click();
        }
    
        ShowImage.onload = function (event) {
          if (Model) {
          try { 
            document.createEvent("TouchEvent");
            setTimeout(function(){predict();},250);
          }
          catch(e) { 
            predict();
          } 
          }
        }    
        </script>
    </body>
</html>)rawliteral";

//http://172.20.10.4 - Main web interface handler
static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

//Custom URL path functions
void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();   

  //http://172.20.10.4/ - Main web interface
  httpd_uri_t index_uri = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = index_handler,
      .user_ctx  = NULL
  };

  //http://172.20.10.4/status - Camera status API
  httpd_uri_t status_uri = {
      .uri       = "/status",
      .method    = HTTP_GET,
      .handler   = status_handler,
      .user_ctx  = NULL
  };

  //http://172.20.10.4/control - Camera control API
  httpd_uri_t cmd_uri = {
      .uri       = "/control",
      .method    = HTTP_GET,
      .handler   = cmd_handler,
      .user_ctx  = NULL
  }; 

  //http://172.20.10.4/capture - Single image capture
  httpd_uri_t capture_uri = {
      .uri       = "/capture",
      .method    = HTTP_GET,
      .handler   = capture_handler,
      .user_ctx  = NULL
  };

  //http://172.20.10.4:81/stream - Video stream
  httpd_uri_t stream_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
  };
  
  Serial.printf("Starting web server on port: '%d'\n", config.server_port);  //Server Port
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
      // Register custom URL path handlers
      httpd_register_uri_handler(camera_httpd, &index_uri);
      httpd_register_uri_handler(camera_httpd, &cmd_uri);
      httpd_register_uri_handler(camera_httpd, &status_uri);
      httpd_register_uri_handler(camera_httpd, &capture_uri);
  }
  
  config.server_port += 1;  //Stream Port - Video streaming on port 81
  config.ctrl_port += 1;    //UDP Port
  Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
      httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

//Parse custom command parameters into variables
void getCommand(char c)
{
  if (c=='?') ReceiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
  
  if (ReceiveState==1)
  {
    Command=Command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) P2=P2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) P3=P3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) P4=P4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) P5=P5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) P6=P6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) P7=P7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) P8=P8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) P9=P9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}

/*
ESP32-CAM Web Interface Enhancement Summary:
- Static IP Configuration: 172.20.10.4
- Modern UI with gradient backgrounds and improved styling
- Enhanced user experience with emojis and better visual feedback
- Responsive design for mobile and desktop devices
- Improved camera controls and settings panel
- Better error handling and user feedback
- Enhanced Teachable Machine integration
- Access URLs:
  * Main Interface: http://172.20.10.4
  * Video Stream: http://172.20.10.4:81/stream
  * Single Capture: http://172.20.10.4/capture
  * Status API: http://172.20.10.4/status
  * Control API: http://172.20.10.4/control
*/