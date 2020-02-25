#include <SoftwareSerial.h>
#define BT_RX A4
#define BT_TX A5

const int ledPin = 11;
bool ledState = false;
char cmd;
String message;

SoftwareSerial HM10(BT_RX, BT_TX);

boolean isValidNumber(String str) {
  for(byte i=0;i<str.length();i++) {
    if(isDigit(str.charAt(i))) {
      return true;
    }
  }
  return false;
}

void ledCtrl(bool state) {
  digitalWrite(ledPin, state);
}

void setup() {
  Serial.begin(9600);
  pinMode(BT_RX, INPUT);
  pinMode(BT_TX, OUTPUT);
  Serial.println("\r\nPOWER ON!!");
  HM10.begin(9600);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  if(HM10.available()) {
    cmd = HM10.read();

    switch(cmd) {
      case '0': //stop
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
        ledCtrl(true);
        HM10.write("자율주행");
        break;
      case '6':
        ledCtrl(false);
        HM10.write("적외선 감지");
        break;
      case '7':
        ledCtrl(true);
        HM10.write("조도 센서");
        break;
      case '8':
        ledCtrl(false);
        HM10.write("진동 센서");
        break;
      case '9':
        ledCtrl(true);
        HM10.write("초음파 센서");
        break;
    }
  }

  if(Serial.available()) {
    HM10.write(Serial.read());
  }
}
