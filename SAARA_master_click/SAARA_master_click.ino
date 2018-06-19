#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            8
#define NUMPIXELS      16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define SM1Address2 0
#define SM2Address2 2
#define EM1Address2 4
#define EM2Address2 14

#define upDownCountAddress 20
#define upDownCountAddress2 22

#define SM1Address 6
#define SM2Address 8
#define EM1Address 10
#define EM2Address 12 

#define HOME_BUTTON 24
#define DRINK_BUTTON 26
#define POWER_BUTTON 22
#define DOOR_BUTTON 23

int relayPin = 12;

int dir;
float upDownCount;
int upDownCountHome, upDownCountHome2;
int em2step = 1171;
int em2home = 1171;

int totalPower;
int totalhomeCorrection;
int drinkTotalCorrection;
int prox = 9; 

int hm1home;
int servoDirection, servoDirection2, servoRequired, servoRequired2;

typedef struct
{
  int pin;
  int speed = 2000;
  int dir = HIGH;
  int dirpin;
  int prox;
  int step = 0;
  int home;
  int enable;
  int home2;
  int powerHome;
  int powerCorrection;
  int homeCorrection;
  int drinkCorrection;  
} motor;

motor em1, em2, sm1, sm2;
Servo hm1, grip;
int v, r, t, w;
int mode_button = 28;
int joystick_button = 27;

int joystickX, joystickY; 
float hm1pos = 200;
float gripperpos = 95;
float hm1Test = 2300;  

volatile int powerFlag = 0, joystickFlag = 0, modeFlag = 0;
int mode1, power1, joystick1;
int mode = 0, power = 0, joystick = 0;
int speed, speed2; 
int timer = 0;
int drinkbut = 0;
int homebut = 0;
int doorbut = 0;
String steps = "steps";
String home = "home";
int count;
int accelerometer = 0;
int accelerometer2 = 0; 
volatile float y_accel, x_accel;
int sm2Count = 0, sm2Count2 = 0;
int em1Count = 0;
int hm1count = 0;
int servoCount = 0;
int led = 0; 
int led1 = 0; 
int led2 = 0;
int led3 = 0;
int led4 = 0;

