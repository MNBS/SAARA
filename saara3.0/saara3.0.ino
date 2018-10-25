#include <Servo.h>

// Motor step Count 
int em1StepCount = 0; 
int sm2StepCount = 0; 
int sm1StepCount = 0; 
int em2StepCount = 0; 

// Wrist servo position track
int wristAngle = 1400; 

// Initialize Robot object which includes power on and off
class Robot{
    private: 
        // Led Pins
        int redLed = 8; 
        int greenLed = 9; 
        int blueLed = 11; 
        // Pin variable 
        int relayPin = 12;  

    public:
        int joystickX = A0; 
        int joystickY = A1;
        // Initialize pin numbers 
        int powerButton = 3;

        // Motor pins
        int em1Dirpin = 5; 
        int em1StepPin = 4;
        int sm2Dirpin = 40; 
        int sm2StepPin = 41; 
        int sm1Dirpin = 43; 
        int sm1StepPin = 10;
        int em2Dirpin = 39; 
        int em2StepPin = 38; 

        // Position where motor is initially turn on
        int sm1PowerHome = 0; 
        int em2PowerHome = 0; 
        // powerButtonState variable to keep track whether of whether robot is on or off
        int powerButtonState = 0; 

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
        }

        // Light up LED with values as brightness
        void lightLed(int redLedBrightness, int greenLedBrightness, int blueLedbrightness){ 
            analogWrite(redLed, redLedBrightness); 
            analogWrite(redLed, greenLedBrightness); 
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
                // Attach servo 
                gripper.attach(6); 
                wrist.attach(7);

                // Light up LED to yellow showing initialization
                lightLed(210, 210, 255); 
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

                wrist.writeMicroseconds(wristAngle); 
                // Update motor power on initial stage
                sm2StepCount += 2500; 
                em1StepCount += 2727; 
            }
            // Turning of robot arm off
            else { 
                // Light up LED with yellow
                lightLed(210, 210, 255); 
                wristAngle = 1400; 
                wrist.writeMicroseconds(wristAngle);
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

                //Turn the relay off
                digitalWrite(relayPin, LOW);

                // Reset counter and steps to 0
                sm2PowerCounter = 0; 
                em1PowerCounter = 0; 
                sm2StepCount = 0; 
                em1StepCount = 0; 
                sm1StepCount = 0; 
                em2StepCount = 0; 

                // Light up red LED
                lightLed(210, 255, 255);
            }
        }

        // Read Joystick value 
        void readJoystick(){ 
            joystickX = analogRead(A1); 
            joystickY = analogRead(A0);  
        }
};

/******************************************************************** Turn Robot Left and Right **********************************************************************/

class MoveLeftRight {
    private: 
        // Composition from Robot Class
        Robot saara; 
    public: 
        // Constructor for MoveLeftRight class
        MoveLeftRight(Robot object) : saara(object){

        }
        
        // Function to turn robot left and right
        void leftRight(){
            // Read joystick Value
            saara.readJoystick();

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
}; 
/*************************************************************** Wrist movement ***************************************************************************************/
class Wrist{ 
    private: 
    // Composition from saara
        Robot saara; 
    public: 
        Wrist(Robot object) : saara(object) {

        }

        // Control left and right wrist movement
        void em2Turn(){ 
            // Read joystick value 
            saara.readJoystick(); 

            if (saara.joystickX > 550){ 
                saara.make1Step(saara.em2Dirpin, LOW, saara.em2StepPin, 1000); 
                em2StepCount ++; 
            }
            else if (saara.joystickX < 500){ 
                saara.make1Step(saara.em2Dirpin, HIGH, saara.em2StepPin, 1000); 
                em2StepCount --; 
            }
        }

