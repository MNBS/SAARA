#include <Servo.h>
#include <EEPROM.h> 

int modeState = 0; 
// Motor step Count 
int em1StepCount = 0; 
int sm2StepCount = 0; 
int sm1StepCount = 0; 
int em2StepCount = 0; 

// Wrist servo position track
int wristAngle = 1500; 

// Initialize Robot object which includes power on and off
class Robot{
    private: 
        // Pin variable 
        int relayPin = 12;  
        // Led Pins
        int redLed = 8; 
        int blueLed = 9; 
        int greenLed = 11;

    public: 
        int ledIsOn = 0; 

        int joystickX = A0; 
        int joystickY = A1;

        // Initialize button pin 
        int powerButton = 3;
        int modeButton = 21; 
        int joystickButton = 20;
        int homeButton = 18; 
        int drinkButton = 19; 
        int doorButton = 2; 

        // Motor pins
        int em1Dirpin = 5; 
        int em1StepPin = 4;
        int sm2Dirpin = 36; 
        int sm2StepPin = 37; 
        int sm1Dirpin = 45; 
        int sm1StepPin = 10;
        int em2Dirpin = 34; 
        int em2StepPin = 35; 

        // Position where motor is initially turn on
        int sm1PowerHome = 0; 
        int em2PowerHome = 0; 
        // powerButtonState variable to keep track whether of whether robot is on or off
        int powerButtonState = 0; 
        int modeButtonState = 0;
        int joystickButtonState = 0; 
        int homeButtonState = 0; 

        // While loop counter variable
        int sm2PowerCounter = 0; 
        int em1PowerCounter = 0; 

        // Servo pins
        Servo gripper, wrist; 

        // Assign pin whether it is an input or output
        void assignPinMode(){ 
            pinMode(relayPin, OUTPUT); 
            pinMode(sm2Dirpin, OUTPUT); 
            pinMode(sm2StepPin, OUTPUT); 
            pinMode(em1Dirpin, OUTPUT); 
            pinMode(em1StepPin, OUTPUT);
            pinMode(sm1Dirpin, OUTPUT);
            pinMode(sm1StepPin, OUTPUT); 
            pinMode(em2Dirpin, OUTPUT); 
            pinMode(em2StepPin, OUTPUT); 
            pinMode(redLed, OUTPUT); 
            pinMode(greenLed, OUTPUT); 
            pinMode(blueLed, OUTPUT); 
            pinMode(powerButton, INPUT); 
            pinMode(modeButton, INPUT);
            pinMode(joystickButton, INPUT); 
            pinMode(homeButton, INPUT); 
        }

        // Light up LED with values as brightness
        void lightLed(int redLedBrightness, int greenLedBrightness, int blueLedbrightness){ 
            analogWrite(redLed, redLedBrightness); 
            analogWrite(greenLed, greenLedBrightness); 
            analogWrite(blueLed, blueLedbrightness); 
        }

        // function to move the stepper motor 1 step where dir is HIGH or LOW and speed is how long will the delay be
        void make1Step(int dirPin, bool dir, int stepPin, int speed){ 
            digitalWrite(dirPin, dir); 
            digitalWrite(stepPin, HIGH); 
            delayMicroseconds(speed);
            digitalWrite(stepPin, LOW); 
            delayMicroseconds(speed); 
        }

