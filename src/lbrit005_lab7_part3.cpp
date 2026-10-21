/*         Luis & lbrit005@ucr.edu:

            Discussion: 023

 *         Assignment: Lab #7 Exercise # 3

 *         Exercise Description: Control a stepper motor and a servo motor using a joystick. 
            The horizontal position of the joystick controls the direction 
            of the stepper motor, while the vertical position 
            controls the angle of the servo motor. The right button on the joystick turns on an 
            LED, while the left button turns it off. 
            Additionally, two buttons are used to adjust the speed of the stepper motor.
            Lastly, the buzzer plays a melody when the stepper motor is idle and the buttons are being pressed.

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: <https://youtu.be/asxEFpykV9g>

 */

#include "timerISR.h"
#include "helper.h"
#include "periph.h"


volatile unsigned int adcHor = 0; // detects horizontal position of joystick
volatile unsigned int adcVer = 0; // detects vertical position of joystick
volatile unsigned char but_one = 0; //right button
volatile unsigned char but_two = 0;  //left button
volatile unsigned char speed = 10; //speed button






#define NUM_TASKS 5 //TODO: Change to the number of tasks being used


//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;

int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};//Stepper motor phases
int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; //8 phases servo
//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long TASK1_PERIOD = 50; //TODO: Set the period of task 1
const unsigned long TASK2_PERIOD = 20; //TODO: Set the period of task 2
const unsigned long TASK3_PERIOD = 50; //TODO: Set the period of task 3
const unsigned long TASK4_PERIOD = 200; //TODO: Set the period of task 4
const unsigned long TASK5_PERIOD = 250; //TODO: Set the period of task 5

const unsigned long GCD_PERIOD = 1;//TODO:Set the GCD Period




task tasks[NUM_TASKS]; // declared task array with 5 tasks

enum JoyStick_STATEs {JOYSTICK_IDLE};  
        int TickFct_JoyStick(int state){
        adcHor = ADC_read(0);
        adcVer = ADC_read(1);
        but_one = !GetBit(PINC, 2); //right button
        but_two = !GetBit(PINC, 3);  //left button

        return state;

}

enum STEPPER_STATES{STEP_IDLE, STEPPER_CW, STEPPER_CCW, NOTE1, NOTE2, NOTE3};
int TickFct_Stepper(int state){
    static unsigned char step = 0;
    static unsigned count = 0;
    static unsigned int timer = 0;
    static unsigned char note = 0;
    switch (state){
        case STEP_IDLE:
                    if(adcHor < 450){
                        state = STEPPER_CW;
                    }
                    else if(adcHor > 600){
                        state = STEPPER_CCW;
                    }
                    timer++;
                    if(timer >= 3200){
                        timer = 0;
                        state = NOTE1;
                    }
                  break;
        case STEPPER_CW:
                    if(adcHor < 450){
                        state = STEPPER_CW;
                    }
                    else if(adcHor > 600){
                      state = STEPPER_CCW;
                    }
                    else{
                        state = STEP_IDLE;
                    }
                break;
                case STEPPER_CCW:
                    if(adcHor > 600){
                     state = STEPPER_CCW;
                    }
                    else if(adcHor < 450){
                        state = STEPPER_CW;
                    }
                    else{
                        state = STEP_IDLE;
                    }
                break;
                case NOTE1:
                    note++;
                    if(note >= 200){
                    note = 0;
                    state = NOTE2;
                    }
                    break;
                case NOTE2:
                    note++;
                    if(note >= 200){
                    note = 0;
                    state = NOTE3;
                    }
                    break;
                case NOTE3:
                    note++;
                    if(note >= 200){
                    note = 0;
                    state = STEP_IDLE;
                    }
                    break;
                default:
                    state = STEP_IDLE;
                    break;
                  }
                  // actions for each state
                  switch (state)  //actions 
                    {
                        case STEP_IDLE:
                count++;
                if(count >= 50){
                    count = 0;
                    if(GetBit(PINB, 0)){
                        PORTB = SetBit(PORTB, 0, 0);
                        PORTD = SetBit(PORTD, 7, 1);
                        PORTD = SetBit(PORTD, 5, 0);

                }
                else{
                    PORTB = SetBit(PORTB, 0, 1);
                    PORTD = SetBit(PORTD, 7, 0);
                    PORTD = SetBit(PORTD, 5, 1);
                }
            }
                break;
                    case STEPPER_CCW:
                    timer = 0;
                step = (step + 1) % 8;
                PORTB = (PORTB & 0x03) | (phases[step] << 2);
                count++;
                    if(count >= 33){
                        PORTB = SetBit(PORTB, 0, 0);
                        PORTD = SetBit(PORTD, 7, 0);
                        PORTD = SetBit(PORTD, 5, 1);
                if(count >= 66){

                        PORTB = SetBit(PORTB, 0, 0);
                        PORTD = SetBit(PORTD, 7, 1);
                        PORTD = SetBit(PORTD, 5, 0);
                    if(count >= 100){
                        count = 0;
                         PORTB = SetBit(PORTB, 0, 1);
                        PORTD = SetBit(PORTD, 7, 0);
                        PORTD = SetBit(PORTD, 5, 0);

                    }
                    
                    
            }
        }
                break;
            case STEPPER_CW:
                    timer = 0;
                step = (step + 7) % 8;
            PORTB = (PORTB & 0x03) | (phases[step] << 2);
            count++;
                    if(count >= 33){
                        PORTB = SetBit(PORTB, 0, 1);
                        PORTD = SetBit(PORTD, 7, 0);
                        PORTD = SetBit(PORTD, 5, 0);
                if(count >= 66){

                        PORTB = SetBit(PORTB, 0, 0);
                        PORTD = SetBit(PORTD, 7, 1);
                        PORTD = SetBit(PORTD, 5, 0);
                    if(count >= 100){
                        count = 0;
                         PORTB = SetBit(PORTB, 0, 0);
                        PORTD = SetBit(PORTD, 7, 0);
                        PORTD = SetBit(PORTD, 5, 1);

                    }
                    
                    
            }
            
        }
            break; 
        case NOTE1:
                TCCR0B = (TCCR0B & 0xF8) | 0x03;
                OCR0A = 128; //C5
                break;
            case NOTE2:
                TCCR0B = (TCCR0B & 0xF8) | 0x02;
                OCR0A = 128; //B4
                break;
            case NOTE3:
                TCCR0B = (TCCR0B & 0xF8) | 0x05;
                OCR0A = 128; //D5
                    break;
        }
                 return state;
                }
   
