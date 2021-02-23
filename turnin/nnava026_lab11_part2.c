/*	Author: Nicole Navarro
 *  Partner(s) Name: 
 *	Lab Section:21
 *	Assignment: Lab #11  Exercise #2
 *	Video: https://youtu.be/bCzpzsmNSXc
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "scheduler.h"
#endif

unsigned char A0; //decrement
unsigned char A1; //increment
unsigned char go = 1;
unsigned char change = 0;
unsigned char system = 0; //initially off
unsigned char num = 0x00;
unsigned char countlight1 = 0;
unsigned char countlight2 = 0;
unsigned char countlight3 = 0;

void transmit_data(unsigned char data) {
    int i;
    for (i = 0; i < 8 ; ++i) {
   	 // Sets SRCLR to 1 allowing data to be set
   	 // Also clears SRCLK in preparation of sending data
   	 PORTC = 0x08;
   	 // set SER = next bit of data to be sent.
   	 PORTC |= ((data >> i) & 0x01);
   	 // set SRCLK = 1. Rising edge shifts next bit of data into the shift register
   	 PORTC |= 0x02;
    }
    // set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
    PORTC |= 0x04;
    // clears all lines in preparation of a new transmission
    PORTC = 0x00;
}

enum Button_States{wait, inc, dec, both, release}Button_State;
int ButtonTick(int Button_State){
	switch(Button_State){
		case wait:
			if(A0){
				Button_State = dec;
			}
			else if(A1){
				Button_State = inc;
			}
			else if(A0 && A1){
				Button_State = both;
			}
			else{ Button_State = wait;}
			break;
		case inc:
			if(go == 3){
				go = 1;
			}
			else{ ++go; }
			Button_State = release;
			break;
		case dec:
			if(go == 1){
				go = 3;
			}
			else{ --go; }
			Button_State = release;
			break;
		case both:
			if(change){
				Button_State = release;
			}
			else{
			if(system == 0 ){
				system = 1;
				go = 1;
			}
			else if(system == 1){
				go = 1;
				system = 0;
			}
			change = 1;
			Button_State = release;
			}
			break;
		case release:
			if(!A0 && !A1){
				Button_State = wait;
				change = 0;
			}
			else if(A0 && A1){
				Button_State = both;
			}
			else{
				Button_State = release;
			}
			break;
		default: Button_State = wait; break;
	}
	return Button_State;
}

enum Light1_States{sequence1}Light1_State;
int Light1Tick(int Light1_State){
	switch(Light1_State){
		case sequence1:
			if(go != 1){ Light1_State = sequence1; countlight1 = 0; }
			else if((go == 1) && system){
				if(countlight1 == 0){ num = 0xFF; countlight1 = 1; }
				else if(countlight1 == 1){ num = 0x00; countlight1 = 0; }
				Light1_State = sequence1;
			}
			break;
		default: Light1_State = sequence1; break;	
	}
	return Light1_State;
}

enum Light2_States{sequence2}Light2_State;
int Light2Tick(int Light2_State){
	switch(Light2_State){
		case sequence2:
			if(go != 2){ Light2_State = sequence2; countlight2 = 0; }
			else if((go == 2) && system){
				if(countlight2 == 0){ num = 0xAA; countlight2 = 1;}
				else if(countlight2 == 1){ num = 0x55; countlight2 = 0;}
				Light2_State = sequence2;
			}
			break;
		default: Light2_State = sequence2; break;
	}
	return Light2_State;
}

enum Light3_States{sequence3}Light3_State;
int Light3Tick(int Light3_State){
        switch(Light3_State){
		case sequence3:
			if(go != 3){ Light3_State = sequence3; countlight3 = 0; }
			else if((go == 3) && system){
				if(countlight3 == 0){ 
					num = 0x00; 
					++countlight3; }
				else if((countlight3 == 1) || (countlight3 == 7)){
					if(countlight3 == 7){
						countlight3 = 0;
					}
					else{
						++countlight3;
					}
				       	num = 0x81; 
					 }
				else if((countlight3 == 2) || (countlight3 == 6)){ 
					num = 0xC3; 
					++countlight3; }
				else if((countlight3 == 3) || (countlight3 == 5)){ 
					num = 0xE7; 
					++countlight3; }
				else if(countlight3 == 4){ num = 0xFF; ++countlight3; }

        		}
			Light3_State = sequence3; break;
		default: Light3_State = sequence3; break;
	}
        return Light3_State;
}

enum Display_States{display}Display_State;
int DisplayTick(int Display_State){
	switch(Display_State){
		case display:
		if(!system){
			transmit_data(0x00);
		}
		else{
			transmit_data(num);
		}
		Display_State = display; break;
		default: Display_State = display; break;	
	}
	return Display_State;
}


int main(void) {
    DDRC = 0xFF; PORTC = 0x00;
    DDRA = 0x00; PINA = 0xFF;

    static task task1, task2, task3, task4, task5;
    task *tasks[] = {&task1, &task2, &task3, &task4, &task5};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    //Buttons
    task1.state = start;
    task1.period = 50; //manual didnt give specifics, but im choosing low anyway
    task1.elapsedTime = task1.period;
    task1.TickFct = &ButtonTick;

    //Blinking all off and on
    task2.state = start;
    task2.period = 800; 
    task2.elapsedTime = task2.period;
    task2.TickFct = &Light1Tick;

    //Blinking every other and switching
    task3.state = start;
    task3.period = 500;
    task3.elapsedTime = task3.period;
    task3.TickFct = &Light2Tick;

    //Going inwards and outwards
    task4.state = start;
    task4.period = 100;
    task4.elapsedTime = task4.period;
    task4.TickFct = &Light3Tick;

    task5.state = start;
    task5.period = 1;
    task5.elapsedTime = task5.period;
    task5.TickFct = &DisplayTick;

    TimerSet(1); //just to match the 1 task there is
    TimerOn();
    unsigned short i;

    while(1){
	A0 = ~PINA & 0x01;
	A1 = ~PINA & 0x02;

	for(i=0; i<numTasks; i++){ //Scheduler code
			if(tasks[i]->elapsedTime == tasks[i]->period){
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
