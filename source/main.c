/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 11  Exercise 4
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <timer.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
////////////////////////////////////////////////////
// ======== task struct ===================

typedef struct task {
  signed char state; // Current state of the task
  unsigned long int period; // Rate at which the task should tick
  unsigned long int elapsedTime; // Time since task's previous tick
  int (*TickFct)(int); // Function to call for task's tick
} task;

// ======== End Task scheduler data structure =======

// ======== Shared Variables ========================
unsigned char num1 = 0xFF;
unsigned char num2 = 0x00;
// ======== End Shared Variables ====================

// ======== Shift Register ==========================
void transmit_data(unsigned char data, unsigned char shifter){
    // PORTC[3] connected to SRCLR //independent
    // PORTC[2] connected to RCLK //independent
    // PORTC[1] connected to SRCLK //same
    // PORTC[0] connected to SER //same

    int i;
    for(i = 0; i < 8; i++){
        //sets SRCLR and clears SRCLK
        if(shifter == 0x00){
            PORTC = 0x08;
        } else if (shifter == 0x01){
            PORTC = 0x20;
        }
        
        //SER //same 
        PORTC |= ((data >> i) & 0x01);
        //SRCLK //same
        PORTC |= 0x02;
    }
    //RCLK 
    if(shifter == 0x00){
        PORTC |= 0x04;
    } else if (shifter == 0x01){
        PORTC |= 0x10;
    }
    
    //all lines
    PORTC = 0x00;
}
// ======== End Shift Register ======================

// ======== Tasks ===================================
//button_SMTick
enum button_States {Start, Init, Wait, Increment, Decrement, Wait2, Wait3, Reset};

int ButtonSMTick(int state){
    switch(state){
        case Start: state = Wait; break;
        case Wait: 
            if(((~PINA) & 0x03) == 0x03){  
                state = Reset;
            } else if (((~PINA) & 0x03) == 0x01){
                state = Increment;
            } else if (((~PINA) & 0x03) == 0x02){
                state = Decrement;
            } 
            break;
        case Increment: state = Wait2; break;
        case Decrement: state = Wait3; break;
        case Wait2:
            if(((~PINA) & 0x03) == 0x03){
                state = Reset;
            } 
            else state = Wait;
            break;
        case Wait3:
            if(((~PINA) & 0x03) == 0x03){
                state = Reset;
            } 
            else state = Wait;
            break;
        case Reset:
            if(((~PINA) & 0x03) == 0x03) state = Reset;
            else state = Wait;
            break;
        default: state = Start; break;
    };

    switch (state){
        case Start: break;
        case Wait: break;
        case Increment: //PORTC = cnt = (cnt == 0x09) ? 0x09 : cnt+1; break;
            if(num1 == 0xFF){
                if(num2 != 0xFF){
                    num1 = 0x00;
                    num2++;
                }
            } else {
                num1++;
            }
            break;
        case Decrement: //PORTC = cnt = (cnt == 0x00) ? 0x00 : cnt-1; break;
            if(num2 != 0x00){
                if(num1 == 0x00){
                    num2--;
                    num1 = 0xFF;
                } else {
                    num1--;
                }
            } else {
                if(num1 != 0x00)
                    num1--;
            }
            
            break;
        case Wait2: break;
        case Wait3: break;
        default: break;
    };
    return state;
}

//displaySMTick
enum display_States {display_display};

int displaySMTick(int state){
    switch(state){
        case display_display: state = display_display; break;
        default: state = display_display; break;
    }

    switch(state){
        case display_display:
            transmit_data(num1, 0x00);
            transmit_data(num2, 0x01);
            break;
    }
    return state;
}
// ======= task struct end ==================

// ======= find GCD =========================
unsigned long int findGCD(unsigned long int a, unsigned long int b){
    unsigned long int c;
    while (1){
        c= a%b;
        if(c==0){return b;}
        a = b; 
        b = c;
    }
    return 0;
}
// ====== end find GCD ======================

// ====== main ==============================
int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;

    static task task1, task2, task3, task4;
    task *tasks[] = { &task1, &task2};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
    
    const char start = -1;

    tasks[0]->state = start;
    tasks[0]->period = 100;
    tasks[0]->elapsedTime = tasks[0]->period;
    tasks[0]->TickFct = &displaySMTick;

    tasks[1]->state = start;
    tasks[1]->period = 50;
    tasks[1]->elapsedTime = tasks[1]->period;
    tasks[1]->TickFct = &ButtonSMTick;

    unsigned short i;
    unsigned long gcd = tasks[0]->period;
    for(i = 1; i < numTasks; i++){
        gcd = findGCD(gcd, tasks[i]->period);
    }

    TimerSet(gcd);
    TimerOn();
    
    /* Insert your solution below */
    while (1) {
        for(i = 0; i < numTasks; i++){
            if(tasks[i]->elapsedTime == tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += gcd;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 0;
}