        void twoMotorMake1Step(int dirPin, bool dir, int stepPin, int dirPin2, bool dir2, int stepPin2, int speed){ 
            digitalWrite(dirPin, dir); 
            digitalWrite(dirPin2, dir2); 
            digitalWrite(stepPin, HIGH);
            digitalWrite(stepPin2, HIGH); 
            delayMicroseconds(speed);
            digitalWrite(stepPin, LOW); 
            digitalWrite(stepPin2, LOW);
            delayMicroseconds(speed); 
        }
        // Function to turn the robot on or off
        void power(){ 
            // powerButtonState is from interrupt handler function, if powerButtonState is HIGH, will turn on the arm and if it's low, arm is power off
            if (powerButtonState){
                // Reset step count
                sm2StepCount = 0; 
                em1StepCount = 0; 
                sm1StepCount = 0; 
                em2StepCount = 0; 
                // Attach servo 
                gripper.attach(6); 
                wrist.attach(7);
                gripper.write(90);
                // Light up LED to yellow showing initialization
                lightLed(210, 255, 255); 
                digitalWrite(relayPin, HIGH); 
                // Move sm2 motor 3100 steps, delayMicroseconds is at 2000 as stepper will lose step if delay decreases
                while (sm2PowerCounter < 3100){
                    make1Step(sm2Dirpin, LOW, sm2StepPin, 2000);
                    sm2PowerCounter++; 
                }
                // Move em1 motor 2727 steps, em1 delayMicroseconds does not matter as it is using uStepper with a built in encoder 
                while (em1PowerCounter < 2727){ 
                    make1Step(em1Dirpin, HIGH, em1StepPin, 400);
                    em1PowerCounter++; 
                }
                while (wristAngle != 1200){
                    if (wristAngle < 1200){ 
                        wristAngle ++; 
                        wrist.writeMicroseconds(wristAngle); 
                        delayMicroseconds(3000);
                    }
                    else if(wristAngle > 1200){ 
                        wristAngle--;
                        wrist.writeMicroseconds(wristAngle); 
                        delayMicroseconds(3000);
                    }
                }
                lightLed(255, 210, 255); 
                // Update motor power on initial stage
                sm2StepCount += 2500; 
                em1StepCount += 2727; 
                ledIsOn = 1; 
            }
            // Turning of robot arm off
            else { 
                // Light up LED with yellow
                lightLed(210, 255, 255); 
                    if (wristAngle > 1200){ 
                        while (wristAngle > 1200){
                            wristAngle --; 
                            wrist.writeMicroseconds(wristAngle); 
                            delayMicroseconds(3000);
                        }
                    }
                    else if(wristAngle < 1200){ 
                        while (wristAngle < 1200){
                            wristAngle++;
                            wrist.writeMicroseconds(wristAngle); 
                            delayMicroseconds(3000); 
                        }
                    }
                // Storing the maximum steps it need to move which will be use in the while loop
                int maxStepToMove = max(max(abs(sm2StepCount), abs(em1StepCount)), max(abs(sm1StepCount), abs(em2StepCount))); 
                // Counter for powering off arm
                int counter = 0; 
                // Using the maximum steps as the limit for while loop
                while (counter < maxStepToMove){ 
                    // Move sm1 to power off position
                    if (sm1StepCount < sm1PowerHome){ 
                        if (counter < abs(sm1StepCount)){ 
                            make1Step(sm1Dirpin, HIGH, sm1StepPin, 400); 
                        }
                    }
                    else if(sm1StepCount > sm1PowerHome){
                        if (counter < sm1StepCount){ 
                            make1Step(sm1Dirpin, LOW, sm1StepPin, 400); 
                        }
                    }

                    // Move em2 to power off position
                    if (em2StepCount < em2PowerHome){ 
                        if (counter < abs(em2StepCount)){ 
                            make1Step(em2Dirpin, LOW, em2StepPin, 400); 
                        }
                    }
                    else if(em2StepCount > em2PowerHome){
                        if (counter < em2StepCount){ 
                            make1Step(em2Dirpin, HIGH, em2StepPin, 400); 
                        }
                    }

                    // Move sm2 to power off position
                    if (counter < sm2StepCount){ 
                        make1Step(sm2Dirpin, HIGH, sm2StepPin, 600); 
                    }

                    // Move em1 to power off position
                    if (counter < em1StepCount){ 
                        make1Step(em1Dirpin, LOW, em1StepPin, 400); 
                    }
                    counter++; 
                }

                // Detach servo so there will be no power supplying to it
                wrist.detach();
                gripper.detach();

                lightLed(250, 255, 255);
                //Turn the relay off
                digitalWrite(relayPin, LOW);

                // Reset counter and steps to 0
                sm2PowerCounter = 0; 
                em1PowerCounter = 0; 
                sm2StepCount = 0; 
                em1StepCount = 0; 
                sm1StepCount = 0; 
                em2StepCount = 0; 
                ledIsOn = 0; 
                // Light up red LED
            }
        }

        // Read Joystick value 
        void readJoystick(){ 
            joystickX = analogRead(A0); 
            joystickY = analogRead(A1);  
        }
};

/******************************************************************** Turn Robot Left and Right **********************************************************************/

class MoveLeftRight {
    private: 
        // Composition from Robot Class
        Robot saara; 
        
    public: 
        // Constructor for MoveLeftRight class
        MoveLeftRight(Robot object) : saara(object){}
        
        // Function to turn robot left and right
        void leftRight(){
            // Read joystick Value
            saara.readJoystick();
            if (saara.joystickY > 400 && saara.joystickY < 700){
                // Speed Control that is divided into 7 speed 
                if (saara.joystickX > 850){ 
                    saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 400);
                    sm1StepCount ++;
                }
                else if (saara.joystickX > 800){ 
                    saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 600);
                    sm1StepCount ++;
                }
                else if (saara.joystickX > 750){ 
                    saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 800);
                    sm1StepCount ++;
                }
                else if(saara.joystickX > 700){ 
                    saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 1000);
                    sm1StepCount ++;
                }
                else if (saara.joystickX > 650){ 
                    saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 1200);
                    sm1StepCount ++;
                }
                else if (saara.joystickX > 600){ 
                    saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 1400);
                    sm1StepCount ++;
                }
                else if (saara.joystickX > 550){ 
                    saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 1600);
                    sm1StepCount ++;
                }
                if (saara.joystickX < 150){ 
                    saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 400);
                    sm1StepCount --;
                }
                else if (saara.joystickX < 200){ 
                    saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 600);
                    sm1StepCount --;
                }
                else if (saara.joystickX < 250){ 
                    saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 800);
                    sm1StepCount --;
                }
                else if (saara.joystickX < 300){ 
                    saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 1000);
                    sm1StepCount --;
                }
                else if (saara.joystickX < 350){ 
                    saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 1200);
                    sm1StepCount --;
                }
                else if (saara.joystickX < 400){ 
                    saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 1400);
                    sm1StepCount --;
                }
                else if(saara.joystickX < 450){ 
                    saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 1600);
                    sm1StepCount --;
                }
            }
        }
}; 
/*************************************************************** Wrist movement ***************************************************************************************/
class Wrist{ 
    private: 
    // Composition from saara
        Robot saara; 
    public: 
        Wrist(Robot object) : saara(object) {}

