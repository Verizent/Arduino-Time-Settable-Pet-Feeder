#include <DS3231.h>
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', ' '},
  {'4', '5', '6', ' '},
  {'7', '8', '9', ' '},
  {'*', '0', '#', ' '} // * set   # cancel   4digits enter
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
DS3231 rtc(A4, A5);
Servo feed_servo;
LiquidCrystal lcd(A0, A1, A2, 11, 12, 13); // rs, en, d4,..,d7

// int hour, minute;  // int t1, t2, t3, t4, t5, t6; // save time (running time)
int feedHour, feedMinute; // int r[6]; // array save setting time (feeding time)
boolean feed;
String timeStr;

void closeFeeder()
{
  feed_servo.write(55);
}

void openFeeder()
{
  feed_servo.write(100);
  delay(1000);
}

void setup()
{
  Serial.begin(9600);
  pinMode(A0, OUTPUT); // RS
  pinMode(A1, OUTPUT); // Enable
  pinMode(A2, OUTPUT); // D4 -- always HIGH
  lcd.begin(16, 2);

  rtc.begin();

  feed_servo.attach(10);
  closeFeeder();

//  buttonPress = 0;
  feed = false;
}

void showDateTime(String t)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time  ");
  lcd.print(t);
  lcd.setCursor(0, 1);
  if (feed == true) {
    lcd.print("Feeding  ");
    lcd.print('0' + feedHour / 10 - 48);
    lcd.print('0' + feedHour % 10 -48);
    lcd.print(':');
    lcd.print('0' + feedMinute / 10 - 48);
    lcd.print('0' + feedMinute % 10 - 48);
  } else {
    lcd.print("Not Feeding");
  }
}

void loop()
{
  timeStr = rtc.getTimeStr();
  showDateTime(timeStr);
  if (feed == true)
    tryFeeding(timeStr);

  if (kpd.getKey() == '*')
    setFeedingTime();
}

void tryFeeding(String t)
{
  int hour = (t.charAt(0) - '0') * 10 + (t.charAt(1) - '0');
  int minute = (t.charAt(3) - '0') * 10 + (t.charAt(4) - '0');

  if (hour == feedHour && minute == feedMinute) {
    openFeeder();
    closeFeeder();
    feed = false;
  }
}

void setFeedingTime()
{
  char key;
  int i = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HH:MM");
  lcd.setCursor(0, 1);

  while (i < 4) {
    key = kpd.getKey();
    switch (key) {
    case '0': case '1': case '2': case '3': 
    case '4': case '5': case '6': case '7': 
    case '8': case '9': // do input digit
      if (i == 0 && key >= '0' && key <= '2') { // puluhan jam
        feedHour = (key - '0') * 10;
        lcd.print(key);
        i++;
      } else if (i == 1 && feedHour < 20 && key >= '0' && key <= '9') { // satuan jam 00..19
        feedHour += key - '0';
        lcd.print(key);
        lcd.print(':');
        i++;
      } else if (i == 1 && feedHour == 20 && key >= '0' && key <= '3') { // satuan jam 20..23
        feedHour += key - '0';
        lcd.print(key);
        lcd.print(':');
        i++;
      } else if (i == 2 && key >= '0' && key <= '5') { // puluhan menit
        feedMinute = (key - '0') * 10;
        lcd.print(key);
        i++;
      } else if (i == 3 && key >= '0' && key <= '9') { // satuan menit
        feedMinute += key - '0';
        lcd.print(key);
        i++;
      }
      if (i == 4)
        feed = true;
      break;
    case '#': // do cancel
      feed = false;
      i = 4;
      break;
    default: // do nothing
      break;
    }
    delay(200);
  }
}
