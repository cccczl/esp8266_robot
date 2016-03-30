#include <ESP8266WiFi.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>

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

const char* MY_SSID = "ROBOT";
const char* MY_KEY = "robot";

Ticker timer;
int op_mode = STA;
bool msg_received = false;
ESP8266WebServer server(HTTP_PORT);

void hwif_dsleep() {
  timer.detach();
  Serial.println("INFO: deep sleep, zzzz ... ");
  system_deep_sleep(0);
}

void hwif_dsleep_timer() {
  timer.detach();  
  
  if (msg_received == false) {
    hwif_dsleep();
  }
}

void hwif_press(int pin) {
  digitalWrite(pin, HIGH);
  Serial.print("DEBUG: press pin ");
  Serial.println(pin);
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
    if (cmd <=PRG || cmd >= START || hwif_cmd(cmd) == false) {
      return false;
    }
  }

  hwif_cmd(PRG);
  hwif_cmd(START);
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
    Serial.println(" to HIGH.");
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

void http_handle_root() {
  msg_received = true;
  
  if (server.hasArg("robot")) {
    String cmds = server.arg("robot");

    if (robot_prg(cmds)) {
      server.send(200, "text/plain", "Success");
    } else {
      server.send(200, "text/plain", "Warning: robot commands not valid");
    }
    
  } else if (server.hasArg("ssid") && server.hasArg("passphrase")) {
    
    if (wifi_credentials(server.arg("ssid"), server.arg("passphrase"))) {
      server.send(200, "text/plain", "Success");
    } else {
      server.send(200, "text/plain", "Warning: access point credentials not set");
    }
    
  } else {
    server.send(200, "text/plain", "Error: no robot argument");
  }

  hwif_dsleep();
}

void http_handle_not_found(){
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
  server.send(404, "text/plain", message);
}

void http_setup() {
  Serial.print("starting HTTP server (port:");
  Serial.print(HTTP_PORT);
  Serial.print(") ... ");
  
  server.on("/", HTTP_GET, http_handle_root);
  server.onNotFound(http_handle_not_found);
  server.begin();
  
  Serial.println("done.");
}

void timer_reset() {
  timer.detach();
  timer.attach(60, hwif_dsleep_timer);
}

void setup(void) {
  hwif_setup();
  wifi_setup();
  http_setup();

  timer.attach(60, hwif_dsleep_timer);
}

void loop(void){
  server.handleClient();
}