        // Control left and right wrist movement
        void em2Turn(){ 
            // Read joystick value 
            saara.readJoystick(); 

            if (saara.joystickX > 600){ 
                saara.make1Step(saara.em2Dirpin, LOW, saara.em2StepPin, 1000); 
                em2StepCount ++; 
            }
            else if (saara.joystickX < 450){ 
                saara.make1Step(saara.em2Dirpin, HIGH, saara.em2StepPin, 1000); 
                em2StepCount --; 
            }
        }

        // Controlling of wrist servo for up down wrist movement where minimum is 700 and max is 2300, 2300 is in upwards position and 700 is pointing down
        void wristUpDown(){
            // Reading of joystick value
            saara.readJoystick(); 

            // Move wrist upwards
            if (saara.joystickY > 600 && wristAngle < 2290){ 
                wristAngle ++; 
                saara.wrist.writeMicroseconds(wristAngle);
                delayMicroseconds(2000);  
            }
            // Move wrist downwards
            else if (saara.joystickY < 450 && wristAngle > 750){
                wristAngle --;  
                saara.wrist.writeMicroseconds(wristAngle); 
                delayMicroseconds(2000); 
            }
        }
};
/************************************************************** Forward Backward *************************************************************************************/ 

class ForwardBackward{ 
    private: 
    // Composition from Robot class and class variable
        Robot saara; 

    public: 
        // Initialize counter variable to count how many steps
        int fwdStepsCounter = 0;

        int speed = 0;

        ForwardBackward(Robot object) : saara(object){}

