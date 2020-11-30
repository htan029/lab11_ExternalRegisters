/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 11  Exercise 1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://youtu.be/NUeQ18wH42k
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
unsigned char lock = 0;
unsigned char check = 0;
unsigned char data = 0;
// ======== End Shared Variables ====================

// ======== Shift Register ==========================
void transmit_data(unsigned char data){
    int i;
    for(i = 0; i < 8; i++){
        PORTC = 0x08;
        PORTC |= ((data >> i) & 0x01);
        PORTC |= 0x02;
    }
    PORTC |= 0x04;
    PORTC = 0x00;
}
// ======== End Shift Register ======================

// ======== Tasks ===================================
//button_SMTick
enum button_States {Start, Init, Wait, Increment, Decrement, Wait2, Wait3, Reset};

int ButtonSMTick(int state){
    switch(state){
        case Start: data = 0x00; state = Wait; break;
        case Wait: 
            if(((~PINA) & 0x03) == 0x03){  
                data = 0x00;
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
            if(((~PINA) & 0x03) == 0x03) data = 0x00;
            else state = Wait;
            break;
        default: state = Start; break;
    };

    switch (state){
        case Start: break;
        case Wait: break;
        case Increment: //PORTC = cnt = (cnt == 0x09) ? 0x09 : cnt+1; break;
            if(data == 0xFF){
                data = 0xFF;
            } else {
                data++;
            }
            break;
        case Decrement: //PORTC = cnt = (cnt == 0x00) ? 0x00 : cnt-1; break;
            if(data == 0x00){
                data = 0x00;
            } else {
                data--;
            }
            break;
        case Wait2: break;
        case Wait3: break;
        default: break;
    };
    return state;
}
// int ButtonSMTick(int state){
//     switch(state){
//         case button_wait: 
//             if(PINA == 0x01)
//             break;
//         default: state = button_wait; break;
//     }

//     switch(state){
//         case button_wait:
//             if(((~PINB) & 0x80) == 0x80){
//                 lock = 0x00;
//             } 
//             break;
//     }
//     return state;
// }

//displaySMTick
enum display_States {display_display};

int displaySMTick(int state){
    switch(state){
        case display_display: state = display_display; break;
        default: state = display_display; break;
    }

    switch(state){
        case display_display:
            transmit_data(data);
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
    tasks[0]->period = 50;
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