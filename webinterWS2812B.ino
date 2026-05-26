#include <WiFi.h>
#include <WebServer.h>
#include <FastLED.h>


#define LED_PIN     13
#define NUM_LEDS    300
CRGB leds[NUM_LEDS];


int globalBrightness = 100;
CRGB lastColor = CRGB::White;
int currentMode = 0;
uint8_t hue = 0;


// Переменные скорости и Демо-режима
int globalSpeed = 5;
bool demoModeActive = false;
int demoIntervalSec = 5; // Время работы одного эффекта в секундах по умолчанию
int demoSpeed = 5;       // Скорость эффектов в демо-режиме по умолчанию
unsigned long lastModeSwitchTime = 0; // Таймер для смены режимов


WebServer server(80);


CRGB hexToCRGB(String hex) {
  long number = strtol(hex.c_str(), NULL, 16);
  return CRGB((number >> 16) & 0xFF, (number >> 8) & 0xFF, number & 0xFF);
}


void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  // CSS Переменные для быстрой смены темы
  html += ":root { --bg: #1a1a1a; --text: #eee; --card-bg: #333; --border: #333; --btn-bg: #444; }";
  html += "body.light { --bg: #f0f2f5; --text: #1a1a1a; --card-bg: #ffffff; --border: #ddd; --btn-bg: #e0e0e0; }";
 
  html += "body{font-family:sans-serif; background: var(--bg); color: var(--text); text-align:center; padding:10px; transition: background 0.3s, color 0.3s;}";
  html += ".theme-toggle{position: absolute; top: 15px; right: 15px; font-size: 24px; cursor: pointer; user-select: none; z-index: 100;}";
  html += ".tabs{display:flex; justify-content:center; margin-bottom:20px; border-bottom:2px solid var(--border); max-width: 400px; margin-left: auto; margin-right: auto;}";
  html += ".tab{padding:10px 20px; cursor:pointer; font-weight:bold; color:#888;}";
  html += ".tab.active{color:#4CAF50; border-bottom:2px solid #4CAF50;}";
  html += ".content{display:none;} .content.active{display:block;}";
  html += ".grid{display:grid; grid-template-columns:repeat(4,1fr); gap:8px; max-width:320px; margin:15px auto;}";
  html += ".card{background: var(--card-bg); padding:10px; border-radius:8px; margin-bottom:10px; border: 1px solid var(--border); transition: background 0.3s;}";
  html += ".all-color-card{background: var(--card-bg); border:1px solid #4CAF50; padding:15px; border-radius:8px; margin:10px auto; max-width:300px;}";
  html += ".btn{padding:15px; margin:5px; cursor:pointer; border:none; border-radius:8px; background: var(--btn-bg); color: var(--text); font-weight:bold; width:80%; max-width:200px;}";
  html += ".on-btn{background:#4CAF50; color:white;} .off-btn{background:#f44336; color:white;} input[type='range']{width:90%;} .color-swatch{width:60px; height:30px; cursor:pointer;}";
  html += ".input-txt{padding:8px; border-radius:6px; border:1px solid #555; background: var(--card-bg); color: var(--text); width:65px; font-size:16px; text-align:center; vertical-align: middle;}";
  html += ".flex-row{display: flex; justify-content: center; align-items: center; gap: 10px; margin-top: 10px;}";
  html += "</style></head><body>";


  // Кнопка смены темы (Эмодзи Луны/Солнца)
  html += "<div class='theme-toggle' id='themeBtn' onclick='toggleTheme()'>🌙</div>";


  // Навигация по вкладкам
  html += "<div class='tabs'>";
  html += "<div class='tab active' onclick='openTab(event, \"main\")'>Управление</div>";
  html += "<div class='tab' onclick='openTab(event, \"modes\")'>Режимы</div>";
  html += "<div class='tab' onclick='openTab(event, \"demo\")'>Демо</div>";
  html += "</div>";


  // Вкладка 1: Управление
  html += "<div id='main' class='content active'>";
  html += "<div class='card'>Яркость<br><input type='range' min='0' max='255' value='" + String(globalBrightness) + "' onchange='sendVal(\"bright\", this.value)'></div>";
  html += "<div><button class='btn off-btn' onclick='sendVal(\"pwr\", 0)'>ВЫКЛЮЧИТЬ</button><button class='btn on-btn' onclick='sendVal(\"pwr\", 1)'>ВКЛЮЧИТЬ</button></div>";
  html += "<div class='all-color-card'><strong>ОБЩИЙ ЦВЕТ</strong><br><br><input type='color' onchange='sendColor(\"all\", this.value)' style='width:100%; height:40px; cursor:pointer;'></div>";


  html += "<h3>Сетка диодов</h3><div class='grid'>";
  for(int i=0; i<NUM_LEDS; i++) {
    html += "<div class='card'>" + String(i+1) + "<br><input type='color' onchange='sendColor(" + String(i) + ", this.value)'></div>";
  }
  html += "</div></div>";


  // Вкладка 2: Режимы
  html += "<div id='modes' class='content'>";
  html += "<div class='card' style='border:1px solid #4CAF50;'><strong>Скорость эффектов</strong><br><input type='range' min='1' max='10' value='" + String(globalSpeed) + "' onchange='sendVal(\"speed\", this.value)'></div>";
  html += "<h3>Выбор эффекта</h3><div class='card'>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 0)'>СТАТИКА</button><br>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 1)'>РАДУГА</button><br>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 2)'>КОМЕТА</button><br>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 3)'>ОГОНЬ</button><br>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 4)'>ПАДАЮЩИЙ СНЕГ</button><br>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 5)'>ДИСКО ШАР</button><br>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 6)'>КОНФЕТТИ</button><br>";
  html += "<button class='btn' onclick='sendVal(\"mode\", 7)'>БЕГУЩАЯ ВОЛНА</button>";
  html += "</div></div>";


  // Вкладка 3: Демо режим
  html += "<div id='demo' class='content'>";
  html += "<div class='card' style='border:1px solid #2196F3;'><strong>Управление Демо-режимом</strong><br><br>";
  html += "<button class='btn off-btn' onclick='sendVal(\"demo\", 0)'>СТОП ДЕМО</button>";
  html += "<button class='btn on-btn' onclick='sendVal(\"demo\", 1)'>СТАРТ ДЕМО</button></div>";
 
  // Связанные Ползунок + Поле ввода секунд
  html += "<div class='card'><strong>Время работы режима (сек)</strong><br>";
  html += "<div class='flex-row'>";
  html += "<input type='range' id='secRange' min='1' max='60' value='" + String(demoIntervalSec) + "' oninput='syncDemoTime(\"range\")'>";
  html += "<input type='number' id='secInput' class='input-txt' min='1' max='60' value='" + String(demoIntervalSec) + "' oninput='syncDemoTime(\"input\")'>";
  html += "</div></div>";
 
  html += "<div class='card'><strong>Скорость в Демо</strong><br><input type='range' min='1' max='10' value='" + String(demoSpeed) + "' onchange='sendVal(\"demospeed\", this.value)'></div>";
  html += "</div>";


  html += "<script>";
  html += "function openTab(evt, tabName){var i, content, tabs; content = document.getElementsByClassName('content'); for(i=0; i<content.length; i++){content[i].classList.remove('active');} tabs = document.getElementsByClassName('tab'); for(i=0; i<tabs.length; i++){tabs[i].classList.remove('active');} document.getElementById(tabName).classList.add('active'); evt.currentTarget.classList.add('active');}";
  html += "function sendVal(k, v){fetch('/'+k+'?v='+v);}\n";
  html += "function sendColor(id, hex){fetch('/color?id='+id+'&hex='+hex.replace('#',''));}\n";
 
  // Синхронизация ввода и ползунка + отправка значения на ESP
  html += "function syncDemoTime(src){var r=document.getElementById('secRange'); var i=document.getElementById('secInput'); if(src==='range'){i.value=r.value;}else{if(i.value<1)i.value=1; if(i.value>60)i.value=60; r.value=i.value;} sendVal('demotime', i.value);}\n";
 
  // Смена темы оформления (светлая / темная)
  html += "function toggleTheme(){var b=document.body; var btn=document.getElementById('themeBtn'); if(b.classList.contains('light')){b.classList.remove('light'); btn.innerHTML='🌙';}else{b.classList.add('light'); btn.innerHTML='☀️';}}";
  html += "</script></body></html>";


  server.send(200, "text/html", html);
}