        // Moving robot Forward and backward where joystickY value > 550 means moving forward and sm2stepCount midpoint is 4953 
        // SpeedControl function will set the speed value
        void fwdBkw(){ 
            saara.readJoystick();
            if (saara.joystickX > 400 && saara.joystickX < 700){ 
                if (saara.joystickY > 550 && sm2StepCount <= 4953 && sm2StepCount < 9411){ 
                    speed = speedControl(); 
                    parallelMovement(saara.sm2StepPin, saara.em1StepPin, LOW, HIGH, 4, 5, 1, 0, speed, 0, 0, LOW, LOW);
                }
                else if(saara.joystickY > 550 && sm2StepCount < 9411){ 
                    speed = speedControl(); 
                    parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, HIGH, 5, 7, 0, 1, speed, 0, 0, LOW,LOW); 
                }
                else if(saara.joystickY < 450 && sm2StepCount <= 4953) { 
                    speed = speedControl(); 
                    parallelMovement(saara.sm2StepPin, saara.em1StepPin, HIGH, LOW, 4, 5, 1, 1, speed, 0, 0, LOW, LOW);
                }
                else if (saara.joystickY < 450){
                    speed = speedControl(); 
                    parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, LOW, 5, 7, 0, 0, speed, 0, 0, LOW, LOW);                 
                }
            }
            else { 
                // Diagonal top left
                if (saara.joystickX > 700 && saara.joystickY > 600 && sm2StepCount <= 4953){ 
                    speed = speedControl();
                    parallelMovement(saara.sm2StepPin, saara.em1StepPin, LOW, HIGH, 4, 5, 1, 0, speed, 0, 1, HIGH, HIGH);
                }
                else if(saara.joystickX > 700 && saara.joystickY > 600){ 
                    speed = speedControl(); 
                    parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, HIGH, 5, 7, 0, 1, speed, 0, 1, HIGH,HIGH); 
                }
                // Diagonal Top Right
                else if(saara.joystickX < 400  && saara.joystickY > 600 && sm2StepCount <= 4953){ 
                    speed = speedControl();
                    parallelMovement(saara.sm2StepPin, saara.em1StepPin, LOW, HIGH, 4, 5, 1, 0, speed, 0, 1, LOW, LOW);
                }
                else if(saara.joystickX < 400 && saara.joystickY > 600){
                    speed = speedControl(); 
                    parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, HIGH, 5, 7, 0, 1, speed, 0, 1, LOW, LOW); 
                }
                //Diagonal bottom right
                else if (saara.joystickX < 400 && saara.joystickY < 400 && sm2StepCount <= 4953) { 
                    speed = speedControl(); 
                    parallelMovement(saara.sm2StepPin, saara.em1StepPin, HIGH, LOW, 4, 5, 1, 1, speed, 0, 1, HIGH, HIGH);
                }
                else if (saara.joystickX < 400 && saara.joystickY < 400) {
                    speed = speedControl(); 
                    parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, LOW, 5, 7, 0, 0, speed, 0, 1, HIGH, HIGH);                 
                }
                // Diagonal bottom left
                else if (saara.joystickX > 700 && saara.joystickY < 400 && sm2StepCount <= 4953) { 
                    speed = speedControl(); 
                    parallelMovement(saara.sm2StepPin, saara.em1StepPin, HIGH, LOW, 4, 5, 1, 1, speed, 0, 1, LOW, LOW);
                }
                else if (saara.joystickX > 700 && saara.joystickY < 400) {
                    speed = speedControl(); 
                    parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, LOW, 5, 7, 0, 0, speed, 0, 1, LOW, LOW);                 
                }
            }
        }

        /* parallelMovement function is to scale the movement of robot arm so em1 is always parallel to the ground when moving
        StepPin is which link is the one that is not scaled and lessStepPin is the one that is scaled
        dir is link to stepPin while, dir2 is link to lessStepPin
        stesValue is the point in which the link that is moving in scale value will stop while reset value will reset the counter to 0
        value is to set the stepcount between em1 and sm2 where value == 1 will calculate em1StepCount and 0 will calculate sm2StepCount
        Negative is to see whether negative is applicable where 1 is where sm2 or em1 step count will - 
        Speed is to control the delay Microseconds */

        void parallelMovement(int stepPin, int lessStepPin, bool dir, bool dir2, int stepsValue, int resetValue, int value, int negative, int speed, int upDownActivate, int diagonalActivate, bool dir3, bool dir3PlusOrMinus){ 
            digitalWrite(saara.sm2Dirpin, dir); 
            digitalWrite(saara.em1Dirpin, dir2);
            if (diagonalActivate){
                digitalWrite(saara.sm1Dirpin, dir3);
                digitalWrite(saara.sm1StepPin, HIGH); 
                if (dir3PlusOrMinus){ 
                    sm1StepCount++;
                }
                else{
                    sm1StepCount--; 
                }
            }
            digitalWrite(stepPin, HIGH);
            // Move the motor to scale
            if (fwdStepsCounter <= stepsValue) { 
                digitalWrite(lessStepPin, HIGH); 
                if(value == 0 && negative == 1 && upDownActivate == 1){ 
                    sm2StepCount ++; 
                }
                else if (value == 0 && negative == 0 && upDownActivate == 2){ 
                    sm2StepCount --; 
                }
                else if (value == 1 && negative == 0){ 
                    em1StepCount ++;
                } 
                else if (value == 1){ 
                    em1StepCount --; 
                }
                else if (value == 0 && negative == 0){ 
                    sm2StepCount --; 
                }
                else{ 
                    sm2StepCount ++; 
                }
            }

            // Reset the value
            if (fwdStepsCounter >= resetValue){
                fwdStepsCounter = 0;  
            }
            // For UpDown function otherwise skip this
            if (upDownActivate == 1){
                if (fwdStepsCounter % 7 == 0 && wristAngle < 2250){
                    wristAngle ++;  
                    saara.wrist.write(wristAngle); 
                }
            }
            else if (upDownActivate == 2){ 
                if (fwdStepsCounter % 7 == 0 && wristAngle > 710){
                    wristAngle --;  
                    saara.wrist.write(wristAngle); 
                }
            }
            delayMicroseconds(speed);
            digitalWrite(saara.sm2StepPin, LOW); 
            digitalWrite(saara.em1StepPin, LOW);
            digitalWrite(saara.sm1StepPin, LOW); 
            delayMicroseconds(speed); 
            fwdStepsCounter ++; 
            if(value == 0 && negative == 0 && upDownActivate == 2){ 
                em1StepCount++;
            }
            else if(value == 0 and negative == 1 && upDownActivate == 1){ 
                em1StepCount --; 
            }
            else if (value == 1 && negative == 0){ 
                sm2StepCount ++;
            } 
            else if (value == 1){ 
                sm2StepCount --; 
            }
            else if (value == 0 && negative == 0){ 
                em1StepCount --; 
            }
            else{ 
                em1StepCount ++; 
            }
        }
        
        // Control the speed of the forward and backward movement
        int speedControl(){ 
            saara.readJoystick(); 
            if (saara.joystickY > 850){
                return speed = 400; 
            }
            else if (saara.joystickY > 800){ 
                return speed = 600; 
            }
            else if (saara.joystickY > 750){ 
                return speed = 800; 
            }
            else if (saara.joystickY > 700){ 
                return speed = 1000; 
            }
            else if (saara.joystickY > 650) {
                return speed = 1200; 
            }
            else if (saara.joystickY > 600) {
                return speed = 1400; 
            }
            else if (saara.joystickY > 550){ 
                return speed = 1600;
            }

            if (saara.joystickY < 150){ 
                return speed = 400; 
            }
            else if (saara.joystickY < 200){ 
                return speed = 600; 
            }
            else if (saara.joystickY < 250){ 
                return speed = 800; 
            }
            else if (saara.joystickY < 300){ 
                return speed = 1000; 
            }
            else if (saara.joystickY < 350) {
                return speed = 1200; 
            }
            else if (saara.joystickY < 400) {
                return speed = 1400; 
            }
            else if (saara.joystickY < 450) { 
                return speed = 1600;
            }
        }
};