int offSm1Move = 0;
int gripperLed = 0, hm1Led = 0, upDownLed, fwdBkLed; 
int sm1Count = 0;
int delayCount = 0;
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * -----------------------------------------------------------------------------------------------------------SETUP------------------------------------------------------------------------------------------------------------------------
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
void setup()
{
  Serial.begin(115200);
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  
  pixels.begin(); // This initializes the NeoPixel library.
  
  sm2.dirpin = 40; // output of the stepper motors where dir is connected to pin 4 of uStepper and 8 is connected to pin 3
  sm2.pin = 41;
  sm1.dirpin = 3;
  sm1.pin = 2;
  em1.pin = 4;
  em1.dirpin = 5;
  em2.pin = 39;
  em2.dirpin = 38;

  sm1.step = 0;
  sm2.step = 0;
  em1.step = 0;
  em2.step = 0;

  sm1.home = (EEPROM.read(6) * 255) + EEPROM.read(7);
  sm2.home = (EEPROM.read(8) * 255) + EEPROM.read(9);
  em1.home = (EEPROM.read(10) * 255) + EEPROM.read(11);
  em2.home = (EEPROM.read(12) * 255) + EEPROM.read(13);

  sm1.home2 = (EEPROM.read(0) * 255) + EEPROM.read(1);
  sm2.home2 = (EEPROM.read(2) * 255) + EEPROM.read(3);
  em1.home2 = (EEPROM.read(4) * 255) + EEPROM.read(5);
  em2.home2 = (EEPROM.read(14) * 255) + EEPROM.read(15);

  sm1.powerHome = 0;
  sm2.powerHome = -3000;
  em1.powerHome = -2400;
  em2.powerHome = 0;

  upDownCountHome = (EEPROM.read(20) * 255) + EEPROM.read(21);
  upDownCountHome2 = (EEPROM.read(22) * 255) + EEPROM.read(23);

  pinMode(prox, INPUT);
  pinMode(HOME_BUTTON, INPUT);
  pinMode(POWER_BUTTON, INPUT);
  pinMode(mode_button, INPUT);
  pinMode(DOOR_BUTTON, INPUT);
  pinMode(joystick_button, INPUT);
  pinMode(sm1.dirpin, OUTPUT);
  pinMode(sm1.pin, OUTPUT);
  pinMode(sm2.dirpin, OUTPUT);
  pinMode(sm2.pin, OUTPUT);
  pinMode(em1.dirpin, OUTPUT);
  pinMode(em1.pin, OUTPUT);
  pinMode(em2.pin, OUTPUT);
  pinMode(em2.dirpin, OUTPUT);
  pinMode(relayPin, OUTPUT);

  hm1.attach(6);
  grip.attach(7);
  grip.write(gripperpos);
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * -----------------------------------------------------------------------------------------------------------CUSTOM FUNCTION BLOCKS -------------------------------------------------------------------------------------------------------
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void makeStep(int motor, int speed)
{
  digitalWrite(motor, HIGH);
  delayMicroseconds(speed);
  digitalWrite(motor, LOW);
  delayMicroseconds(speed);
}

void makeStep2(int motor1, int motor2, int speed)
{
  digitalWrite(motor1, HIGH);
  digitalWrite(motor2, HIGH);
  delayMicroseconds(speed);
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
  delayMicroseconds(speed);
}

void makeStep3(int motor1, int motor2, int motor3, int speed)
{
  digitalWrite(motor1, HIGH);
  digitalWrite(motor2, HIGH);
  digitalWrite(motor3, HIGH);
  delayMicroseconds(speed);
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
  digitalWrite(motor3, LOW);
  delayMicroseconds(speed);
}

void makeStep4(int motor1, int motor2, int motor3, int motor4, int speed)
{
  digitalWrite(motor1, HIGH);
  digitalWrite(motor2, HIGH);
  digitalWrite(motor3, HIGH);
  digitalWrite(motor4, HIGH);
  delayMicroseconds(speed);
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
  digitalWrite(motor3, LOW);
  digitalWrite(motor4, LOW);
  delayMicroseconds(speed);
}

void makeSteps(int motor, int speed, int steps)
{
  int x = 1;
  while (x <= steps)
  {
    makeStep(motor, speed);
    x++;
  }
}

void makeSteps3(int motor1, int motor2, int motor3, int speed, int steps)
{
  int x = 1;
  while (x <= steps)
  {
    makeStep3(motor1, motor2, motor3, speed);
    x++;
  }
}

void makeSteps4(int motor1, int motor2, int motor3, int motor4, int speed, int steps)
{
  int x = 1;
  while (x <= steps)
  {
    makeStep4(motor1, motor2, motor3, motor4, speed);
    x++;
  }
}

void makeScaledStep(int motor1, int motor2, int speed, float scale)
{
  int step1 = 5;
  int step2 = 5;
  step2 = step1 * scale;
  makeSteps(motor1, speed, step1);
  makeSteps(motor2, speed, step2);
}

void makeScaledStep2(int motor1, int motor2, int speed, int speed2, float scale)
{
  int step1 = 30;
  int step2 = 30;
  step2 = step1 * scale;
  makeSteps(motor1, speed, step1);
  makeSteps(motor2, speed2, step2);
}

void makeScaledStep3(int motor1, int motor2, int speed, int speed2, float scale)
{
  int step1 = 10;
  int step2 = 10;
  step2 = step1 * scale;
  makeSteps(motor1, speed, step1);
  makeSteps(motor2, speed2, step2);
}

void fwdBk(){
  if (joystickY > 600 && sm2.step <= 2230 && em1.step <= 6670){
    digitalWrite(sm2.dirpin, HIGH);
    digitalWrite(em1.dirpin, HIGH);
    makeScaledStep3(sm2.pin, em1.pin, speed2*1.5, speed2, 0.8);
    sm2.step += 10;
    em1.step += 8;
  }
  else if (joystickY > 600 && sm2.step >= 2230 && em1.step <= 6670){
    digitalWrite(sm2.dirpin, HIGH);
    digitalWrite(em1.dirpin, HIGH);
    makeScaledStep3(sm2.pin, em1.pin, speed2*1.5, speed2, 1.2);
    sm2.step += 10;
    em1.step += 12;
  }
  else if (joystickY < 450 && sm2.step <= 2230){
    digitalWrite(sm2.dirpin, LOW);
    digitalWrite(em1.dirpin, LOW);
    makeScaledStep3(sm2.pin, em1.pin, speed2*1.5, speed2, 0.8);
    sm2.step -= 10;
    em1.step -= 8;
  }
  else if (joystickY < 450 && sm2.step >= 2230){
    digitalWrite(sm2.dirpin, LOW);
    digitalWrite(em1.dirpin, LOW);
    makeScaledStep3(sm2.pin, em1.pin, speed2*1.5, speed2, 1.2);
    sm2.step -= 10;
    em1.step -= 12;
  }
}
void upDown(){
  if (joystickY > 600 && upDownCount >= 0){
    digitalWrite(em1.dirpin, HIGH);
    digitalWrite(sm2.dirpin, HIGH);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step += 30;
    sm2.step += 6;
    upDownCount += 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      if ((hm1pos > 30 && hm1pos < 200) || hm1pos >= 200)
      {
        hm1pos -= 0.3;
        hm1.write(hm1pos);
        delayMicroseconds(10);
      }
    }
  }

  else if (joystickY > 600 && upDownCount <= 0){
    digitalWrite(em1.dirpin, HIGH);
    digitalWrite(sm2.dirpin, LOW);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step += 30;
    sm2.step -= 6;
    upDownCount += 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      if ((hm1pos > 30 && hm1pos < 200) || hm1pos >= 200)
      {
        hm1pos -= 0.3; 
        hm1.write(hm1pos);
        delayMicroseconds(10);
      }
    }
  }

  else if (joystickY < 450 && upDownCount >= 0){
    digitalWrite(em1.dirpin, LOW);
    digitalWrite(sm2.dirpin, LOW);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step -= 30;
    sm2.step -= 6;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      if ((hm1pos > 30 && hm1pos < 200) || hm1pos <= 200)
      {
        hm1pos += 0.3;
        hm1.write(hm1pos);
        delayMicroseconds(10);
      }
    }
  }

  else if (joystickY < 450 && upDownCount <= 0){
    digitalWrite(em1.dirpin, LOW);
    digitalWrite(sm2.dirpin, HIGH);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step -= 30;
    sm2.step += 6;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      if ((hm1pos > 30 && hm1pos < 200) || hm1pos <= 200)
      {
        hm1pos += 0.3;
        hm1.write(hm1pos);
        delayMicroseconds(10);
      }
    }
  }
}
void down(){
  if (joystickY < 450 && upDownCount >= 0){
    digitalWrite(em1.dirpin, LOW);
    digitalWrite(sm2.dirpin, LOW);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step -= 30;
    sm2.step -= 6;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      if (hm1pos > 30 && hm1pos < 200 || hm1pos <= 200)
      {
        hm1pos += 0.3;
        hm1.write(hm1pos);
        delayMicroseconds(10);
      }
    }
  }

  else if (joystickY < 450 && upDownCount <= 0){
    digitalWrite(em1.dirpin, LOW);
    digitalWrite(sm2.dirpin, HIGH);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step -= 30;
    sm2.step += 6;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      if ((hm1pos > 30 && hm1pos < 200) || hm1pos <= 200)
      {
        hm1pos += 0.3;
        hm1.write(hm1pos);
        delayMicroseconds(10);
      }
    }
  }
}