// Функции эффектов
void effectRainbow() {
  static uint8_t i = 0; static bool forward = true;
  EVERY_N_MILLISECONDS_I(thistimer, 120) {
    thistimer.setPeriod(map(globalSpeed, 1, 10, 240, 20));
    fadeToBlackBy(leds, NUM_LEDS, 150);
    leds[i] = CHSV(hue, 255, 255); hue += 30;
    if (forward) { i++; if (i >= NUM_LEDS - 1) forward = false; }
    else { i--; if (i <= 0) forward = true; }
  }
}


void effectComet() {
  static int pos = 0; static int direction = 1;
  EVERY_N_MILLISECONDS_I(thistimer, 150) {
    thistimer.setPeriod(map(globalSpeed, 1, 10, 300, 30));
    fadeToBlackBy(leds, NUM_LEDS, 100);
    leds[pos] = CRGB(0, 255, 200);
    pos += direction; if (pos == NUM_LEDS - 1 || pos == 0) direction = -direction;
  }
}


void effectFire() {
  EVERY_N_MILLISECONDS_I(thistimer, 60) {
    thistimer.setPeriod(map(globalSpeed, 1, 10, 150, 15));
    for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB(random(10, 256), 0, 0);
  }
}


void effectSnow() {
  EVERY_N_MILLISECONDS_I(thistimer, 100) {
    thistimer.setPeriod(map(globalSpeed, 1, 10, 250, 25));
    fadeToBlackBy(leds, NUM_LEDS, 40);
    if (random8() < 60) {
      int i = random(0, NUM_LEDS);
      leds[i] = CRGB::White;
    }
  }
}


