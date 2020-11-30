/*	Author: Heng Tan
 *  Partner(s) Name: 
 *	Lab Section: 024
 *	Assignment: Lab 11  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *  Demo Link: https://youtu.be/JKYUGMksMHo
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
unsigned char go = 0x00;
unsigned char onOff1 = 0x00;
unsigned char onOff2 = 0x00;
unsigned char light_chosen1 = 0;
unsigned char light_chosen2 = 0;
unsigned char light1[] = {0x00, 0x81, 0xC3, 0xE7, 0xFF, 0xE7, 0xC3, 0x81,
                          0x00, 0x81, 0xC3, 0xE7, 0xFF, 0xE7, 0xC3, 0x81};
unsigned char light2[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
                          0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
unsigned char light3[] = {0x00, 0x81, 0x42, 0xC3, 0x24, 0xA5, 0x66, 0xE7,
                          0x18, 0x99, 0x5A, 0xDB, 0x3C, 0xBD, 0x7E, 0xFF};
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
        case Start: light_chosen1 = 0x00; light_chosen2 = 0x00; state = Wait; break;
        case Wait: 
            if(((~PINA) & 0x03) == 0x03){  
                onOff1 = ~onOff1;
                go = 0x00;
                state = Reset;
            } else if (((~PINA) & 0x03) == 0x01){
                state = Increment;
                go = 0x00;
            } else if (((~PINA) & 0x03) == 0x02){
                state = Decrement;
                go = 0x00;
            } else if(((~PINA) & 0x0C) == 0x0C){  
                onOff2 = ~onOff2;
                go = 0x01;
                state = Reset;
            } else if (((~PINA) & 0x0C) == 0x04){
                state = Increment;
                go = 0x01;
            } else if (((~PINA) & 0x0C) == 0x08){
                state = Decrement;
                go = 0x01;
            }
            break;
        case Increment: state = Wait2; break;
        case Decrement: state = Wait3; break;
        case Wait2:
            if(((~PINA) & 0x03) == 0x03){
                state = Reset;
            } else if(((~PINA) & 0x0C) == 0x0C){
                state = Reset;
            }
            else state = Wait;
            break;
        case Wait3:
            if(((~PINA) & 0x03) == 0x03){
                state = Reset;
            } else if(((~PINA) & 0x0C) == 0x0C){
                state = Reset;
            } 
            else state = Wait;
            break;
        case Reset:
            if(((~PINA) & 0x03) == 0x03) light_chosen1 = 0x02;
            else if(((~PINA) & 0x0C) == 0x0C) light_chosen2 = 0x02;
            else state = Wait;
            break;
        default: state = Start; break;
    };

    switch (state){
        case Start: break;
        case Wait: break;
        case Increment: //PORTC = cnt = (cnt == 0x09) ? 0x09 : cnt+1; break;
            if(go == 0x00){
                if(light_chosen1 == 0x02){
                    light_chosen1 = 0x00;
                } else {
                    light_chosen1++;
                }
            } else if(go == 0x01){
                if(light_chosen2 == 0x02){
                    light_chosen2 = 0x00;
                } else {
                    light_chosen2++;
                }
            }
            break;
        case Decrement: //PORTC = cnt = (cnt == 0x00) ? 0x00 : cnt-1; break;
            if(go == 0x00){
                if(light_chosen1 == 0x00){
                    light_chosen1 = 0x02;
                } else {
                    light_chosen1--;
                }
            } else if(go == 0x01){
                if(light_chosen2 == 0x00){
                    light_chosen2 = 0x02;
                } else {
                    light_chosen2--;
                }
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
    static unsigned char light_localtion1;
    static unsigned char light_localtion2;
    switch(state){
        case display_display: state = display_display; break;
        default: state = display_display; light_localtion1 = 0; light_localtion2 = 0; break;
    }

    switch(state){
        case display_display:
            if(onOff1 == 0x00){
                transmit_data(0x00, 0x00);
            } else {
                if(light_localtion1 == 0x10){
                    light_localtion1 = 0x00;
                } else {
                    light_localtion1++;
                }
                if(light_chosen1 == 0x00) transmit_data(light1[light_localtion1], 0x00);
                else if(light_chosen1 == 0x01) transmit_data(light2[light_localtion1], 0x00);
                else if(light_chosen1 == 0x02) transmit_data(light3[light_localtion1], 0x00);
            }
            if(onOff2 == 0x00){
                transmit_data(0x00, 0x01);
            } else {
                if(light_localtion2 == 0x10){
                    light_localtion2 = 0x00;
                } else {
                    light_localtion2++;
                }
                if(light_chosen2 == 0x00) transmit_data(light1[light_localtion2], 0x01);
                else if(light_chosen2 == 0x01) transmit_data(light2[light_localtion2], 0x01);
                else if(light_chosen2 == 0x02) transmit_data(light3[light_localtion2], 0x01);
            }
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