void maxReach(){
  if (em1.step <= 6670)
  {
    upDown();
  }
  else if (em1.step >= 6670)
  {
    down();
  }
}

void grip1()
{
  if (joystickY < 450)
  {
    grip.write(180);
  }

  else if (joystickY > 600)
  {
    grip.write(0);
  }
  else
  {
    grip.write(95);
  }
}

void hm1move()
{
  if (joystickY > 600)
  {
    if (hm1pos > 30 && hm1pos < 270 || hm1pos <= 30)
    {
      hm1pos += 1;
      hm1.write(hm1pos);
      delayMicroseconds(5000);
    }
  }
  else if (joystickY < 450)
  {
    if (hm1pos > 30 && hm1pos < 270 || hm1pos >= 270)
    {
      hm1pos -= 1;
      hm1.write(hm1pos);
      delayMicroseconds(5000);
    }
  }
}

void em2move()
{
  if (joystickX > 600)
  {
    if (em2.step < 2537){
      delayMicroseconds(100);
      digitalWrite(em2.dirpin, LOW);
      makeStep(em2.pin, 600);
      em2.step++;
    }
  }
  else if (joystickX < 450)
  {
    if (em2.step > -2537){
      delayMicroseconds(100);
      digitalWrite(em2.dirpin, HIGH);
      makeStep(em2.pin, 600);
      em2.step--;
      }
  }
}