enum SERVO_STATES{SERVO_IDLE, SERVO_UP, SERVO_DOWN};
int TickFct_Servo(int state){
    switch(state){
        case SERVO_IDLE:
            if(adcVer < 450){
                state = SERVO_UP;
            }
            else if(adcVer > 600){
                state = SERVO_DOWN;
            }
            break;
        case SERVO_UP:
            if(adcVer < 450){
                state = SERVO_UP;
            }
            else if(adcVer > 600){
                state = SERVO_DOWN;
            }
            else{
                state = SERVO_IDLE;
            }
        break;
        case SERVO_DOWN:
            if(adcVer > 600){
                state = SERVO_DOWN;
            }
            else if(adcVer < 450){
                state = SERVO_UP;
            }
            else{
                state = SERVO_IDLE;
            }
        break;
        default:
        state = SERVO_IDLE;
        break;
    }
      // actions for each state
        switch(state){
            case SERVO_IDLE:
                OCR1A = 2999;
                PORTD = SetBit(PORTD, 4, 0);
                PORTD = SetBit(PORTD, 3, 1);
                PORTD = SetBit(PORTD, 2, 0);
            break;
            case SERVO_UP:
                OCR1A = 999;
                PORTD = SetBit(PORTD, 4, 0);
                PORTD = SetBit(PORTD, 3, 0);
                PORTD = SetBit(PORTD, 2, 1);
            break;
            case SERVO_DOWN:
                OCR1A = 4999;
                PORTD = SetBit(PORTD, 4, 1);
                PORTD = SetBit(PORTD, 3, 0);
                PORTD = SetBit(PORTD, 2, 0);
            break;

        }
      return state;
  }

enum SPEED_STATES{IDLE_SPEED, BUTTON_ONE, BUTTON_TWO};
int TickFct_Speed(int state){
    static unsigned char prev_but_one = 0;
    static unsigned char prev_but_two = 0;
    unsigned char one_pressed = but_one && !prev_but_one;
    unsigned char two_pressed = but_two && !prev_but_two;
    
    switch(state){
        case IDLE_SPEED:
            if(one_pressed){      
                state = BUTTON_ONE;
            }
            else if(two_pressed){ 
                state = BUTTON_TWO;
            }
            break;
        case BUTTON_ONE:
            if(speed > 5){
                speed -= 5;
            }
            tasks[1].period = speed;
            state = IDLE_SPEED;
            break;
        case BUTTON_TWO:
            if(speed < 30){
                speed += 5;
            }
            tasks[1].period = speed;
            state = IDLE_SPEED;
            break;
        default:
            state = IDLE_SPEED;
            break;
    }
    prev_but_one = but_one;
    prev_but_two = but_two;
    
    return state;
}

