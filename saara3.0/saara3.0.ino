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
        // Initialize pin numbers 
        int powerButton = 3;

        // Motor pins
        int em1Dirpin = 5; 
        int em1StepPin = 4;
        int sm2Dirpin = 40; 
        int sm2StepPin = 41; 

        // Motor step Count 
        int em1StepCount = 0; 
        int sm2StepCount = 0; 

        // powerButtonState variable to keep track whether of whether robot is on or off
        int powerButtonState = 0; 

        // While loop counter variable
        int sm2PowerCounter = 0; 
        int em1PowerCounter = 0; 

        // Assign pin whether it is an input or output
        void assignPinMode(){ 
            pinMode(relayPin, OUTPUT); 
            pinMode(sm2Dirpin, OUTPUT); 
            pinMode(sm2StepPin, OUTPUT); 
            pinMode(em1Dirpin, OUTPUT); 
            pinMode(em1StepPin, OUTPUT);
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
        // Function to turn the robot on or off
        void power(){ 
            // powerButtonState is from interrupt handler function, if powerButtonState is HIGH, will turn on the arm and if it's low, arm is power off
            if (powerButtonState){ 
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

                // Update motor power on initial stage
                sm2StepCount += 3100; 
                em1StepCount += 2727; 
            }
            // Turning of robot arm off
            else { 
                // Light up LED with yellow
                lightLed(210, 210, 255); 

                // Storing the maximum steps it need to move which will be use in the while loop
                int maxStepToMove = max(abs(sm2StepCount), abs(em1StepCount)); 
                // Counter for powering off arm
                int counter = 0; 
                // USing the maximum steps as the limit for while loop
                while (counter < maxStepToMove){ 
                    if (counter < sm2StepCount){ 
                        make1Step(sm2Dirpin, HIGH, sm2StepPin, 2000); 
                    }
                    if (counter < em1StepCount){ 
                        make1Step(em1Dirpin, LOW, em1StepCount, 400); 
                    }
                    counter++; 
                }
                digitalWrite(relayPin, LOW);

                // Reset counter and steps to 0
                sm2PowerCounter = 0; 
                em1PowerCounter = 0; 
                sm2StepCount = 0; 
                em1StepCount = 0; 

                // Light up red LED
                lightLed(210, 255, 255);
            }
        }
};

/*************************************************************** Main Program ****************************************************************************************/

Robot saara;
 
void setup(){ 
    saara.assignPinMode();
    attachInterrupt(digitalPinToInterrupt(saara.powerButton), powerOn, RISING);
}

void loop(){ 
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