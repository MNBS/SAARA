#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SparkFunLSM9DS1.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN            5
#define NUMPIXELS      16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

LSM9DS1 imu;

#define LSM9DS1_M  0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG  0x6B // Would be 0x6A if SDO_AG is LOW

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

#define HOME_BUTTON 41
#define DRINK_BUTTON 40
#define POWER_BUTTON 43

int relayPin = 27;

int dir;
float upDownCount;
int upDownCountHome, upDownCountHome2;
int em2step = 1171;
int em2home = 1171;

int totalPower;
int totalhomeCorrection;
int drinkTotalCorrection;

int hm1home;

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
int pb1 = 40;
int pb3 = 42;
int mode_button = 45;
int joystick_button = 44;

int redLed = 16;
int greenLed = 17;
int blueLed = 5;

int joystickX, joystickY; 
float hm1pos = 120;
float gripperpos = 90;

volatile int modeFlag = 0;
volatile int powerFlag = 0;
int mode1;
int mode = 0;
int power1;
int power = 0;
int speed, speed2; 
int timer = 0;
int drinkbut = 0;
int homebut = 0;
String steps = "steps";
String home = "home";
int count;
int accelerometer = 0;
int accelerometer2 = 0; 
volatile float y_accel, x_accel;
int sm2Count = 0;
int em1Count = 0;
int hm1count = 0;
int servoCount = 0;
int led = 0; 
int led1 = 0; 
int led2 = 0;
int led3 = 0;
int led4 = 0;
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
  
  imu.settings.device.commInterface = IMU_MODE_I2C;
  imu.settings.device.mAddress = LSM9DS1_M;
  imu.settings.device.agAddress = LSM9DS1_AG;

  if (!imu.begin())
  {
    Serial.println("Failed to communicate with LSM9DS1.");
    Serial.println("Double-check wiring.");
    Serial.println("Default settings in this sketch will " \
                  "work for an out of the box LSM9DS1 " \
                  "Breakout, but may need to be modified " \
                  "if the board jumpers are.");
    while (1)
      ;
  }
  sm2.dirpin = 25; // output of the stepper motors where dir is connected to pin 4 of uStepper and 8 is connected to pin 3
  sm2.pin = 23;
  sm1.dirpin = 9;
  sm1.pin = 8;
  em1.pin = 10;
  em1.dirpin = 11;
  em2.pin = 22;
  em2.dirpin = 24;

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
  sm2.powerHome = -1000;
  em1.powerHome = -2700;
  em2.powerHome = 0;

  upDownCountHome = (EEPROM.read(20) * 255) + EEPROM.read(21);
  upDownCountHome2 = (EEPROM.read(22) * 255) + EEPROM.read(23);

  pinMode(pb1, INPUT);
  pinMode(HOME_BUTTON, INPUT);
  pinMode(pb3, INPUT);
  pinMode(POWER_BUTTON, INPUT);
  pinMode(mode_button, INPUT);
  pinMode(joystick_button, INPUT);
  pinMode(sm1.dirpin, OUTPUT);
  pinMode(sm1.pin, OUTPUT);
  pinMode(sm2.dirpin, OUTPUT);
  pinMode(sm2.pin, OUTPUT);
  pinMode(em1.dirpin, OUTPUT);
  pinMode(em1.pin, OUTPUT);
  pinMode(em2.pin, OUTPUT);
  pinMode(em2.dirpin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(relayPin, OUTPUT);

  hm1.attach(6);
  grip.attach(7);
  // hm1.write(hm1pos);
  grip.write(gripperpos);
}

/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * -----------------------------------------------------------------------------------------------------------LOOP------------------------------------------------------------------------------------------------------------------------
 * ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 * --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

