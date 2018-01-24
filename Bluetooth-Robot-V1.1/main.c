/*
 * File:        main.c
 * Author:      Anthony Stark
 * Target PIC:  PIC32MX250F128B
 * 
 * New Updates: Now has PWM control for motors.
 *              Limited to range of 20% duty cycle
 *              and 80% duty cycle.
 */

////////////////////////////////////
// clock AND protoThreads configure!
#include "config.h"
// threading library
#include "pt_cornell_bluetooth.h"

// suppress error
#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
////////////////////////////////////



/* Bit Macros */
#define A_IN1 BIT_1
#define A_IN2 BIT_0
#define B_IN1 BIT_8
#define B_IN2 BIT_7
#define LED BIT_15

/* thread definitions */
static struct pt pt_bluetooth, pt_DMA_output, pt_input, pt_motor, pt_LED;

/* motor direction variables*/

static int motor_movement=0; // starts at 0 to ensure robot is not moving
static char cmd[16]; // for commands from terminal program

/* end motor direction variables*/

/* motor speed variables */

static int generate_period = 1000; //frequency of generating period of wave in Hz
static float value=0, pulse_width=0; //starts at 0 to ensure robot is not moving
static float duty_cycle;

/* end motor speed variables */

/* individual motor control functions */

void forward_A()
{
    mPORTASetBits(A_IN1);
    mPORTAClearBits(A_IN2);
}
void forward_B()
{
    mPORTBSetBits(B_IN1);
    mPORTBClearBits(B_IN2);
}

void backward_A()
{
    mPORTASetBits(A_IN2);
    mPORTAClearBits(A_IN1);
}

void backward_B()
{
    mPORTBSetBits(B_IN2);
    mPORTBClearBits(B_IN1);
}

void stop_A()
{
    mPORTAClearBits(A_IN1);
    mPORTAClearBits(A_IN2);
}

void stop_B()
{
    mPORTBClearBits(B_IN1);
    mPORTBClearBits(B_IN2);
}

/* end individual motor control functions */

/* robot moving functions 
   to simplify moving the robot in to certain directions */

void forward()
{
    forward_A();
    forward_B();
}

void backward()
{
    backward_A();
    backward_B();
}

void turn_left()
{
    backward_A();
    forward_B();
}

void turn_right()
{
    forward_A();
    backward_B();
}

void stop()
{
    stop_A();
    stop_B();
}
/* end robot moving functions */


/* begin threads */

/* thread to handle serial communication with terminal application */
static PT_THREAD (protothread_bluetooth(struct pt *pt))
{
    PT_BEGIN(pt);
    while(1){
        
        // send the prompt via DMA to serial
        sprintf(PT_send_buffer,"command: ");
        // by spawning a print thread
        PT_SPAWN(pt, &pt_DMA_output, PT_DMA_PutSerialBuffer(&pt_DMA_output) );
 
        // spawn a thread to handle terminal input
        // the input thread waits for input
        // -- BUT does NOT block other threads
        // string is returned in "PT_term_buffer"
        PT_SPAWN(pt, &pt_input, PT_GetSerialBuffer(&pt_input) );
        // returns when the thread dies
        // in this case, when <enter> is pushed
        // now parse the string
        sscanf(PT_term_buffer, "%s %f" , cmd, &value);
             
        if (cmd[0]=='u' ) {motor_movement=1; printf("forward\n");} 
        if (cmd[0]=='d' ) {motor_movement=2; printf("backward\n");} 
        if (cmd[0]=='l' ) {motor_movement=3; printf("turning left\n");}
        if (cmd[0]=='r' ) {motor_movement=4; printf("turning right\n");}
        if (cmd[0]=='s' )
        {
            /* change any non zero value to 0; 
             * stop command should ensure PWM channels are pulled completely low */
            if(cmd[0] == 's' && value != 0){value = 0;}
            motor_movement=0; 
            printf("stopped\n");
            
        }
        
        /* motor speed validation / correction */
        if ((cmd[0] == 'u' || cmd[0] == 'd' || cmd[0] == 'l' || cmd[0] == 'r') && value<1)
        {
            printf("changing to 1\n");
            value=1;
        }
        
        if ((cmd[0] == 'u' || cmd[0] == 'd' || cmd[0] == 'l' || cmd[0] == 'r') && value>4)
        {
            printf("changing to 4\n");
            value=4;
        }
        /* end motor speed validation / correction */
                 
    }//end while(1)     
    PT_END(pt);

}

/* thread to move motors; changes direction based on motor_movement flag */
static PT_THREAD (protothread_motor(struct pt *pt))
{
    PT_BEGIN(pt);
    
    /* duty cycle calculation for PWM */
    pulse_width = 2 * (value / 10);
    duty_cycle = generate_period * pulse_width;
    /* end duty cycle calculation for PWM */
        switch(motor_movement){
            case 0:     stop();
                        break;
            case 1:     forward();
                        break;
            case 2:     backward();
                        break;
            case 3:     turn_left();
                        break;
            case 4:     turn_right();
                        break;
            
        }//end switch statement
        
    //set up OC1 and OC2 with timer 3 to produce PWM
    OpenOC1(OC_ON | OC_TIMER3_SRC | OC_PWM_FAULT_PIN_DISABLE, duty_cycle, generate_period);
    OpenOC2(OC_ON | OC_TIMER3_SRC | OC_PWM_FAULT_PIN_DISABLE, duty_cycle, generate_period);
    PT_YIELD_TIME_msec(10); //yield thread
        
    //}//end while (1)
    PT_EXIT(pt);
    PT_END(pt);
}

/* thread that blinks LED 
   only purpose is for debugging whether or not other threads have blocking functions or logic */
static PT_THREAD (protothread_LED(struct pt *pt))
{
    PT_BEGIN(pt);
    while(1)
    {
        mPORTBToggleBits(LED);
        PT_YIELD_TIME_msec(1000); //yield thread
    }//end while (1)
    PT_END(pt);
}
/* end threads */

void main(void)
{
    /* set up timer 3 to produce PWM */
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_1, generate_period);
    
    /* setting up OutputCompare units 1 and 2; needed for PWM */
    PPSOutput(1, RPB4, OC1);// sets RPB4 to OutputCompare 1 --- pin 11]
    PPSOutput(2, RPB11, OC2);// sets RPB11 to OutputCompare 2 --- pin 22]
    
    /* attempting to keep PORT A pins for motor A and PORT B pins for motor B */
    mPORTASetPinsDigitalOut(A_IN1 | A_IN2); //sets pins for motor A as output
    mPORTBSetPinsDigitalOut(B_IN1 | B_IN2 | LED); // sets pins for motor B and LED as output
    
    /* config the uart, DMA, vref, timer5 ISR */
    PT_setup();
    
    /* setup system wide interrupts */
    
    /* needed to use PT_YIELD_TIME_msec because it uses timer5 ISR
      also needed for any other ISR that may be used */ 
    INTEnableSystemMultiVectoredInt(); 
    
    
    /* initialize threads */
    PT_INIT(&pt_bluetooth);
    PT_INIT(&pt_motor);
    PT_INIT(&pt_LED);
    
    //round-robin scheduler for threads
    while (1)
    {
        PT_SCHEDULE(protothread_bluetooth(&pt_bluetooth));
        PT_SCHEDULE(protothread_motor(&pt_motor));
        PT_SCHEDULE(protothread_LED(&pt_LED));
    }// end round-robin scheduler

}// end main