/*	Author: Nicole Navarro
 *  Partner(s) Name: 
 *	Lab Section:21
 *	Assignment: Lab #11  Exercise #1
 *	Video: https://youtu.be/Ihzll3KVDec
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

enum 


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