void loop()
{
  // imu.readAccel(); 
  // Serial.println(imu.calcAccel(imu.ax));

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

  joystickY = analogRead(A1);
  joystickX = analogRead(A0);
  if(joystickY > 850 || joystickY < 300){
    speed2 = 400;
  }
  else{
    speed2 = 800;
  }

  if (modeFlag)
  {
    delay(100);
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

  if (power)
  {
    digitalWrite(relayPin, HIGH);
    while (led3 < 1){
      pixels.setPixelColor(0, pixels.Color(10,10,0)); // Moderately bright green color.
      pixels.setPixelColor(1, pixels.Color(10,10,0)); // Moderately bright green color.
      pixels.setPixelColor(2, pixels.Color(10,10,0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      led3 += 1; 
    }
    while(servoCount < 1){ 
      hm1.attach(6); 
      grip.attach(7);
      servoCount += 1; 
    }
    // digitalWrite(sm2.dirpin, HIGH);
    // while (sm2Count < 4750)
    // {
    //   digitalWrite(sm2.pin, HIGH);
    //   delayMicroseconds(600);
    //   digitalWrite(sm2.pin, LOW);
    //   delayMicroseconds(600);
    //   sm2Count++;
    // }

  //   while (accelerometer < 5000){
  //   imu.readAccel();
  //   y_accel2 = imu.calcAccel(imu.ay);
  //   Serial.print("  y_accel: ");Serial.println(y_accel2); 
  //   while (y_accel2 > -0.2){
  //     digitalWrite(em1.dirpin, HIGH); 
  //     makeSteps(em1.pin, 400, 5);
  //     y_accel2 -= 1;
  //   }
  //   accelerometer++;
  // }
  while (accelerometer2 < 5000){
    imu.readAccel();
    y_accel = imu.calcAccel(imu.ay);
    Serial.print("  y_accel: ");Serial.println(y_accel); 
    if (y_accel > 0.25){
      digitalWrite(sm2.dirpin, LOW); 
      makeSteps(sm2.pin, 600, 5);
      // y_accel -= 1;
    }
    else if (y_accel < 0.25){
      
    }
    accelerometer2++;
  }
  
  if (accelerometer2 == 5000){ 
    digitalWrite(em1.dirpin, LOW);
    digitalWrite(sm2.dirpin, HIGH);
    while (em1Count < 3100)
    {
      if (em1Count < 1800) digitalWrite(sm2.pin, HIGH);
      if (em1Count < 3100) digitalWrite(em1.pin, HIGH);
      delayMicroseconds(600);
      if (em1Count < 1800) digitalWrite(sm2.pin, LOW);
      if (em1Count < 3100) digitalWrite(em1.pin, LOW);
      delayMicroseconds(600);
      em1Count++;
    }
  }     
    while (timer < 1)
    {
      delay(4000);
      timer++;
    }
    while (hm1count < 100){
      if (hm1pos > 70){
        hm1.write(hm1pos);
        hm1pos -= 0.5; 
        delay(25); 
        hm1count ++; 
      }
    }
    while (led2 < 1){
      pixels.setPixelColor(0, pixels.Color(0,10,0)); // Moderately bright green color.
      pixels.setPixelColor(1, pixels.Color(0,10,0)); // Moderately bright green color.
      pixels.setPixelColor(2, pixels.Color(0,10,0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      led2 += 1; 
    }
    v = 0;
    r = 0;
    t = 0;
    w = 0;
    led4 = 0;
    accelerometer = 0; 
  }
  else
  {
    powerOff();
    timer = 0;
    sm2Count = 0;
    accelerometer2 = 0; 
    em1Count = 0;
    hm1count = 0;
    servoCount = 0;
    led2 = 0;
    led3 = 0;
    mode =0; 
  }

  if (mode)
  {
    while (led < 1){
      pixels.setPixelColor(0, pixels.Color(0,0,10)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      pixels.setPixelColor(1, pixels.Color(0,0,10)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      pixels.setPixelColor(2, pixels.Color(0,0,10)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      led +=1; 
    } 
    led1 = 0; 
    em2move();
    if (digitalRead(joystick_button))
    {
      grip1();
    }
    else
    {
      grip.write(90);
      hm1move();
    }
  }

  else
  {
    while (led1 < 1){
      pixels.setPixelColor(0, pixels.Color(0,10,0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      pixels.setPixelColor(1, pixels.Color(0,10,0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      pixels.setPixelColor(2, pixels.Color(0,10,0)); // Moderately bright green color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      led1 +=1; 
    } 
    led = 0; 
    leftRight();
    if (digitalRead(joystick_button))
    {
      maxReach();
    }
    else
    {
      fwdBk();
    }
  }
  // // if (digitalRead(43)){
  //   doorOpen();
  // }
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
  int step1 = 10;
  int step2 = 10;
  step2 = step1 * scale;
  makeSteps(motor1, speed, step1);
  makeSteps(motor2, speed2, step2);
}

void fwdBk(){
  if (joystickY < 450 && sm2.step <= 1718 && em1.step <= 6670){
    digitalWrite(sm2.dirpin, HIGH);
    digitalWrite(em1.dirpin, LOW);
    makeScaledStep2(sm2.pin, em1.pin, speed2*2, speed2, 0.9);
    sm2.step += 10;
    em1.step += 9;
  }
  else if (joystickY < 450 && sm2.step >= 1718 && em1.step <= 6670){
    digitalWrite(sm2.dirpin, HIGH);
    digitalWrite(em1.dirpin, LOW);
    makeScaledStep2(sm2.pin, em1.pin, speed2*2, speed2, 1.2);
    sm2.step += 10;
    em1.step += 12;
  }
  else if (joystickY > 600 && sm2.step <= 1718){
    digitalWrite(sm2.dirpin, LOW);
    digitalWrite(em1.dirpin, HIGH);
    makeScaledStep2(sm2.pin, em1.pin, speed2*2, speed2, 0.9);
    sm2.step -= 10;
    em1.step -= 9;
  }
  else if (joystickY > 600 && sm2.step >= 1718){
    digitalWrite(sm2.dirpin, LOW);
    digitalWrite(em1.dirpin, HIGH);
    makeScaledStep2(sm2.pin, em1.pin, speed2*2, speed2, 1.2);
    sm2.step -= 10;
    em1.step -= 12;
  }
}
void upDown(){
  if (joystickY < 450 && upDownCount >= 0){
    digitalWrite(em1.dirpin, LOW);
    digitalWrite(sm2.dirpin, HIGH);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.4);
    em1.step += 10;
    sm2.step += 4;
    upDownCount += 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      hm1.write(hm1pos);
      hm1pos -= 0.10;
      // delayMicroseconds(15);
    }
  }

  else if (joystickY < 450 && upDownCount <= 0){
    digitalWrite(em1.dirpin, LOW);
    digitalWrite(sm2.dirpin, LOW);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step += 10;
    sm2.step -= 2;
    upDownCount += 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      hm1.write(hm1pos);
      hm1pos -= 0.12;
      // delayMicroseconds(15);
    }
  }

  else if (joystickY > 600 && upDownCount >= 0){
    digitalWrite(em1.dirpin, HIGH);
    digitalWrite(sm2.dirpin, LOW);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.4);
    em1.step -= 10;
    sm2.step -= 4;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      hm1.write(hm1pos);
      hm1pos += 0.10;
      // delayMicroseconds(15);
    }
  }

  else if (joystickY > 600 && upDownCount <= 0){
    digitalWrite(em1.dirpin, HIGH);
    digitalWrite(sm2.dirpin, HIGH);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step -= 10;
    sm2.step += 2;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      hm1.write(hm1pos);
      hm1pos += 0.12;
      // delayMicroseconds(15);
    }
  }
}
void down(){
  if (joystickY > 600 && upDownCount >= 0){
    digitalWrite(em1.dirpin, HIGH);
    digitalWrite(sm2.dirpin, LOW);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.4);
    em1.step -= 10;
    sm2.step -= 4;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      hm1.write(hm1pos);
      hm1pos += 0.10;
      // delayMicroseconds(15);
    }
  }

  else if (joystickY > 600 && upDownCount <= 0){
    digitalWrite(em1.dirpin, HIGH);
    digitalWrite(sm2.dirpin, HIGH);
    makeScaledStep2(em1.pin, sm2.pin, speed2, speed2*2, 0.2);
    em1.step -= 10;
    sm2.step += 2;
    upDownCount -= 5;
    if (em2.step > -1072 && em2.step < 1072)
    {
      hm1.write(hm1pos);
      hm1pos += 0.12;
      // delayMicroseconds(15);
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
  if (joystickY > 600)
  {
    grip.write(180);
  }

  else if (joystickY < 450)
  {
    grip.write(0);
  }
  else
  {
    grip.write(90);
  }
}

void hm1move()
{
  if (joystickY > 600)
  {
    if (hm1pos > 0 && hm1pos < 120 || hm1pos >= 120)
    {
      hm1pos -= 0.5;
      hm1.write(hm1pos);
      delay(10);
    }
  }
  else if (joystickY < 450)
  {
    if (hm1pos > 0 && hm1pos < 120 || hm1pos <= 0)
    {
      hm1pos += 0.5;
      hm1.write(hm1pos);
      delay(10);
    }
  }
}

void em2move()
{
  if (joystickX < 450)
  {
    digitalWrite(em2.dirpin, LOW);
    makeStep(em2.pin, 600);
    em2.step++;
  }
  else if (joystickX > 600)
  {
    digitalWrite(em2.dirpin, HIGH);
    makeStep(em2.pin, 600);
    em2.step--;
  }
}

void buttoncheck()
{
  modeFlag = digitalRead(45);
  powerFlag = digitalRead(POWER_BUTTON);
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
  (joystickX > 850 || joystickX < 300) ? speed = 500: speed = 1000;
  if (joystickX > 600)
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
  else if (joystickX < 450)
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
      hm1pos += 0.01;
    }
    else
    {
      hm1pos -= 0.01;
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
      hm1pos -= 0.01;
    }
    else
    {
      hm1pos -= 0.01;
    }
    hm1.write(hm1pos);
    i++;
  }
}

void doorOpen()
{
  digitalWrite(em2.dirpin, LOW);
  digitalWrite(em1.dirpin, HIGH);
  digitalWrite(sm1.dirpin, HIGH);
  makeSteps3(em2.pin, em1.pin, sm1.pin, 600, 500);
  em1.step -= 500;
  em2.step += 500;
  sm1.step -= 500;
}

void powerOff()
{
  while (led4 < 1){
  pixels.setPixelColor(0, pixels.Color(10,10,0)); // Moderately bright green color.
  pixels.setPixelColor(1, pixels.Color(10,10,0)); // Moderately bright green color.
  pixels.setPixelColor(2, pixels.Color(10,10,0)); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.
  led4 += 1;
  }

  while (hm1pos < 120){
    hm1.write(hm1pos);
    hm1pos += 0.5;
    delay(25); 
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
    digitalWrite(em1.dirpin, HIGH);
  }
  else if (em1.step > 0)
  {
    em1.powerCorrection = abs(em1.powerHome - em1.step);
    digitalWrite(em1.dirpin, HIGH);
  }

  totalPower = max(max(sm1.powerCorrection, sm2.powerCorrection), max(em1.powerCorrection, em2.powerCorrection));

  while (t < totalPower)
  {
    if (t < sm1.powerCorrection)
      digitalWrite(sm1.pin, HIGH);
    if (t < em2.powerCorrection)
      digitalWrite(em2.pin, HIGH);
    if (t < sm2.powerCorrection)
      digitalWrite(sm2.pin, HIGH);
    if (t < em1.powerCorrection)
      digitalWrite(em1.pin, HIGH);
    delayMicroseconds(400);
    if (t < sm1.powerCorrection)
      digitalWrite(sm1.pin, LOW);
    if (t < em2.powerCorrection)
      digitalWrite(em2.pin, LOW);
    if (t < sm2.powerCorrection)
      digitalWrite(sm2.pin, LOW);
    if (t < em1.powerCorrection)
      digitalWrite(em1.pin, LOW);
    delayMicroseconds(400);
    t++;
  }
  // while (accelerometer < 4000){
  //   imu.readAccel();
  //   x_accel = imu.calcAccel(imu.ax);
  //   Serial.print("  x_accel: ");Serial.println(x_accel); 
  //   while (x_accel > 0.23){
  //     digitalWrite(sm1.dirpin, HIGH); 
  //     makeSteps(sm1.pin, 600, 10);
  //     x_accel -= 1;
  //   }
    // while (x_accel < 0.1){
    //   digitalWrite(sm1.dirpin, LOW); 
    //   makeSteps(sm1.pin, 600, 5); 
    //   x_accel += 1;
    // }
  //   accelerometer++;
  // }


  sm1.step = 0;
  sm2.step = 0;
  em1.step = 0;
  em2.step = 0;
  upDownCount = 0;

  digitalWrite(relayPin, LOW);
  
  pixels.setPixelColor(0, pixels.Color(10,0,0)); // Moderately bright green color.
  pixels.setPixelColor(1, pixels.Color(10,0,0)); // Moderately bright green color.
  pixels.setPixelColor(2, pixels.Color(10,0,0)); // Moderately bright green color.
  pixels.show(); // This sends the updated pixel color to the hardware.

}

void goToHome()
{
  int servoDirection, servoDirection2, servoRequired;
  if (sm1.step <= sm1.home)
  {
    sm1.homeCorrection = abs(sm1.step - sm1.home);
    digitalWrite(sm1.dirpin, LOW);
    servoDirection = 0;
    servoDirection2 = 0;
    servoRequired = 0;
  }
  else if (sm1.step >= sm1.home)
  {
    sm1.homeCorrection = abs(sm1.home - sm1.step);
    digitalWrite(sm1.dirpin, HIGH);
    servoDirection = 0;
    servoDirection2 = 0;
    servoRequired = 0;
  }

  if (em2.step <= em2.home)
  {
    em2.homeCorrection = abs(em2.step - em2.home);
    digitalWrite(em2.dirpin, LOW);
    servoDirection = 0;
    servoDirection2 = 0;
    servoRequired = 0;
  }
  else if (em2.step > em2.home)
  {
    em2.homeCorrection = abs(em2.home - em2.step);
    digitalWrite(em2.dirpin, HIGH);
    servoDirection = 0;
    servoDirection2 = 0;
    servoRequired = 0;
  }

  if (sm2.step <= sm2.home)
  {
    sm2.homeCorrection = abs(sm2.step - sm2.home);
    digitalWrite(sm2.dirpin, HIGH);
    servoDirection = 0;
    servoDirection2 = 0;
    servoRequired = 1;
  }
  else if (sm2.step > sm2.home)
  {
    sm2.homeCorrection = abs(sm2.home - sm2.step);
    digitalWrite(sm2.dirpin, LOW);
    servoDirection = 0;
    servoDirection2 = 1;
    servoRequired = 1;
  }

  if (em1.step <= em1.home)
  {
    em1.homeCorrection = abs(em1.step - em1.home);
    digitalWrite(em1.dirpin, LOW);
    servoDirection = 1;
    servoDirection2 = 0;
    servoRequired = 1;
  }
  else if (em1.step > em1.home)
  {
    em1.homeCorrection = abs(em1.home - em1.step);
    digitalWrite(em1.dirpin, HIGH);
    servoDirection = 0;
    servoDirection2 = 0;
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
          hm1pos -= 0.015;
          hm1.write(hm1pos);
        }
        else
        {
          hm1pos += 0.015;
          hm1.write(hm1pos);
        }
      }
    }

    if (w < sm2.homeCorrection)
    {
      if (servoRequired)
      {
        if (servoDirection2)
        {
          hm1pos -= 0.012;
          hm1.write(hm1pos);
        }
        else
        {
          hm1pos -= 0.012;
          hm1.write(hm1pos);
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
    servoRequired = 0;
    servoDirection = 0;
    servoDirection2 = 0;
  }
  else if (sm1.step >= sm1.home2)
  {
    sm1.drinkCorrection = abs(sm1.home2 - sm1.step);
    digitalWrite(sm1.dirpin, HIGH);
    servoRequired = 0;
    servoDirection = 0;
    servoDirection2 = 0;
  }

  if (em2.step <= em2.home2)
  {
    em2.drinkCorrection = abs(em2.step - em2.home2);
    digitalWrite(em2.dirpin, LOW);
    servoRequired = 0;
    servoDirection = 0;
    servoDirection2 = 0;
  }
  else if (em2.step > em2.home2)
  {
    em2.drinkCorrection = abs(em2.home2 - em2.step);
    digitalWrite(em2.dirpin, HIGH);
    servoRequired = 0;
    servoDirection = 0;
    servoDirection2 = 0;
  }

  if (sm2.step <= sm2.home2)
  {
    sm2.drinkCorrection = abs(sm2.step - sm2.home2);
    digitalWrite(sm2.dirpin, HIGH);
    servoRequired = 1;
    servoDirection2 = 0;
    servoDirection = 0;
  }
  else if (sm2.step > sm2.home2)
  {
    sm2.drinkCorrection = abs(sm2.home2 - sm2.step);
    digitalWrite(sm2.dirpin, LOW);
    servoRequired = 0;
    servoDirection = 0;
    servoDirection2 = 1;
  }

  if (em1.step <= em1.home2)
  {
    em1.drinkCorrection = abs(em1.step - em1.home2);
    digitalWrite(em1.dirpin, LOW);
    servoRequired = 1;
    servoDirection = 1;
    servoDirection2 = 0;
  }
  else if (em1.step > em1.home2)
  {
    em1.drinkCorrection = abs(em1.home2 - em1.step);
    digitalWrite(em1.dirpin, HIGH);
    servoRequired = 1;
    servoDirection = 0;
    servoDirection2 = 0;
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
          hm1pos -= 0.014;
          hm1.write(hm1pos);
        }
        else
        {
          hm1pos += 0.014;
          hm1.write(hm1pos);
        }
      }
    }

    if (w < sm2.drinkCorrection)
    {
      if (servoRequired)
      {
        if (servoDirection2)
        {
          hm1pos += 0.014;
          hm1.write(hm1pos);
        }
        else
        {
          hm1pos -= 0.014;
          hm1.write(hm1pos);
        }
      }
    }
  }
  while (t < sm1.drinkCorrection)
  {
    digitalWrite(sm1.pin, HIGH);
    delayMicroseconds(400);
    digitalWrite(sm1.pin, LOW);
    delayMicroseconds(400);
    t++;
  }
  sm1.step = sm1.home2; 
  sm2.step = sm2.home2;
  em1.step = em1.home2;
  em2.step = em2.home2;
  upDownCount = upDownCountHome2;
}