enum BUZZER_STATES{BUZZER_IDLE, BUZZER_NOTEUP1, BUZZER_NOTEDOWN1, BUZZER_NOTEUP2, BUZZER_NOTEDOWN2};
int TickFct_Buzzer(int state){
        static unsigned char prev_but_one = 0;
    static unsigned char prev_but_two = 0;
    unsigned char one_pressed = but_one && !prev_but_one;
    unsigned char two_pressed = but_two && !prev_but_two;
    switch(state){
        case BUZZER_IDLE:
            if(one_pressed){      
                state = BUZZER_NOTEUP1;
            }
            else if(two_pressed){ 
                state = BUZZER_NOTEDOWN1;
            }
            break;
        case BUZZER_NOTEUP1:
            state = BUZZER_NOTEUP2;
            break;
        case BUZZER_NOTEDOWN1:
            state = BUZZER_NOTEDOWN2;
            break;
        case BUZZER_NOTEUP2:
            state = BUZZER_IDLE;    
            break;
        case BUZZER_NOTEDOWN2:
            state = BUZZER_IDLE;    
            break;  
        default:
            state = BUZZER_IDLE;    
            break;
        //actions for each state
    }
        switch(state){
            case BUZZER_IDLE:
                OCR0A = 255;
            break;
            case BUZZER_NOTEUP1:
            TCCR0B = (TCCR0B & 0xF8) | 0x03;
                OCR0A = 128; //C5
            break;
            case BUZZER_NOTEDOWN1:
                TCCR0B = (TCCR0B & 0xF8) | 0x02;
                OCR0A = 128; //B4
            break;
            case BUZZER_NOTEUP2:
                TCCR0B = (TCCR0B & 0xF8) | 0x05;
                OCR0A = 150; //D5
            break;
            case BUZZER_NOTEDOWN2:
                TCCR0B = (TCCR0B & 0xF8) | 0x04;
                OCR0A = 128; //E5
            break;
        }
            prev_but_one = but_one;
        prev_but_two = but_two;
        return state;
    }


void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime >= tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}



//TODO: Create your tick functions for each task



int main(void) {
    //TODO: initialize all your inputs and ouputs
 // e.g. DDRx = 0xFF; for output, DDRx = 0x00; for input
    DDRC = 0x00;
    PORTC = 0x00;
    DDRD = 0xFC;
    PORTD = 0x00;
    DDRB = 0xFF;
    PORTB = 0x00;
    ADC_init();   // initializes ADC
    


    //TODO: Initialize the buzzer timer/pwm(timer0)
        TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);
        TCCR0B |= (1 << CS01);
        OCR0A = 255;

    //TODO: Initialize the servo timer/pwm(timer1)
    TCCR1A |= (1 << WGM11) | (1 << COM1A1);
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11);
    ICR1 = 39999;
    OCR1A = 2999;


    //TODO: Initialize tasks here
    // e.g. 
    // tasks[0].period = ;
    // tasks[0].state = ;
    // tasks[0].elapsedTime = ;
    // tasks[0].TickFct = ;
    tasks[0].period = TASK1_PERIOD;
    tasks[0].state = JOYSTICK_IDLE;
    tasks[0].elapsedTime = TASK1_PERIOD;
    tasks[0].TickFct = &TickFct_JoyStick;

    tasks[1].period = TASK2_PERIOD;
    tasks[1].state = STEP_IDLE;
    tasks[1].elapsedTime = TASK2_PERIOD;
    tasks[1].TickFct = &TickFct_Stepper;

    tasks[2].period = TASK3_PERIOD;
    tasks[2].state = SERVO_IDLE;
    tasks[2].elapsedTime = TASK3_PERIOD;
    tasks[2].TickFct = &TickFct_Servo;

    tasks[3].period = TASK4_PERIOD;
    tasks[3].state = IDLE_SPEED;
    tasks[3].elapsedTime = TASK4_PERIOD;
    tasks[3].TickFct = &TickFct_Speed;

    tasks[4].period = TASK5_PERIOD;
    tasks[4].state = BUZZER_IDLE;
    tasks[4].elapsedTime = TASK5_PERIOD;
    tasks[4].TickFct = &TickFct_Buzzer;


    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1) {}

    return 0;
}