void effectDisco() {
  EVERY_N_MILLISECONDS_I(thistimer, 80) {
    thistimer.setPeriod(map(globalSpeed, 1, 10, 200, 20));
    for (int i = 0; i < NUM_LEDS; i++) {
      if (random8() < 80) {
        leds[i] = CHSV(random8(), 255, 255);
      } else {
        leds[i] = CRGB::Black;
      }
    }
  }
}


void effectConfetti() {
  EVERY_N_MILLISECONDS_I(thistimer, 20) {
    thistimer.setPeriod(map(globalSpeed, 1, 10, 60, 5));
    fadeToBlackBy(leds, NUM_LEDS, 10);
    if (random8() < 40) {
      int pos = random16(NUM_LEDS);
      leds[pos] += CHSV(hue + random8(64), 200, 255);
    }
    hue++;
  }
}


void effectWave() {
  EVERY_N_MILLISECONDS_I(thistimer, 30) {
    thistimer.setPeriod(map(globalSpeed, 1, 10, 80, 8));
    static uint8_t waveIndex = 0;
    waveIndex++;
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(waveIndex + (i * 15), 255, 255);
    }
  }
}


void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(globalBrightness);
  WiFi.softAP("ESP32_LED_Controller", "password123");


  server.on("/", handleRoot);
  server.on("/bright", []() { globalBrightness = server.arg("v").toInt(); FastLED.setBrightness(globalBrightness); server.send(200); });
  server.on("/speed", []() { globalSpeed = server.arg("v").toInt(); server.send(200); });


  server.on("/demo", []() {
    int state = server.arg("v").toInt();
    if (state == 1) {
      demoModeActive = true;
      globalSpeed = demoSpeed;
      currentMode = 1;        
      lastModeSwitchTime = millis();
    } else {
      demoModeActive = false;
      currentMode = -1;
      fill_solid(leds, NUM_LEDS, CRGB::Black);
    }
    server.send(200);
  });


  server.on("/demotime", []() {
    demoIntervalSec = server.arg("v").toInt();
    if (demoIntervalSec < 1) demoIntervalSec = 1;
    server.send(200);
  });


  server.on("/demospeed", []() {
    demoSpeed = server.arg("v").toInt();
    if (demoModeActive) {
      globalSpeed = demoSpeed;
    }
    server.send(200);
  });


  server.on("/pwr", []() {
    int state = server.arg("v").toInt();
    demoModeActive = false;
    if (state == 0) { currentMode = -1; fill_solid(leds, NUM_LEDS, CRGB::Black); }
    else { currentMode = 0; fill_solid(leds, NUM_LEDS, lastColor); }
    server.send(200);
  });


  server.on("/mode", []() {
    demoModeActive = false;
    currentMode = server.arg("v").toInt();
    server.send(200);
  });
 
  server.on("/color", []() {
    demoModeActive = false;
    currentMode = 0;
    String id = server.arg("id");
    CRGB color = hexToCRGB(server.arg("hex"));
    if (id == "all") {
      lastColor = color;
      fill_solid(leds, NUM_LEDS, color);
    } else {
      leds[id.toInt()] = color;
    }
    server.send(200);
  });


  server.begin();
}


void loop() {
  server.handleClient();


  if (demoModeActive) {
    if (millis() - lastModeSwitchTime >= ((unsigned long)demoIntervalSec * 1000)) {
      lastModeSwitchTime = millis();
      currentMode++;
      if (currentMode > 7) {
        currentMode = 1;
      }
    }
  }


  if (currentMode == 1) effectRainbow();
  else if (currentMode == 2) effectComet();
  else if (currentMode == 3) effectFire();
  else if (currentMode == 4) effectSnow();
  else if (currentMode == 5) effectDisco();
  else if (currentMode == 6) effectConfetti();
  else if (currentMode == 7) effectWave();
 
  FastLED.show();
}



