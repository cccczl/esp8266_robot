/* 
  Copyright (c) 2016 Thomas Graziadei. All rights reserved.
  
  Thanks to Hristo Gochkov for the ESP8266WebServer library 
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in $(ls -A1); do curl -F "file=@$PWD/$file" esp8266fs.local/add; done

  to remove files use curl -X DELETE esp8266.local/remove?path=file
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <FS.h>

extern "C" {
#include "user_interface.h"
}

enum COMMAND {
  PRG=0,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  START
};

enum OP_MODE {
  AP=LOW,
  STA=HIGH
};

const int MAX_GPIOS = 6;
const int GPIOS[MAX_GPIOS] = {4,5,12,13,14,15};
const int GPIO_IN = 16;
const int HTTP_PORT = 80;
const int MAX_COMMANDS = 50;

const int HTTP_OK = 200;
const int HTTP_NOT_FOUND = 404;
const int HTTP_SERVER_ERROR = 500;

const char* MY_SSID = "ROBOT";
const char* MY_KEY = "robot";

Ticker timer;
int op_mode = STA;
bool msg_received = false;
bool sleeping_enabled = true;
ESP8266WebServer server(HTTP_PORT);
File fsUploadFile;

void hwif_dsleep() {
  timer.detach();

  if (sleeping_enabled) {
    Serial.println("INFO: deep sleep, zzzz ... ");
    system_deep_sleep(0);
  }
}

void hwif_dsleep_timer() {
  timer.detach();  
  
  if (msg_received == false) {
    hwif_dsleep();
  }
}

void hwif_press(int pin) {
  Serial.print("DEBUG: press pin ");
  Serial.println(pin);

  digitalWrite(pin, HIGH);
  delay(100);
  digitalWrite(pin, LOW);
}

bool hwif_cmd(int cmd) {
  if (cmd >= 0 && cmd < MAX_GPIOS) {
    hwif_press(GPIOS[cmd]);
    return true;
  }
  
  return false;
}

void hwif_setup(void) {
  Serial.begin(115200);
  Serial.println("");
  Serial.print("hwif setup ... ");

  for (int i=0; i<MAX_GPIOS; i++) {
    pinMode(GPIOS[i], OUTPUT);
    digitalWrite(GPIOS[i], LOW);    
  }

  pinMode(GPIO_IN, INPUT_PULLUP);  
  Serial.println("done.");

  Serial.print("using gpio ");
  Serial.print(GPIO_IN);
  Serial.print(" to select mode: ");

  op_mode = digitalRead(GPIO_IN);
  if (op_mode == AP) {
    Serial.println("AP");
  } else {
    Serial.println("STA");
  }
}

bool robot_prg(String cmds) {
  int cmd;
  char commands[MAX_COMMANDS];
  cmds.toCharArray(commands, MAX_COMMANDS);

  for (int i=0; i<MAX_COMMANDS && commands[i] != 0; i++) {
    cmd = commands[i] - '0';
    if (cmd < PRG || cmd > START || hwif_cmd(cmd) == false) {
      Serial.print("Warning: unknown command");
      Serial.println(cmd);
      return false;
    }
  }

  return true;
}

void wifi_ap(void) {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(MY_SSID, MY_KEY);
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(" done.");
  
  Serial.print("AP SSID: ");
  Serial.println(MY_SSID);
  Serial.print("IP Address: ");
  Serial.println(myIP);
}

bool wifi_sta_connect(void) {
  int timeout = 10 * 2; // 10 secound timeout
  int count = 0; 

  Serial.print("connect to ");
  Serial.print(WiFi.SSID());
  Serial.print(" .");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED && count < timeout) {
    delay(500);
    Serial.print(".");
    count++;
  }

  if (count < timeout) {
    Serial.println(" done.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  
  } else {
    Serial.println(" failed (timeout).");
    Serial.print("Try and reconfigure in AP mode. Set gpio ");
    Serial.print(GPIO_IN);
    Serial.println(" to LOW.");
  }

  return (count < timeout);
}

void wifi_sta(void) {
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  
  if (wifi_sta_connect() == false) {
    hwif_dsleep();  
  }
}

void timer_reset();
bool wifi_credentials(String ssid, String passphrase) {
  timer_reset();
  WiFi.begin(ssid.c_str(), passphrase.c_str());
  return wifi_sta_connect();
}

void wifi_setup(void) {
  Serial.print("wifi setup ");

  WiFi.persistent(true);
  if (op_mode == AP) {
    wifi_ap();
  } else{
    wifi_sta();
  }
}

void http_cache(bool cache) {
  if (cache) {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  }  
}

void http_response_json(int code, bool cache, String json) {
  http_cache(cache);
  server.send(code, "text/json", json);
}

void http_response_plain(int code, bool cache, String msg) {
  http_cache(cache);
  server.send(code, "text/plain", msg);
}

void http_handle_robot() {
  msg_received = true;
  
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");

    if (robot_prg(cmd)) {
      http_response_plain(HTTP_OK, false, "Success");
    } else {
      http_response_plain(HTTP_OK, false, "Warning: robot commands not valid");
    }
    
  } else if (server.hasArg("ssid") && server.hasArg("passphrase")) {
    
    if (wifi_credentials(server.arg("ssid"), server.arg("passphrase"))) {
      String json = "{\"ip\":\"";
      json += WiFi.localIP().toString();
      json += "\"}";
      http_response_json(HTTP_OK, false, json);
    } else {
      http_response_plain(HTTP_OK, false, "Warning: access point credentials not set");
    }
    
  } else {
    http_response_plain(HTTP_OK, false, "Error: no robot argument");
  }

  hwif_dsleep();
}

void http_handle_sleep() {
  timer.detach();
  
  if (server.hasArg("time")) {
    String time = server.arg("time");
    Serial.print("Sleeping in ");
    Serial.print(time.toInt());
    Serial.println("s");

    sleeping_enabled = true;
    timer.attach(time.toInt(), hwif_dsleep_timer);
    http_response_plain(HTTP_OK, false, "Sleeping enabled");
  } else {
    sleeping_enabled = false;
    Serial.println("Sleeping disabled");
    http_response_plain(HTTP_OK, false, "Sleeping disabled");
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  return "text/plain";
}

bool http_handle_fileread(String path){
  msg_received = true;
  path.trim();
  
  Serial.println("Read file: " + path);
  if(path.endsWith("/")) {
    path += "index.html";
  }
  
  String contentType = getContentType(path);

  if(SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    timer_reset();
    
    http_response_plain(HTTP_OK, true, "File " + path + " read");
    return true;
  }

  Serial.print("Info: file ");
  Serial.print(path);
  Serial.println(" does not exist");
  return false;
}

void http_handle_fileupload(){
  msg_received = true;
  HTTPUpload& upload = server.upload();

  if(upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    
    if(!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    
    Serial.print("Upload file: "); 
    Serial.println(filename);
    
    fsUploadFile = SPIFFS.open(filename, "w");
  } else if(upload.status == UPLOAD_FILE_WRITE){

    if(fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {
      fsUploadFile.close();
    }
    
    Serial.print("Upload size: "); 
    Serial.println(upload.totalSize);

    String json = "{\"name\":\"" + upload.filename + "\",\"size\":";
    json += upload.totalSize;
    json += "}";
    http_response_json(HTTP_OK, false, json);
  }

  timer_reset();
}

void http_handle_filedelete(){
  msg_received = true;
  if(server.args() == 0) {
    return http_response_plain(HTTP_SERVER_ERROR, false, "BAD ARGS");
  }
  
  String path = server.arg(0);
  if(!path.startsWith("/")) {
    path = "/" + path;
  }

  Serial.println("Delete file: " + path);
  
  if(!SPIFFS.exists(path)) {
    return http_response_plain(HTTP_NOT_FOUND, false, path + "not found");
  }
  
  SPIFFS.remove(path);
  http_response_plain(HTTP_OK, false, path + " deleted");
  timer_reset();
}

void http_handle_filelist() {
  msg_received = true;
  String path = "/";
  if(server.hasArg("dir")) {  
    path = server.arg("dir");
  }
  
  Serial.println("Show directory: " + path);
  Dir dir = SPIFFS.openDir(path);

  String output = "[";
  while(dir.next()){
    if (output != "[") {
      output += ',';
    }
    
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += dir.fileName();
    output += "\",\"size\":";
    output += dir.fileSize();
    output += "}";
  }
  
  output += "]";
  http_response_json(HTTP_OK, false, output);
  timer_reset();
}

void http_handle_not_found(){
  msg_received = true;
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  http_response_plain(HTTP_NOT_FOUND, false, message);
  timer_reset();
}

void http_setup() {
  Serial.print("starting HTTP server (port:");
  Serial.print(HTTP_PORT);
  Serial.print(") ... ");

  server.on("/robot", HTTP_GET, http_handle_robot);
  server.on("/sleep", HTTP_GET, http_handle_sleep);
  server.on("/list", HTTP_GET, http_handle_filelist);
  server.on("/remove", HTTP_DELETE, http_handle_filedelete);

  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  //do not send cache headers (do not use http_response)
  server.on("/add", HTTP_POST, [](){ http_response_plain(HTTP_OK, true, ""); }, http_handle_fileupload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if(!http_handle_fileread(server.uri())) {
      http_handle_not_found();
    }
  });

  server.begin();
  
  Serial.println("done.");
}

void timer_reset() {
  timer.detach();
  timer.attach(60, hwif_dsleep_timer);
}

void spiffs_setup() {
  SPIFFS.begin();
}

void setup(void) {
  hwif_setup();
  spiffs_setup();
  wifi_setup();
  http_setup();

  timer.attach(60, hwif_dsleep_timer);
}

void loop(void){
  server.handleClient();
}