/*************************************************************** Up Down ********************************************************************************************/ 
class UpDown{ 
    private: 
        Robot saara;
        ForwardBackward fwdBkw;
        int speed; 
    public: 
        UpDown(Robot object, ForwardBackward object1) : saara(object), fwdBkw(object1){}
        // Control the up and down movement of the arm
        void upDownMovement(){ 
            if (saara.ledIsOn){
                // saara.lightLed(255, 255, 210); 
            }
            saara.readJoystick(); 
            if (saara.joystickX > 400 && saara.joystickX < 700){
                // Both Motor moving up, sm2StepCount add em1StepCount add
                if (saara.joystickY > 550 && em1StepCount >= 2727){ 
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, HIGH, 5, 35, 0, 1, speed, 2, 0, LOW, LOW);         
                }
                // Em1 moving up, sm2 moving down, sm2stepCount minus and em1StepCount add
                else if(saara.joystickY > 550){ 
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, HIGH, 3, 35, 0, 0, speed, 2, 0, LOW, LOW); 
                }
                // Both motor moving down, sm2StepCount and em1StepCount minus
                else if(saara.joystickY < 450 && em1StepCount >= 2727){
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, LOW, 5, 35, 0, 0, speed, 1, 0, LOW, LOW);  
                }
                // Em1 moving down and sm2 moving up , sm2stepCount minus and em1StepCount plus 
                else if(saara.joystickY < 450){ 
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, LOW, 3, 35, 0, 1, speed, 1, 0, LOW, LOW);                            
                }
            }
            else { 
                // Diagonal top left
                if (saara.joystickX > 700 && saara.joystickY > 600 && em1StepCount >= 2727){ 
                    speed = fwdBkw.speedControl();
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, HIGH, 5, 35, 0, 1, speed, 2, 1, HIGH, HIGH);         
                }
                else if(saara.joystickX > 700 && saara.joystickY > 600){ 
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, HIGH, 3, 35, 0, 0, speed, 2, 1, HIGH, HIGH); 
                }
                // Diagonal Top Right
                else if(saara.joystickX < 400  && saara.joystickY > 600 && em1StepCount >= 2727){ 
                    speed = fwdBkw.speedControl();
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, HIGH, 5, 35, 0, 1, speed, 2, 1, LOW, LOW);         
                }
                else if(saara.joystickX < 400 && saara.joystickY > 600){
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, HIGH, 3, 35, 0, 0, speed, 2, 1, LOW, LOW); 
                }
                //Diagonal bottom right
                else if (saara.joystickX < 400 && saara.joystickY < 400 && sm2StepCount >= 2727) { 
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, LOW, 5, 35, 0, 0, speed, 1, 1, HIGH, HIGH);  
                }
                else if (saara.joystickX < 400 && saara.joystickY < 400) {
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, LOW, 3, 35, 0, 1, speed, 1, 1, HIGH, HIGH);                            
                }
                // Diagonal bottom left
                else if (saara.joystickX > 700 && saara.joystickY < 400 && sm2StepCount >= 2727) { 
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, LOW, 5, 35, 0, 0, speed, 1, 1, LOW, LOW);  
                }
                else if (saara.joystickX > 700 && saara.joystickY < 400) {
                    speed = fwdBkw.speedControl(); 
                    fwdBkw.parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, LOW, 3, 35, 0, 1, speed, 1, 1, LOW, LOW);                            
                }
            }
        }
};


/**************************************************************** Gripper ********************************************************************************************/ 

class Gripper{
    private: 
    // Composition from saara class
        Robot saara; 
    public: 
    // Constructor for gripper
        Gripper(Robot object) : saara(object){}
        // Open and close the gripper where push forward will open the gripper and backward will close
        void openClose(){ 
            saara.readJoystick(); 
            if (saara.joystickY > 550){ 
                saara.gripper.write(0); 
            }
            else if (saara.joystickY < 500){ 
                saara.gripper.write(180);
            }
            else{ 
                saara.gripper.write(90); 
            }
        }
};


/****************************************************************** Custom Positions ******************************************************************************************/
class Home { 
    private: 
        // Composition from Robot class
        Robot saara; 

        // Counter variable to keep count of the duration of button pressed
        int counter = 0; 
        
        // Home, drink amd door starting address in memory
        int sm1HomeAddress = 0; 
        int sm2HomeAddress = 2;
        int em1HomeAddress = 4; 
        int em2HomeAddress = 6; 

        int sm1DrinkAddress = 8; 
        int sm2DrinkAddress = 10;
        int em1DrinkAddress = 12; 
        int em2DrinkAddress = 14;

        int sm1DoorAddress = 16; 
        int sm2DoorAddress = 18; 
        int em1DoorAddress = 20; 
        int em2DoorAddress = 22;

        // HomeCount as a calculation variable
        int sm1HomeCount = 0; 
        int sm2HomeCount = 0; 
        int em1HomeCount = 0; 
        int em2HomeCount = 0; 
        int totalHomeCount = 0; 

        // Function to check the LED state to light up the different colours
        void checkLedState(){ 
            if (!modeState){ 
                if (!modeButtonState){ 
                    saara.lightLed(255, 210, 255); 
                }
                else { 
                    saara.lightLed(255, 255, 210);
                }
            }
            else{
                if (!joystickButtonState){ 
                    saara.lightLed(200, 255, 200);
                }
                else { 
                    saara.lightLed(0, 210, 255);
                }
            }
        }

        void blinkLed() { 
            saara.lightLed(210, 255, 255); 
            delay(500); 
            saara.lightLed(255, 255, 255); 
            delay(500);
            saara.lightLed(210, 255, 255); 
        }

    public:
        // Store LED state 
        int joystickButtonState = 0; 
        int modeButtonState = 0; 

        // Store home value into addresses (EEPROM)
        int sm1Home = (EEPROM.read(0) * 255) + EEPROM.read(1);
        int sm2Home = (EEPROM.read(2) * 255) + EEPROM.read(3);
        int em1Home = (EEPROM.read(4) * 255) + EEPROM.read(5);
        int em2Home = (EEPROM.read(6) * 255) + EEPROM.read(7);
        
        int sm1DrinkHome = (EEPROM.read(8) * 255) + EEPROM.read(9);
        int sm2DrinkHome = (EEPROM.read(10) * 255) + EEPROM.read(11);
        int em1DrinkHome = (EEPROM.read(12) * 255) + EEPROM.read(13);
        int em2DrinkHome = (EEPROM.read(14) * 255) + EEPROM.read(15);

