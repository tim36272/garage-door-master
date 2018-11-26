#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#define LOG_LEVEL_DEBUG
#include "log.h"
#include "EasyAPConfig.h"

#define CFG_OFFSET (0)
#define HTTP_SERVER_PORT (80)
#define DOOR1_CONTROL_PIN (4)
#define DOOR2_CONTROL_PIN (5)

void HandleDiscrete(void);

typedef struct ProgramCtrl_s {
    EasyAPConfig apConfig;
    char setupAPName[16];
    ESP8266WebServer httpServer;
    ESP8266HTTPUpdateServer updateServer;
    ProgramCtrl_s() : apConfig(CFG_OFFSET), httpServer(HTTP_SERVER_PORT) {
        //Setup EasyAPConfig
        sprintf(setupAPName,"setup_%hu",(uint16_t)(micros() % 65536)/65536);
        //Setup HTTP updater
        updateServer.setup(&httpServer, "USERNAME", "PASSWORD");

        //Setup discrete server
        httpServer.on("/",HandleDiscrete);
        httpServer.begin();
    }
} ProgramCtrl_t;

ProgramCtrl_t g_ctrl;

static String prepareHtmlPage()
{
    String htmlPage =
        String("<html><form method=\"post\">Click to open stuff door:<input type=\"submit\" name=\"t1\"/></form><form method=\"post\">Click to open vehicle door:<input type=\"submit\" name=\"t2\"/></form></html>");
    return htmlPage;
}

void HandleDiscrete(void) {
    if (g_ctrl.httpServer.hasArg("t1")) {
        //Toggle discrete
        digitalWrite(DOOR1_CONTROL_PIN,HIGH);
        delay(500);
        digitalWrite(DOOR1_CONTROL_PIN,LOW);
    }
    if (g_ctrl.httpServer.hasArg("t2")) {
        //Toggle discrete twice (that door requires two button presses)
        digitalWrite(DOOR2_CONTROL_PIN,HIGH);
        delay(500);
        digitalWrite(DOOR2_CONTROL_PIN,LOW);
        delay(500);
        digitalWrite(DOOR2_CONTROL_PIN,HIGH);
        delay(500);
        digitalWrite(DOOR2_CONTROL_PIN,LOW);
    }
    g_ctrl.httpServer.send(200, "text/html", prepareHtmlPage());
}

void setup (void)
{
    pinMode(DOOR1_CONTROL_PIN,OUTPUT);
    pinMode(DOOR2_CONTROL_PIN,OUTPUT);
    
    digitalWrite(DOOR1_CONTROL_PIN,LOW);
    digitalWrite(DOOR2_CONTROL_PIN,LOW);
    
    Serial.begin(115200);
    while(!Serial);
  
    ion::LogEnable(&Serial);
    LOGINFO("Starting up");

    WiFi.hostname("GarageDoor");

    g_ctrl.apConfig.Connect(g_ctrl.setupAPName);
    //ArduinoOTA.setPasswordHash("26ad71af3dff48b2174bd64b35285d92"); //corresponds to a common username
}

void loop (void)
{
    g_ctrl.apConfig.Connect(g_ctrl.setupAPName);
    LOGDEBUG("Client IP: %s", WiFi.localIP().toString().c_str());
    g_ctrl.httpServer.handleClient();
    delay(2000);
}