void buttoncheck()
{
  modeFlag = digitalRead(mode_button);
  powerFlag = digitalRead(POWER_BUTTON);
  joystickFlag = digitalRead(joystick_button);
}

void setHomePos(int x, int y, int z, int a, int b)
{
  sm1.home = x;
  setHome(sm1.home, SM1Address);
  sm2.home = y;
  setHome(sm2.home, SM2Address);
  em1.home = z;
  setHome(em1.home, EM1Address);
  em2.home = a;
  setHome(em2.home, EM2Address);
  upDownCountHome = b;
  setHome(upDownCountHome, upDownCountAddress);
}

int ButtonRead(int button)
{
  count = 0;
  while (digitalRead(button))
  {
    delay(10);
    count++;
  }
  if (count > 100)
  {
    return 2;
  }
  else if (count > 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void setDrinkingPos(int b, int c, int d, int e, int f)
{
  sm1.home2 = b;
  setHome(sm1.home2, SM1Address2);

  sm2.home2 = c;
  setHome(sm2.home2, SM2Address2);

  em1.home2 = d;
  setHome(em1.home2, EM1Address2);

  em2.home2 = e;
  setHome(em2.home2, EM2Address2);

  upDownCountHome2 = f;
  setHome(upDownCountHome2, upDownCountAddress2);
}

void setHome(int home, int address)
{
  int x = home / 255;
  EEPROM.write(address, x);
  int y = home % 255;
  EEPROM.write(address + 1, y);
}

void leftRight()
{
  (joystickX > 850 || joystickX < 300) ? speed = 600: speed = 1200;
  if (joystickX > 600 && (sm1.step > -5123 && sm1.step < 5123) || sm1.step >= 5123)
  {
    digitalWrite(sm1.dirpin, HIGH);
    makeStep(sm1.pin, speed);
    sm1.step--;
    if (joystickY > 600)
    {
      makeSteps(sm1.pin, 900, 5);
      sm1.step -= 5;
    }
    else if (joystickY < 450)
    {
      makeSteps(sm1.pin, 900, 5);
      sm1.step -= 5;
    }
  }
  else if (joystickX < 450 && (sm1.step > -5123 && sm1.step < 5123) || sm1.step <= -5123)
  {
    digitalWrite(sm1.dirpin, LOW);
    makeStep(sm1.pin, speed);
    sm1.step++;
    if (joystickY < 450)
    {
      makeSteps(sm1.pin, 900, 5);
      sm1.step += 5;
    }
    if (joystickY > 600)
    {
      makeSteps(sm1.pin, 900, 5);
      sm1.step += 5;
    }
  }
}

void makeServoSteps(int motor, int speed, int correction, int direction)
{
  int i = 0;
  if (i < correction)
  {
    if (direction)
    {
      hm1pos -= 0.01;
    }
    else
    {
      hm1pos += 0.01;
    }
    hm1.write(hm1pos);
    i++;
  }
}

void makeServoSteps2(int motor, int speed, int correction, int direction)
{
  int i = 0;
  if (i < correction)
  {
    if (direction)
    {
      hm1pos += 0.01;
    }
    else
    {
      hm1pos += 0.01;
    }
    hm1.write(hm1pos);
    i++;
  }
}

void doorOpen()
{
  digitalWrite(em2.dirpin, HIGH);
  digitalWrite(em1.dirpin, LOW);
  digitalWrite(sm1.dirpin, HIGH);
  makeSteps3(em2.pin, em1.pin, sm1.pin, 600, 500);
  em1.step -= 500;
  em2.step -= 500;
  sm1.step -= 500;
}

void powerOff()
{
  while (led4 < 1){
    showLED(10, 10, 0);
    led4 += 1;
  }

  while (hm1pos < 200){
    hm1pos += 0.1;
    hm1.write(hm1pos);
    delayMicroseconds(5000); 
  }
  
  hm1.detach(); 
  grip.detach();

  if (sm1.step <= 0)
  {
    sm1.powerCorrection = abs(sm1.step - sm1.powerHome);
    digitalWrite(sm1.dirpin, LOW);
  }
  else if (sm1.step >= 0)
  {
    sm1.powerCorrection = abs(sm1.powerHome - sm1.step);
    digitalWrite(sm1.dirpin, HIGH);
  }

  if (em2.step <= 0)
  {
    em2.powerCorrection = abs(em2.step - em2.powerHome);
    digitalWrite(em2.dirpin, LOW);
  }
  else if (em2.step > 0)
  {
    em2.powerCorrection = abs(em2.powerHome - em2.step);
    digitalWrite(em2.dirpin, HIGH);
  }

  if (sm2.step <= 0)
  {
    sm2.powerCorrection = abs(sm2.step - sm2.powerHome);
    digitalWrite(sm2.dirpin, LOW);
  }
  else if (sm2.step > 0)
  {
    sm2.powerCorrection = abs(sm2.powerHome - sm2.step);
    digitalWrite(sm2.dirpin, LOW);
  }

  if (em1.step <= 0)
  {
    em1.powerCorrection = abs(em1.step - em1.powerHome);
    digitalWrite(em1.dirpin, LOW);
  }
  else if (em1.step > 0)
  {
    em1.powerCorrection = abs(em1.powerHome - em1.step);
    digitalWrite(em1.dirpin, LOW);
  }

  totalPower = max(max(sm1.powerCorrection, sm2.powerCorrection), max(em1.powerCorrection, em2.powerCorrection));

  while (offSm1Move < sm1.powerCorrection)
  {
    digitalWrite(sm1.pin, HIGH);
    delayMicroseconds(400);
    digitalWrite(sm1.pin, LOW);
    delayMicroseconds(400);
    offSm1Move++;
  }

  while (t < totalPower)
  {
    if (t < em2.powerCorrection)
      digitalWrite(em2.pin, HIGH);
    if (t < sm2.powerCorrection)
      digitalWrite(sm2.pin, HIGH);
    if (t < em1.powerCorrection)
      digitalWrite(em1.pin, HIGH);
    delayMicroseconds(400);
    if (t < em2.powerCorrection)
      digitalWrite(em2.pin, LOW);
    if (t < sm2.powerCorrection)
      digitalWrite(sm2.pin, LOW);
    if (t < em1.powerCorrection)
      digitalWrite(em1.pin, LOW);
    delayMicroseconds(400);
    t++;
  }

  sm1.step = 0;
  sm2.step = 0;
  em1.step = 0;
  em2.step = 0;
  upDownCount = 0;

  digitalWrite(relayPin, LOW);
  
  showLED(10, 0, 0);    
  timer = 0;
  sm2Count = 0;
  sm2Count2 = 0;
  accelerometer2 = 0; 
  em1Count = 0;
  hm1count = 0;
  servoCount = 0;
  led2 = 0;
  led3 = 0;
  mode =0; 
  delayCount = 0; 
}

void goToHome()
{
  if (sm1.step <= sm1.home)
  {
    sm1.homeCorrection = abs(sm1.step - sm1.home);
    digitalWrite(sm1.dirpin, LOW);
  }
  else if (sm1.step >= sm1.home)
  {
    sm1.homeCorrection = abs(sm1.home - sm1.step);
    digitalWrite(sm1.dirpin, HIGH);
  }

  if (em2.step <= em2.home)
  {
    em2.homeCorrection = abs(em2.step - em2.home);
    digitalWrite(em2.dirpin, LOW);
  }
  else if (em2.step > em2.home)
  {
    em2.homeCorrection = abs(em2.home - em2.step);
    digitalWrite(em2.dirpin, HIGH);
  }

  if (sm2.step <= sm2.home)
  {
    sm2.homeCorrection = abs(sm2.step - sm2.home);
    digitalWrite(sm2.dirpin, HIGH);
    servoDirection2 = 1;
    servoRequired2 = 1;
  }
  else if (sm2.step > sm2.home)
  {
    sm2.homeCorrection = abs(sm2.home - sm2.step);
    digitalWrite(sm2.dirpin, LOW);
    servoDirection2 = 0;
    servoRequired2 = 1;
  }

  if (em1.step <= em1.home)
  {
    em1.homeCorrection = abs(em1.step - em1.home);
    digitalWrite(em1.dirpin, HIGH);
    servoDirection = 1;
    servoRequired = 1;
  }
  else if (em1.step > em1.home)
  {
    em1.homeCorrection = abs(em1.home - em1.step);
    digitalWrite(em1.dirpin, LOW);
    servoDirection = 0;
    servoRequired = 1;
  }
  totalhomeCorrection = max(em1.homeCorrection, max(em2.homeCorrection, sm2.homeCorrection));
  while (w < totalhomeCorrection)
  {
    if (w < em2.homeCorrection)
      digitalWrite(em2.pin, HIGH);
    if (w < sm2.homeCorrection)
      digitalWrite(sm2.pin, HIGH);
    if (w < em1.homeCorrection)
      digitalWrite(em1.pin, HIGH);
    delayMicroseconds(400);
    if (w < em2.homeCorrection)
      digitalWrite(em2.pin, LOW);
    if (w < sm2.homeCorrection)
      digitalWrite(sm2.pin, LOW);
    if (w < em1.homeCorrection)
      digitalWrite(em1.pin, LOW);
    delayMicroseconds(400);
    if (w < em1.homeCorrection)
    {
      if (servoRequired)
      {
        if (servoDirection)
        {
          if (hm1Test < 2300){
            hm1Test -= 0.1;
            hm1.writeMicroseconds(hm1Test);
          }
        }
        else
        {
          if (hm1Test > 1000){
            hm1Test += 0.1;
            hm1.writeMicroseconds(hm1Test);
          }
        }
      }
    }
    if (w < sm2.homeCorrection)
    {
      if (servoRequired2)
      {
        if (servoDirection2)
        {
          if (hm1Test > 1000){
          hm1Test += 0.1;
          hm1.writeMicroseconds(hm1Test);
          }
        }
        else
        {
          if (hm1Test < 2300){
          hm1Test -= 0.1;
          hm1.writeMicroseconds(hm1Test);
          }
        }
      }
    }
    w++;
  }

  while (t < sm1.homeCorrection)
  {
    digitalWrite(sm1.pin, HIGH);
    delayMicroseconds(400);
    digitalWrite(sm1.pin, LOW);
    delayMicroseconds(400);
    t++;
  }
  sm1.step = sm1.home;
  sm2.step = sm2.home;
  em1.step = em1.home;
  em2.step = em2.home;
  upDownCount = upDownCountHome;
}

void goToDrink()
{
  int servoDirection, servoDirection2, servoRequired;
  if (sm1.step <= sm1.home2)
  {
    sm1.drinkCorrection = abs(sm1.step - sm1.home2);
    digitalWrite(sm1.dirpin, LOW);
  }
  else if (sm1.step >= sm1.home2)
  {
    sm1.drinkCorrection = abs(sm1.home2 - sm1.step);
    digitalWrite(sm1.dirpin, HIGH);
  }

  if (em2.step <= em2.home2)
  {
    em2.drinkCorrection = abs(em2.step - em2.home2);
    digitalWrite(em2.dirpin, LOW);
  }
  else if (em2.step > em2.home2)
  {
    em2.drinkCorrection = abs(em2.home2 - em2.step);
    digitalWrite(em2.dirpin, HIGH);
  }

  if (sm2.step <= sm2.home2)
  {
    sm2.drinkCorrection = abs(sm2.step - sm2.home2);
    digitalWrite(sm2.dirpin, HIGH);
    servoRequired = 1;
    servoDirection2 = 0;
  }
  else if (sm2.step > sm2.home2)
  {
    sm2.drinkCorrection = abs(sm2.home2 - sm2.step);
    digitalWrite(sm2.dirpin, LOW);
    servoRequired = 1;
    servoDirection2 = 1;
  }

  if (em1.step <= em1.home2)
  {
    em1.drinkCorrection = abs(em1.step - em1.home2);
    digitalWrite(em1.dirpin, HIGH);
    servoRequired = 1;
    servoDirection = 1;
  }
  else if (em1.step > em1.home2)
  {
    em1.drinkCorrection = abs(em1.home2 - em1.step);
    digitalWrite(em1.dirpin, LOW);
    servoRequired = 1;
    servoDirection = 0;
  }

  drinkTotalCorrection = max(sm2.drinkCorrection, max(em1.drinkCorrection, em2.drinkCorrection));

  while (w < drinkTotalCorrection)
  {
    if (w < em2.drinkCorrection)
      digitalWrite(em2.pin, HIGH);
    if (w < sm2.drinkCorrection)
      digitalWrite(sm2.pin, HIGH);
    if (w < em1.drinkCorrection)
      digitalWrite(em1.pin, HIGH);
    delayMicroseconds(400);
    if (w < em2.drinkCorrection)
      digitalWrite(em2.pin, LOW);
    if (w < sm2.drinkCorrection)
      digitalWrite(sm2.pin, LOW);
    if (w < em1.drinkCorrection)
      digitalWrite(em1.pin, LOW);
    delayMicroseconds(400);
    w++;
    if (w < em1.drinkCorrection)
    {
      if (servoRequired)
      {
        if (servoDirection)
        {
          if (hm1Test < 2300){
            hm1Test -= 0.1;
            hm1.writeMicroseconds(hm1Test);
          }
        }
        else
        {
          if (hm1Test > 1000){
            hm1Test += 0.1;
            hm1.writeMicroseconds(hm1Test);
          }
        }
      }
    }

    if (w < sm2.drinkCorrection)
    {
      if (servoRequired)
      {
        if (servoDirection2)
        {
          if (hm1Test > 1000){
            hm1Test -= 0.1;
            hm1.writeMicroseconds(hm1Test);
          }
        }
        else
        {
          if (hm1Test < 2300){
            hm1Test += 0.1;
            hm1.writeMicroseconds(hm1Test);
          }
        }
      }
    }
  }
  while (t < sm1.drinkCorrection)
  {
    makeStep(sm1.pin, 400); 
    t++;
  }
  sm1.step = sm1.home2; 
  sm2.step = sm2.home2;
  em1.step = em1.home2;
  em2.step = em2.home2;
  upDownCount = upDownCountHome2;
}

void powerOn(){ 
  digitalWrite(relayPin, HIGH);
    while (led3 < 1){
      showLED(10, 10, 0); 
      led3 += 1; 
    }
    while(servoCount < 1){
      // Serial.println(hm1pos); 
      hm1.attach(6); 
      grip.attach(7);
      hm1.write(hm1pos);
      servoCount += 1; 
    }
    while(delayCount < 1){
      delay(5000);
      delayCount += 1; 
    }
    while (sm2Count2 < 1){
      digitalWrite(sm2.dirpin, LOW);
      while (!digitalRead(prox)){
        makeStep(sm2.pin, 600); 
      }
      sm2Count2++;
    }
    digitalWrite(sm2.dirpin, HIGH);
    while (sm2Count < 3000)
    {
      makeStep(sm2.pin, 600); 
      sm2Count++;
    }

    while (timer < 1)
    {
      delay(2000);
      timer++;
    }
  
    digitalWrite(em1.dirpin, HIGH);
    while (em1Count < 3200)
    {
      if (em1Count < 3200){
        makeStep(em1.pin, 600); 
      }
      em1Count++;
    }     
    while (hm1count < 1800){
      if (hm1pos > 120){
        hm1pos -= 0.1; 
        hm1.write(hm1pos);
        delayMicroseconds(5000); 
      }
    hm1count ++; 
    }
    while (led2 < 1){
      showLED(0, 10, 0); 
      led2 += 1; 
    }
    v = 0;
    r = 0;
    t = 0;
    w = 0;
    led4 = 0;
    accelerometer = 0;
    offSm1Move = 0; 
}

void showLED(int a, int b, int c)
{
  pixels.setPixelColor(0, pixels.Color(a, b , c)); 
  pixels.show();
  pixels.setPixelColor(1, pixels.Color(a, b , c)); 
  pixels.show();
  pixels.setPixelColor(2, pixels.Color(a, b , c)); 
  pixels.show();
  pixels.setPixelColor(3, pixels.Color(a, b , c)); 
  pixels.show();
  pixels.setPixelColor(4, pixels.Color(a, b , c)); 
  pixels.show(); 
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * -----------------------------------------------------------------------------------------------------------LOOP------------------------------------------------------------------------------------------------------------------------
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void loop()
{
  homebut = ButtonRead(HOME_BUTTON);
  switch (homebut)
  {
  case 1:
    Serial.println("Going to Home Position");
    goToHome();
    break;
    break;
  case 2:
    Serial.println("Setting new Home");
    setHomePos(sm1.step, sm2.step, em1.step, em2.step, upDownCount);
    break;
  default:
    break;
  }

  drinkbut = ButtonRead(DRINK_BUTTON);
  switch (drinkbut)
  {
  case 1:
    Serial.println("Going to Home Position");
    goToDrink();
    break;
    break;
  case 2:
    Serial.println("Setting new Home");
    setDrinkingPos(sm1.step, sm2.step, em1.step, em2.step, upDownCount);
    break;
  default:
    break;
  }

  buttoncheck();

  joystickY = analogRead(A0);
  joystickX = analogRead(A1);
  if(joystickY > 750 || joystickY < 300){
    speed2 = 400;
  }
  else{
    speed2 = 800;
  }

  if (modeFlag)
  {
    delay(200);
    mode1 = digitalRead(mode_button);
    if (mode1 == modeFlag)
    {
      mode = !mode;
      modeFlag = 0;
    }
    else
    {
      modeFlag = 0;
    }
  }

  if (joystickFlag)
  {
    delay(150);
    joystick1 = digitalRead(joystick_button);
    if (joystick1 == joystickFlag)
    {
      joystick = !joystick;
      joystickFlag = 0;
    }
    else
    {
      joystickFlag = 0;
    }
  }

  if (powerFlag)
  {
    delay(100);
    power1 = digitalRead(POWER_BUTTON);
    if (power1 == powerFlag)
    {
      power = !power;
      powerFlag = 0;
    }
    else
    {
      powerFlag = 0;
    }
  }

  if (power){
    powerOn(); 
  }
  else
  {
    powerOff();
  }

  if (mode)
  {    
    upDownLed = 0; 
    fwdBkLed = 0; 
    if (joystick)
    {
      hm1Led = 0; 
      for (gripperLed; gripperLed < 1; gripperLed++)
      {
        showLED(0, 10, 10); 
      }
      grip1();
    }
    else
    {
      gripperLed = 0;
      while (hm1Led < 1){
        showLED(0, 0, 10); 
        hm1Led +=1; 
      } 
      em2move();
      grip.write(95);
      hm1move();
    }
  }

  else
  { 
    hm1Led = 0; 
    gripperLed = 0;    
    leftRight();
    if (joystick)
    {
      fwdBkLed = 0;
      for (upDownLed; upDownLed < 1; upDownLed++)
      {
        showLED(4, 10, 0); 
      }
      maxReach();
    }
    else
    {
      upDownLed = 0; 
      while (fwdBkLed < 1){
        showLED(0, 10, 0); 
        fwdBkLed +=1; 
      } 
      fwdBk();
    }
  }

  doorbut = ButtonRead(DOOR_BUTTON);
  if (doorbut){
    doorOpen();
  }

  Serial.println(hm1pos);
}
