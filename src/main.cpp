// #include "web.h"
#include <Arduino.h>
#include <web.h>
#include <DNSServer.h>
#include <mdns.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <Preferences.h>

// 白色 gnd
// 绿色 拍照
// 灰色 对焦

const char *ap_ssid = "ESP32-Camera-AP"; // 热点名称
const char *ap_password = "12345678";    // 热点密码（至少8位）

#define BUTTON_PIN 0   // 按钮输入引脚（内部上拉，低电平有效）
#define FOCUS_PIN 11   // 对焦控制引脚（拉低有效）
#define SHUTTER_PIN 13 // 快门控制引脚（拉低有效）

// timelapse相关全局变量
volatile bool timelapseActive = false;
int timelapseInterval = 0;
int timelapseCount = 0;
int timelapseShot = 0;
unsigned long timelapseLastTime = 0;
int timelapseShutter = 100; // 默认快门速度（毫秒）
bool timelapseBuleMode = false;

// 声明WebServer对象
AsyncWebServer server(80);

void focus()
{
    digitalWrite(FOCUS_PIN, LOW);  // 拉低对焦线
    delay(100);                    // 等待对焦完成 
    digitalWrite(FOCUS_PIN, HIGH); // 释放对焦线
}

// 触发相机对焦和拍照
void triggerCamera(int shutterMs = 100)
{
    digitalWrite(FOCUS_PIN, LOW);
    delay(100);
    digitalWrite(SHUTTER_PIN, LOW);
    delay(shutterMs); // 使用自定义快门速度
    digitalWrite(FOCUS_PIN, HIGH);
    digitalWrite(SHUTTER_PIN, HIGH);
}

void timelapseShoot(int interval, int count)
{
    for (int i = 0; i < count; ++i)
    {
        triggerCamera();
        if (i < count - 1)
            delay(interval * 1000);
    }
}

void setup()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password); // 设置热点名称和密码

    mdns_init();                     // 初始化 mDNS
    mdns_hostname_set("esp32");      // 设置 mDNS 主机名
    mdns_instance_name_set("ESP32"); // 设置 mDNS 实例名称

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", index_html); });

    // 拍照接口
    server.on("/shoot", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        triggerCamera();
        request->send(200, "text/plain", "拍照完成"); });

    // 定时拍摄接口（非阻塞，只设置参数和启动标志）
    server.on("/timelapse", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        if (request->hasParam("interval") && request->hasParam("count")) {
            timelapseInterval = request->getParam("interval")->value().toInt();
            timelapseCount = request->getParam("count")->value().toInt();
            timelapseShot = 0;
            timelapseLastTime = millis();
            timelapseActive = true;
            // 处理bule模式和快门速度
            if (request->hasParam("bule") && request->getParam("bule")->value() == "1" && request->hasParam("shutter")) {
                timelapseBuleMode = true;
                timelapseShutter = request->getParam("shutter")->value().toInt() * 1000; // 秒转毫秒
            } else {
                timelapseBuleMode = false;
                timelapseShutter = 100;
            }
            request->send(200, "text/plain", "定时拍摄任务已启动");
        } else {
            request->send(400, "text/plain", "参数错误");
        }
    });

    // 停止定时拍摄接口
    server.on("/timelapse_stop", HTTP_GET, [](AsyncWebServerRequest *request){
        timelapseActive = false;
        request->send(200, "text/plain", "拍摄已停止");
    });

    server.begin();

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(FOCUS_PIN, OUTPUT);
    pinMode(SHUTTER_PIN, OUTPUT);
    digitalWrite(FOCUS_PIN, HIGH);   // 初始状态：不触发对焦
    digitalWrite(SHUTTER_PIN, HIGH); // 初始状态：不触发快门
    Serial.begin(115200);
}

void focusHoldAndShoot()
{
    static uint32_t lastDebounceTime = 0;
    static bool lastButtonState = HIGH;
    static bool focusActive = false;
    static bool shutterPending = false;
    const uint32_t debounceDelay = 50;

    bool buttonState = digitalRead(BUTTON_PIN);

    // 消抖
    if (buttonState != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        // 按钮按下，保持对焦
        if (buttonState == LOW && !focusActive)
        {
            digitalWrite(FOCUS_PIN, LOW); // 保持对焦
            focusActive = true;
        }
        // 按钮松开，释放对焦并准备触发快门
        else if (buttonState == HIGH && focusActive)
        {
            digitalWrite(FOCUS_PIN, HIGH); // 释放对焦
            shutterPending = true;         // 标记需要触发快门
            focusActive = false;
        }
    }

    // 独立快门触发逻辑，确保快门动作完整
    if (shutterPending)
    {
        // 先拉低对焦线
        digitalWrite(FOCUS_PIN, LOW);
        delay(100); // 可根据需要调整

        // 再拉低快门线
        digitalWrite(SHUTTER_PIN, LOW);
        delay(100);

        // 全部释放
        digitalWrite(FOCUS_PIN, HIGH);
        digitalWrite(SHUTTER_PIN, HIGH);

        shutterPending = false; // <--- 关键：快门动作完成后清零
    }

    lastButtonState = buttonState;
}

void loop()
{
    focusHoldAndShoot();

    if (timelapseActive && timelapseShot < timelapseCount)
    {
        unsigned long now = millis();
        unsigned long intervalMs = (unsigned long)timelapseInterval * 1000;
        if (timelapseShot == 0 || (now - timelapseLastTime) >= intervalMs)
        {
            if (timelapseBuleMode)
                triggerCamera(timelapseShutter);
            else
                triggerCamera();
            timelapseShot++;
            timelapseLastTime = now;
        }
        if (timelapseShot >= timelapseCount)
        {
            timelapseActive = false;
            Serial.println("定时拍摄完成");
        }
    }
}