        int sm1DoorHome = (EEPROM.read(16) * 255) + EEPROM.read(17);
        int sm2DoorHome = (EEPROM.read(18) * 255) + EEPROM.read(19);
        int em1DoorHome = (EEPROM.read(20) * 255) + EEPROM.read(21);
        int em2DoorHome = (EEPROM.read(22) * 255) + EEPROM.read(23);


        // Constructor for Home class
        Home(Robot object) : saara(object){}
        // Go to first custom home position
        void goToHome(){
            while (digitalRead(saara.homeButton)){ 
                Serial.println(counter);
                counter++; 
                if (counter > 500){ 
                    blinkLed(); 
                }
            }
            // If counter is more than 500 set as a new home
            if (counter > 500){ 
                sm1Home = sm1StepCount; 
                sm2Home = sm2StepCount; 
                em1Home = em1StepCount;
                em2Home = em2StepCount; 
                setHome(sm1Home, sm1HomeAddress); 
                setHome(sm2Home, sm2HomeAddress); 
                setHome(em1Home, em1HomeAddress); 
                setHome(em2Home, em2HomeAddress);
                checkLedState();
            }
            else{ 
                saara.lightLed(210, 255, 255); 
                // Do the calculation between home and the current tep count and set the direction to move
                if (sm2StepCount > sm2Home) { 
                    sm2HomeCount = sm2StepCount - sm2Home; 
                    digitalWrite(saara.sm2Dirpin, HIGH); 
                }
                else{ 
                    sm2HomeCount = sm2Home - sm2StepCount; 
                    digitalWrite(saara.sm2Dirpin, LOW); 
                }
                if (em1StepCount > em1Home){
                    em1HomeCount = em1StepCount - em1Home; 
                    digitalWrite(saara.em1Dirpin, LOW); 
                }
                else { 
                    em1HomeCount = em1Home - em1StepCount; 
                    digitalWrite(saara.em1Dirpin, HIGH); 
                }
                if (em2StepCount > em2Home){ 
                    em2HomeCount = em2StepCount - em2Home; 
                    digitalWrite(saara.em2Dirpin, HIGH);  
                }
                else { 
                    em2HomeCount = em2Home - em2StepCount; 
                    digitalWrite(saara.em2Dirpin, LOW);
                }
                totalHomeCount = max(max(abs(sm2HomeCount), abs(em1HomeCount)), em2HomeCount); 
                // For loop to move robot back to home position
                for (int i = 0; i < totalHomeCount; i++){ 
                    if (i < sm2HomeCount){ 
                        digitalWrite(saara.sm2StepPin, HIGH);
                        if (i % 7 == 0){
                            if (digitalRead(saara.sm2Dirpin) == 1 && wristAngle > 710) { 
                                wristAngle--; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                            else if (wristAngle <= 2250){ 
                                wristAngle ++; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                        }
                    }
                    if (i < em1HomeCount){ 
                        digitalWrite(saara.em1StepPin, HIGH); 
                        if (i % 6 == 0){
                            if (digitalRead(saara.em1Dirpin) == 1 && wristAngle > 710) { 
                                wristAngle--; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                            else if (wristAngle <= 2250){ 
                                wristAngle ++; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                        }
                    } 
                    if (i < em2HomeCount){ 
                        digitalWrite(saara.em2StepPin, HIGH); 
                    }
                    delayMicroseconds(1000);
                    digitalWrite(saara.sm2StepPin, LOW); 
                    digitalWrite(saara.em1StepPin, LOW); 
                    digitalWrite(saara.em2StepPin, LOW); 
                    delayMicroseconds(1000); 
                }
                if (sm1StepCount > sm1Home){ 
                    sm1HomeCount = sm1StepCount - sm1Home; 
                    for (int i = 0; i < abs(sm1HomeCount); i++){ 
                        saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 400); 
                    }
                }
                else {
                    sm1HomeCount = sm1Home - sm1StepCount;
                    for (int i = 0; i < abs(sm1HomeCount); i++){ 
                        saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 400); 
                    }
                }
                // Set the step count to the home 
                sm1StepCount = sm1Home;
                sm2StepCount = sm2Home; 
                em1StepCount = em1Home; 
                em2StepCount = em2Home; 
            }
            checkLedState(); 
            counter = 0; 
        }

        // Move to second custom home position
        void goToDrink(){
            while (digitalRead(saara.drinkButton)){ 
                Serial.println(counter);
                counter++; 
                if (counter > 500){ 
                    blinkLed(); 
                }
            }
            // If counter is more than 500 set current step count to home value
            if (counter > 500){ 
                sm1DrinkHome = sm1StepCount; 
                sm2DrinkHome = sm2StepCount; 
                em1DrinkHome = em1StepCount;
                em2DrinkHome = em2StepCount; 
                setHome(sm1DrinkHome, sm1DrinkAddress); 
                setHome(sm2DrinkHome, sm2DrinkAddress); 
                setHome(em1DrinkHome, em1DrinkAddress); 
                setHome(em2DrinkHome, em2DrinkAddress);
                checkLedState(); 
            }
            else{ 
                saara.lightLed(210, 255, 255); 
                // Do the calculation to calculate distance between home and the current step count
                if (sm2StepCount > sm2DrinkHome) { 
                    sm2HomeCount = sm2StepCount - sm2DrinkHome; 
                    digitalWrite(saara.sm2Dirpin, HIGH); 
                }
                else{ 
                    sm2HomeCount = sm2DrinkHome - sm2StepCount; 
                    digitalWrite(saara.sm2Dirpin, LOW); 
                }
                if (em1StepCount > em1DrinkHome){
                    em1HomeCount = em1StepCount - em1DrinkHome; 
                    digitalWrite(saara.em1Dirpin, LOW); 
                }
                else { 
                    em1HomeCount = em1DrinkHome - em1StepCount; 
                    digitalWrite(saara.em1Dirpin, HIGH); 
                }
                if (em2StepCount > em2DrinkHome){ 
                    em2HomeCount = em2StepCount - em2DrinkHome; 
                    digitalWrite(saara.em2Dirpin, HIGH); 
                }
                else { 
                    em2HomeCount = em2DrinkHome - em2StepCount; 
                    digitalWrite(saara.em2Dirpin, LOW);
                }
                totalHomeCount = max(max(abs(sm2HomeCount), abs(em1HomeCount)), em2HomeCount); 
                // For loop to move arm to the home position
                for (int i = 0; i < totalHomeCount; i++){ 
                    if (i < sm2HomeCount){ 
                        digitalWrite(saara.sm2StepPin, HIGH);
                        // To move servo parallel to where it is last at
                        if (i % 7 == 0){
                            if (digitalRead(saara.sm2Dirpin) == 1 && wristAngle > 700) { 
                                wristAngle--; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                            else if (wristAngle <= 2250) { 
                                wristAngle ++; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                        }
                    }
                    if (i < em1HomeCount){ 
                        digitalWrite(saara.em1StepPin, HIGH); 
                        if (i % 6 == 0){
                            if (digitalRead(saara.em1Dirpin) == 1 && wristAngle > 700) { 
                                wristAngle--; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                            else if (wristAngle <= 2250) { 
                                wristAngle ++; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                        }
                    } 
                    if (i < em2HomeCount){ 
                        digitalWrite(saara.em2StepPin, HIGH); 
                    }
                    delayMicroseconds(1000);
                    digitalWrite(saara.sm2StepPin, LOW); 
                    digitalWrite(saara.em1StepPin, LOW); 
                    digitalWrite(saara.em2StepPin, LOW); 
                    delayMicroseconds(1000); 
                }
                if (sm1StepCount > sm1DrinkHome){ 
                    sm1HomeCount = sm1StepCount - sm1DrinkHome; 
                    for (int i = 0; i < abs(sm1HomeCount); i++){ 
                        saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 400); 
                    }
                }
                else {
                    sm1HomeCount = sm1DrinkHome - sm1StepCount;
                    for (int i = 0; i < abs(sm1HomeCount); i++){ 
                        saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 400); 
                    }
                }
                // Set step count to be the same as drink
                sm1StepCount = sm1DrinkHome;
                sm2StepCount = sm2DrinkHome; 
                em1StepCount = em1DrinkHome; 
                em2StepCount = em2DrinkHome; 
            }

            checkLedState(); 
            counter = 0; 
        }

        // Custom position 3 function
        void goToDoor(){
            while (digitalRead(saara.doorButton)){ 
                Serial.println(counter);
                counter++; 
                if (counter > 500){ 
                    blinkLed();
                } 
            }
            // If counter is more than 500 set step count to door home 
            if (counter > 500){ 
                sm1DoorHome = sm1StepCount; 
                sm2DoorHome = sm2StepCount; 
                em1DoorHome = em1StepCount;
                em2DoorHome = em2StepCount; 
                setHome(sm1DoorHome, sm1DoorAddress); 
                setHome(sm2DoorHome, sm2DoorAddress); 
                setHome(em1DoorHome, em1DoorAddress); 
                setHome(em2DoorHome, em2DoorAddress);
                checkLedState(); 
            }
            else{ 
                saara.lightLed(210, 255, 255); 
                // Calculation for the difference in home and current step count
                if (sm2StepCount > sm2DoorHome) { 
                    sm2HomeCount = sm2StepCount - sm2DoorHome; 
                    digitalWrite(saara.sm2Dirpin, HIGH); 
                }
                else{ 
                    sm2HomeCount = sm2DoorHome - sm2StepCount; 
                    digitalWrite(saara.sm2Dirpin, LOW); 
                }
                if (em1StepCount > em1DoorHome){
                    em1HomeCount = em1StepCount - em1DoorHome; 
                    digitalWrite(saara.em1Dirpin, LOW); 
                }
                else { 
                    em1HomeCount = em1DoorHome - em1StepCount; 
                    digitalWrite(saara.em1Dirpin, HIGH); 
                }
                if (em2StepCount > em2DoorHome){ 
                    em2HomeCount = em2StepCount - em2DoorHome; 
                    digitalWrite(saara.em2Dirpin, HIGH); 
                }
                else { 
                    em2HomeCount = em2DoorHome - em2StepCount; 
                    digitalWrite(saara.em2Dirpin, LOW);
                }
                totalHomeCount = max(max(abs(sm2HomeCount), abs(em1HomeCount)), em2HomeCount); 
                // For loop to move robot
                for (int i = 0; i < totalHomeCount; i++){ 
                    if (i < sm2HomeCount){ 
                        digitalWrite(saara.sm2StepPin, HIGH);
                        if (i % 7 == 0){
                            if (digitalRead(saara.sm2Dirpin) == 1 && wristAngle > 700) { 
                                wristAngle--; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                            else if (wristAngle <= 2250) { 
                                wristAngle ++; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                        }
                    }
                    if (i < em1HomeCount){ 
                        digitalWrite(saara.em1StepPin, HIGH); 
                        if (i % 6 == 0){
                            if (digitalRead(saara.em1Dirpin) == 1 && wristAngle > 700) { 
                                wristAngle--; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                            else if (wristAngle <= 2250) { 
                                wristAngle ++; 
                                saara.wrist.writeMicroseconds(wristAngle); 
                            }
                        }
                    } 
                    if (i < em2HomeCount){ 
                        digitalWrite(saara.em2StepPin, HIGH); 
                    }
                    delayMicroseconds(1000);
                    digitalWrite(saara.sm2StepPin, LOW); 
                    digitalWrite(saara.em1StepPin, LOW); 
                    digitalWrite(saara.em2StepPin, LOW); 
                    delayMicroseconds(1000); 
                }

                // Move sm1 to the home value
                if (sm1StepCount > sm1DoorHome){ 
                    sm1HomeCount = sm1StepCount - sm1DoorHome; 
                    for (int i = 0; i < abs(sm1HomeCount); i++){ 
                        saara.make1Step(saara.sm1Dirpin, LOW, saara.sm1StepPin, 400); 
                    }
                }
                else {
                    sm1HomeCount = sm1DoorHome - sm1StepCount;
                    for (int i = 0; i < abs(sm1HomeCount); i++){ 
                        saara.make1Step(saara.sm1Dirpin, HIGH, saara.sm1StepPin, 400); 
                    }
                }

                // Set the current step count to the home
                sm1StepCount = sm1DoorHome;
                sm2StepCount = sm2DoorHome; 
                em1StepCount = em1DoorHome; 
                em2StepCount = em2DoorHome; 
            }
            counter = 0; 
            checkLedState(); 
        }
        
        void setHome(int home, int address)
        {
            int x = home / 255;
            EEPROM.write(address, x);
            int y = home % 255;
            EEPROM.write(address + 1, y);
        }

        void homeSetup(){ 
            sm1Home; 
            sm2Home; 
            em1Home;
            em2Home;
            sm1DrinkHome;
            sm2DrinkHome; 
            em1DrinkHome;
            em2DrinkHome;
        }
};

/*************************************************************** Main Program ****************************************************************************************/

Robot saara;
MoveLeftRight em1(saara); 
Wrist em2(saara); 
ForwardBackward sm2Em1(saara); 
UpDown upDown(saara, sm2Em1); 
Gripper gripper(saara);
Home home(saara);

void setup(){ 
    Serial.begin(9600);
    saara.lightLed(250, 255, 255);
    saara.assignPinMode();
    home.homeSetup(); 
    attachInterrupt(digitalPinToInterrupt(saara.powerButton), powerOn, RISING);
    attachInterrupt(digitalPinToInterrupt(saara.modeButton), switchMode, RISING); 
    attachInterrupt(digitalPinToInterrupt(saara.joystickButton), switchJoystickMode, RISING);
}

void loop(){ 
    if (digitalRead(saara.powerButton)) { 
        saara.power(); 
    }
    if (!modeState){
        if (saara.modeButtonState){ 
            saara.readJoystick(); 
            upDown.upDownMovement();
            em1.leftRight();
            saara.gripper.write(90);
        }
        else { 
            em1.leftRight();
            sm2Em1.fwdBkw();
            saara.gripper.write(90);
        }
    }
    else {
        if (saara.joystickButtonState){ 
            gripper.openClose();
        }
        else { 
            em2.em2Turn(); 
            em2.wristUpDown();
            saara.gripper.write(90);
        }
    }
    if (digitalRead(saara.homeButton)){  
        home.goToHome();
    }
    if (digitalRead(saara.drinkButton)){
        home.goToDrink(); 
    }
    if (digitalRead(saara.doorButton)){ 
        home.goToDoor();
    }

    // Serial.print("sm2 stepcount: "); Serial.print(sm2StepCount); 
    // Serial.print("  Em1 stepcount: ");Serial.println(em1StepCount);
    // Serial.print("Joystick X: "); Serial.print(analogRead(A0)); 
    // Serial.print("  Joystick Y: "); Serial.println(analogRead(A1));
}


/********************************************************* interrupt handler ***************************************************************************************/

// Power button interrupt handler to turn on robot with debouncing
void powerOn(){ 
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 50ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 200) 
    {
        saara.powerButtonState = !saara.powerButtonState;   
    }
    last_interrupt_time = interrupt_time;
}

void switchMode(){ 
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 50ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 200) 
    {
        saara.modeButtonState = !saara.modeButtonState; 
        if (!saara.modeButtonState){ 
            saara.lightLed(255, 210, 255); 
        }
        else { 
            saara.lightLed(255, 255, 210); 
        }
        modeState = 0; 
        home.modeButtonState = saara.modeButtonState;
    }
    last_interrupt_time = interrupt_time;
}

void switchJoystickMode(){ 
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 50ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 200) 
    {
        saara.joystickButtonState = !saara.joystickButtonState; 
        if (!saara.joystickButtonState){ 
            saara.lightLed(200, 255, 200);
        }
        else { 
            saara.lightLed(0, 210, 255);
        }
        modeState = 1; 
        home.joystickButtonState =saara.joystickButtonState;
    }
    last_interrupt_time = interrupt_time;
}
