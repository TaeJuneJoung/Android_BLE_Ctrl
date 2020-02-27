#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#define BT_RX           A4
#define BT_TX           A5
#define NEO_PIXELS_PIN  10
#define NEO_PIXELS      7
#define GAS_PIN         A0
#define BUZZER_PIN      3
#define CDS_PIN         A1
#define VIBE_PIN        A3
const int ledPin =      11;
char cmd;
int gas_visited;
int cds_visited;
int vibe_visited;
int vibe_value;
unsigned long start_time;
bool buzzer_state = false;
SoftwareSerial HM10(BT_RX, BT_TX);
Adafruit_NeoPixel neo_pixels(NEO_PIXELS, NEO_PIXELS_PIN, NEO_GRBW + NEO_KHZ800);

void ledCtrl(bool state) {
  gas_visited = 0;
  cds_visited = 0;
  vibe_visited = 0;
  digitalWrite(ledPin, state);
}

/* Buzzer - Gas Sensor
 * @param(bool buzzer_state)
 */
void buzzer_sound(bool buzzer_state) {
  if(buzzer_state) {
    tone(BUZZER_PIN, 392, 100);
  } else {
    tone(BUZZER_PIN, 494, 100);
  }
}

/* Gas Sensor
 * @param(char v)
 */
void detect_gas(char v) {
  if(gas_visited == 0) {
    gas_visited = 1;
    cds_visited = 0;
    vibe_visited = 0;
    while(cmd == v){
      int gas_value = analogRead(GAS_PIN);
      String str_gas_value = (String)gas_value;
      int size = str_gas_value.length();
      char gas_value_char[4];
      str_gas_value.toCharArray(gas_value_char, 4);
      while(millis() - start_time > 500) {
        HM10.write(gas_value_char);
        start_time = millis();
        if(gas_value > 450) {
          buzzer_state = !buzzer_state;
          buzzer_sound(buzzer_state);
          if(buzzer_state) {
            neo_pixels_color(neo_pixels.Color(255, 0, 0, 0));
          } else {
            neo_pixels_color(neo_pixels.Color(0, 0, 255, 0));
          }
        } else {
          buzzer_state = false;
          neo_pixels_color(neo_pixels.Color(0, 0, 0, 0));
        }
        Serial.println(millis() - start_time);
      }
      ble_check();
    }
  }
}

/* NeoPixels
 * @param(uint32_t color)
 */
void neo_pixels_color(uint32_t color) {
  if(cmd != '7') {
    gas_visited = 0;
  } else if(cmd != '6') {
    vibe_visited = 0;
  }
  neo_pixels.clear();
  for(int i=0; i<NEO_PIXELS; i++) {
    neo_pixels.setPixelColor(i, color);
  }
  neo_pixels.show();
}

/* CDS(조도센서)
 * @param(char v)
 */
void detect_cds(char v) {
  if(cds_visited == 0) {
    gas_visited = 0;
    cds_visited = 1;
    vibe_visited = 0;
    while(cmd == v) { //보드에서는 반대로 되어있기에 처리 필요
      int cds_value = analogRead(CDS_PIN);
      cds_value = map(cds_value, 0, 1023, 0, 255);
      neo_pixels.setBrightness(cds_value);
      String str_cds_value = (String)cds_value;
      char cds_value_char[4];
      str_cds_value.toCharArray(cds_value_char, 4);
      while(millis() - start_time > 500) {
        HM10.write(cds_value_char);
        start_time = millis();
        neo_pixels_color(neo_pixels.Color(0,0,0,cds_value));
        neo_pixels.show();
        Serial.println(cds_value);
      }
      ble_check();
    }
  }
}

/* vibe_alarm
 * @param(char v)
 * : 진동 센서 일정 값 이상시 버저/네오픽셀 작동
 */
void vibe_alarm(char v) {
  start_time = millis();
  unsigned long delay_time = millis();
  bool state = false;
  while(cmd == v && millis() - start_time < 5000) {
    while(millis() - delay_time > 500) {
      delay_time = millis();
      state = !state;
      buzzer_sound(state);
      if(state) {
        neo_pixels_color(neo_pixels.Color(255, 0, 0, 0));
      } else {
        neo_pixels_color(neo_pixels.Color(0, 0, 255, 0));
      }
    }
    vibe_value = analogRead(VIBE_PIN);
    vibe_value = map(vibe_value, 0, 1023, 0, 100);
    //Serial.print("ALARM:");
    //Serial.println(vibe_value);
    ble_check();
  }
}

/* SW420(진동 센서)
 * @param(char v)
 */
void detect_vibe(char v) {
  if(vibe_visited == 0) {
    gas_visited = 0;
    cds_visited = 0;
    vibe_visited = 1;
    while(cmd == v) {
      vibe_value = analogRead(VIBE_PIN);
      vibe_value = map(vibe_value, 0, 1023, 0, 100);
      String str_vibe_value = (String)vibe_value;
      char vibe_value_char[4];
      str_vibe_value.toCharArray(vibe_value_char, 4);
      Serial.println(vibe_value);
      if(vibe_value > 50) {
        HM10.write(vibe_value_char);
        vibe_alarm(v);
      } else {
        while(millis() - start_time > 1000) {
          start_time = millis();
          HM10.write(vibe_value_char);
          neo_pixels_color(neo_pixels.Color(0, 0, 0, 0));
        }
      }
      ble_check();
    }
  }
}

/*
 * Bluetooth(HM10)
 */
void ble_check() {
  if(HM10.available()) {
    HM10.write("");
    cmd = HM10.read();
    Serial.println(cmd);
    switch(cmd) {
      case '0': //Stop
        ledCtrl(false);
        HM10.write("STOP");
        break;
      case '1':
        ledCtrl(true);
        HM10.write("전진");
        break;
      case '2':
        ledCtrl(false);
        HM10.write("좌회전");
        break;
      case '3':
        ledCtrl(true);
        HM10.write("우회전");
        break;
      case '4':
        ledCtrl(false);
        HM10.write("후진");
        break;
      case '5':
        detect_cds('5');
        break;
      case '6':
        detect_vibe('6');
        break;
      case '7':
        detect_gas('7');
        break;
    }
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(BT_RX, INPUT);
  pinMode(BT_TX, OUTPUT);
  Serial.println("\r\BT ON!!");
  HM10.begin(9600);
  pinMode(ledPin, OUTPUT);
  neo_pixels.begin();
  neo_pixels.setBrightness(50);
  pinMode(BUZZER_PIN, OUTPUT);
  start_time = millis();
}

void loop() {
  ble_check();

  if(Serial.available()) {
    HM10.write(Serial.read());
  }
}
