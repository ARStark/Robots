////////////////////////////////////
// clock AND protoThreads configure!
#include "config.h"
// threading library
#include "pt_cornell_bluetooth.h"

// suppress error
#define _SUPPRESS_PLIB_WARNING 
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
////////////////////////////////////



/// BIT MACROS
#define A_IN1 BIT_1
#define A_IN2 BIT_0
#define B_IN1 BIT_8
#define B_IN2 BIT_7
#define LED BIT_15

static struct pt pt_bluetooth, pt_DMA_output, pt_input, pt_robot, pt_LED;

static int robot_movement=0; // starts at 0 to ensure robot is not moving

static char cmd[16]; // for commands from terminal program

//individual motor control functions
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
// end individual motor control functions

/* robot moving functions 
   to simplify moving the robot in to certain directions */
void forward()
{
    forward_A();
    //forward_B();
}

void backward()
{
    backward_A();
    //backward_B();
}

void turn_left()
{
    backward_A();
    //forward_B();
}

void turn_right()
{
    forward_A();
    //backward_B();
}

void stop()
{
    stop_A();
    stop_B();
}
// end robot moving functions

static PT_THREAD (protothread_bluetooth(struct pt *pt))
{
    PT_BEGIN(pt);
    while(1){
        // send the prompt via DMA to serial
            sprintf(PT_send_buffer,"command: ");
            // by spawning a print thread
            PT_SPAWN(pt, &pt_DMA_output, PT_DMA_PutSerialBuffer(&pt_DMA_output) );
 
          //spawn a thread to handle terminal input
            // the input thread waits for input
            // -- BUT does NOT block other threads
            // string is returned in "PT_term_buffer"
            PT_SPAWN(pt, &pt_input, PT_GetSerialBuffer(&pt_input) );
            // returns when the thread dies
            // in this case, when <enter> is pushed
            // now parse the string
             sscanf(PT_term_buffer, "%s" , cmd);
             
            if (cmd[0]=='u' ) {robot_movement=1; printf("forward\n");} 
            if (cmd[0]=='d' ) {robot_movement=2; printf("backward\n");} 
            if (cmd[0]=='l' ) {robot_movement=3; printf("turning left\n");}
            if (cmd[0]=='r' ) {robot_movement=4; printf("turning right\n");}
            if (cmd[0]=='s' ) {robot_movement=0; printf("stopped\n");}
           
                 
    }//end while(1)     
    PT_END(pt);

}

//thread to move robot; changes direction based on robot_movement flag
static PT_THREAD (protothread_robot(struct pt *pt))
{
    PT_BEGIN(pt);
    //while(1)
    //{
       
        switch(robot_movement){
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
        
    
    //}//end while (1)
    PT_EXIT(pt);
    PT_END(pt);
}

static PT_THREAD (protothread_LED(struct pt *pt))
{
    PT_BEGIN(pt);
    while(1)
    {
        mPORTBToggleBits(LED);
        PT_YIELD_TIME_msec(1000);
        //printf("%d\n", robot_movement);
    }//end while (1)
    PT_END(pt);
}

void main(void)
{
    //attemtping to keep PORT A pins for motor A and PORT B pins for motor B
    mPORTASetPinsDigitalOut(A_IN1 | A_IN2); //sets pins for motor A as output
    mPORTBSetPinsDigitalOut(B_IN1 | B_IN2 | LED); // sets pins for motor B as output
    
    // === config the uart, DMA, vref, timer5 ISR ===========
    PT_setup();
    
    // === setup system wide interrupts  ====================
    /*needed to use PT_YIELD_TIME_msec because it uses timer5 ISR
      also needed for any other ISR that may be used*/ 
    INTEnableSystemMultiVectoredInt(); 
    
    
    // initialize threads
    PT_INIT(&pt_bluetooth);
    PT_INIT(&pt_robot);
    PT_INIT(&pt_LED);
    
        // round-robin scheduler for threads
    while (1)
    {
        PT_SCHEDULE(protothread_bluetooth(&pt_bluetooth));
        PT_SCHEDULE(protothread_robot(&pt_robot));
        PT_SCHEDULE(protothread_LED(&pt_LED));
    }// end round-robin scheduler

}