        // Controlling of wrist servo for up down wrist movement where minimum is 700 and max is 1800, 700 is in upwards position and 1800 is pointing down
        void wristUpDown(){
            // Reading of joystick value
            saara.readJoystick(); 

            // Move wrist upwards
            if (saara.joystickY > 550 && wristAngle > 750){ 
                wristAngle --; 
                saara.wrist.writeMicroseconds(wristAngle);
                delayMicroseconds(2000);  
            }
            // Move wrist downwards
            else if (saara.joystickY < 500 && wristAngle < 1750){
                wristAngle ++;  
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
        int speed = 0;
        // Initialize counter variable to count how many steps
        int fwdStepsCounter = 0; 

    public: 
        ForwardBackward(Robot object) : saara(object){

        }

        // Moving robot Forward and backward where joystickY value > 550 means moving forward and sm2stepCount midpoint is 4953 
        // SpeedControl function will set the speed value
        void fwdBkw(){ 
            saara.readJoystick();
            if (saara.joystickY > 550 && sm2StepCount <= 4953 && sm2StepCount < 9411){ 
                speedControl(); 
                parallelMovement(saara.sm2StepPin, saara.em1StepPin, LOW, HIGH, 4, 5, 1, 0, speed);
            }
            else if(saara.joystickY > 550 && sm2StepCount < 9411){ 
                speedControl(); 
                parallelMovement(saara.em1StepPin, saara.sm2StepPin, LOW, HIGH, 5, 7, 0, 0, speed); 
            }
            else if(saara.joystickY < 450 && sm2StepCount <= 4953) { 
                speedControl(); 
                parallelMovement(saara.sm2StepPin, saara.em1StepPin, HIGH, LOW, 4, 5, 1, 1, speed);
            }
            else if (saara.joystickY < 450){
                speedControl(); 
                parallelMovement(saara.em1StepPin, saara.sm2StepPin, HIGH, LOW, 5, 7, 0, 1, speed);                 
            }
        }

        /* parallelMovement function is to scale the movement of robot arm so em1 is always parallel to the ground when moving
        StepPin is which link is the one that is not scaled and lessStepPin is the one that is scaled
        dir is link to stepPin while, dir2 is link to lessStepPin
        stesValue is the point in which the link that is moving in scale value will stop while reset value will reset the counter to 0
        value is to set the stepcount between em1 and sm2 where value == 1 will calculate em1StepCount and 0 will calculate sm2StepCount
        Negative is to see whether negative is applicable where 1 is where sm2 or em1 step count will - 
        Speed is to control the delay Microseconds */
        void parallelMovement(int stepPin, int lessStepPin, bool dir, bool dir2, int stepsValue, int resetValue, int value, int negative, int speed){ 
            digitalWrite(saara.sm2Dirpin, dir); 
            digitalWrite(saara.em1Dirpin, dir2);
            digitalWrite(stepPin, HIGH);
            if (fwdStepsCounter <= 4) { 
                digitalWrite(lessStepPin, HIGH); 
                if (value == 1 && negative == 0){ 
                    em1StepCount ++;
                } 
                else if (value == 1){ 
                    em1StepCount --; 
                }
                else if (value == 0 && negative == 0){ 
                    sm2StepCount ++; 
                }
                else{ 
                    sm2StepCount --; 
                }
            }
            if (fwdStepsCounter >= 5){
                fwdStepsCounter = 0;  
            }
            delayMicroseconds(speed);
            digitalWrite(saara.sm2StepPin, LOW); 
            digitalWrite(saara.em1StepPin, LOW);
            delayMicroseconds(speed); 
            fwdStepsCounter ++; 
            if (value == 1 && negative == 0){ 
                sm2StepCount ++;
            } 
            else if (value == 1){ 
                sm2StepCount --; 
            }
            else if (value == 0 && negative == 0){ 
                em1StepCount ++; 
            }
            else{ 
                em1StepCount --; 
            }
        }
        
        // Control the speed of the forward and backward movement
        void speedControl(){ 
            if (saara.joystickY > 850){
                speed = 400; 
            }
            else if (saara.joystickY > 800){ 
                speed = 600; 
            }
            else if (saara.joystickY > 750){ 
                speed = 800; 
            }
            else if (saara.joystickY > 700){ 
                speed = 1000; 
            }
            else if (saara.joystickY > 650) {
                speed = 1200; 
            }
            else if (saara.joystickY > 600) {
                speed = 1400; 
            }
            else if (saara.joystickY > 550){ 
                speed = 1600;
            }

            if (saara.joystickY < 150){ 
                speed = 400; 
            }
            else if (saara.joystickY < 200){ 
                speed = 600; 
            }
            else if (saara.joystickY < 250){ 
                speed = 800; 
            }
            else if (saara.joystickY < 300){ 
                speed = 1000; 
            }
            else if (saara.joystickY < 350) {
                speed = 1200; 
            }
            else if (saara.joystickY < 400) {
                speed = 1400; 
            }
            else if (saara.joystickY < 450) { 
                speed = 1600;
            }
        }
};


/*************************************************************** Main Program ****************************************************************************************/

Robot saara;
MoveLeftRight em1(saara); 
Wrist em2(saara); 
ForwardBackward sm2Em1(saara); 

void setup(){ 
    Serial.begin(9600);
    saara.assignPinMode();
    attachInterrupt(digitalPinToInterrupt(saara.powerButton), powerOn, RISING);
}

void loop(){ 
    /********************************************************** Test Classes ***************************************************************************************/ 

    // em1.leftRight();
    // em2.em2Turn(); 
    // em2.wristUpDown();
    sm2Em1.fwdBkw(); 
}


/********************************************************* interrupt handler ***************************************************************************************/

// Power button interrupt handler to turn on robot with debouncing
void powerOn(){ 
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 50ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 50) 
    {
        saara.powerButtonState = !saara.powerButtonState;   
        saara.power();        
    }
    last_interrupt_time = interrupt_time;
}