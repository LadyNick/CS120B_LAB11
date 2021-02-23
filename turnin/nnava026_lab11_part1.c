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

enum Register_States{display, inc, dec, reset, release}Register_State;
int RegisterTick(int Register_State){
	switch(Register_State){
		case display:
			transmit_data(num);
			if(A0){
				Register_State = dec;
			}
			else if(A1){
				Register_State = inc;
			}
			else if(A0 && A1){
				Register_State = reset;
			}
			else{ Register_State = display;}
			break;
		case inc:
			if(num == 0xFF){
				//do nothing, it stays the same because it's the highest
			}
			else{ ++num; }
			transmit_data(num);
			Register_State = release;
			break;
		case dec:
			if(num == 0x00){
				//do nothing, stays the same because it's the lowest
			}
			else{ --num; }
			transmit_data(num);
			Register_State = release;
			break;
		case reset:
			num = 0x00;
			transmit_data(num);
			Register_State = release;
			break;
		case release:
			if(!A0 && !A1){
				Register_State = display;
			}
			else if(A0 && A1){
				Register_State = reset;
			}
			break;
		default: Register_State = display; break;
	}
	return Register_State;
}


int main(void) {
    DDRC = 0xFF; PORTC = 0x00;
    DDRA = 0x00; PINA = 0xFF;

    static task task1;
    task *tasks[] = {&task1};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

    const char start = -1;

    //TASK1: Register + 2 Buttons
    task1.state = start;
    task1.period = 50; //manual didnt give specifics, but im choosing low anyway
    task1.elapsedTime = task1.period;
    task1.TickFct = &RegisterTick;

    TimerSet(50); //just to match the 1 task there is
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
			tasks[i]->elapsedTime += 50;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    }
    return 1;
}
