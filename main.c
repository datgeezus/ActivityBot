#include <stdint.h>
#include "simpletools.h"
#include "abdrive.h"   

//#define _debug

#define CYCLES_PER_MS   100
#define HB_MAX          CYCLES_PER_MS * 10
#define LED_HB          26
#define LED_1           27

#define PIN_R_ENCODER   7 // 15
#define PIN_L_ENCODER   6 // 14

#define IN_MAX          3
#define COGS_MAX        8

#define DELAY           400
#define SPEED           50

enum Pins
{
    PIN_Q_L,
    PIN_Q_C,
    PIN_Q_R,
    PIN_Q_3,
    PIN_Q_4,
    PIN_Q_5,
    PIN_Q_6,
    PIN_Q_7,
    PIN_PING
};


typedef enum
{
    ST_DECIDE,
    ST_MOVE,
    ST_OVER_LINE
} State;

typedef enum
{
    MV_LEFT,
    MV_RIGHT,
    MV_REVERSE,
    MV_FORWARD
} MoveSt;


int* cog[COGS_MAX];
uint8_t inPins[] = {0, 1, 2, 3, 4, 5, 6, 7};
volatile uint8_t inputs[IN_MAX];

volatile State state = ST_MOVE;
volatile MoveSt moveState = MV_FORWARD;

uint32_t hbCount = 0U;
uint8_t hbLast = 0U;

void(*Toggle[])(int pin) = {low, high};

void Robot_Init(void);
void Robot_CheckInputs(void* data);
void Robot_HeartBeat(void);

void Process_DecideMove(void* data);
void Process_Move(void* data);
void Process_OverLine(void* data);

void Move_Left(void* data);
void Move_Right(void* data);
void Move_Reverse(void* data);

typedef void(*ProcessCB)(void* data);
ProcessCB Robot_Process[] = {Process_DecideMove, Process_Move, Process_OverLine};
ProcessCB Move[] = {Move_Left, Move_Right, Move_Reverse};

int main(void)
{
    state = ST_DECIDE;

    //cog[0] = cog_run(Process_CheckInputs, 128);
    //cog[1] = cog_run(Process_Move, 128);
    //cog[2] = cog_run(Move_Toggle, 128);

    Robot_Init();

    while(1)
    {
        Robot_CheckInputs(0);
        Robot_Process[state](0);

        //Robot_HeartBeat();
    }
    return 0;
}

void Robot_Init(void)
{
    pause(8000);
    drive_speed(SPEED, SPEED);
}

void Robot_CheckInputs(void* data)
{
    uint8_t i;
    for(i = 0; i < IN_MAX; ++i)
    {
        inputs[i] = input(inPins[i]);
    }
}

void Process_DecideMove(void* data)
{
#ifdef _debug
    if(inputs[PIN_Q_L] && inputs[PIN_Q_C] && !(inputs[PIN_Q_R]))
#else
    if(!(inputs[PIN_Q_L]) && !(inputs[PIN_Q_C]) && inputs[PIN_Q_R])
#endif
    {
        moveState= MV_RIGHT;
        state = ST_MOVE;
    }
#ifdef _debug
    else if(inputs[PIN_Q_R] && inputs[PIN_Q_C] &&!(inputs[PIN_Q_L]))
#else
    else if(!(inputs[PIN_Q_R]) && !(inputs[PIN_Q_C]) && inputs[PIN_Q_L])
#endif
    {
        moveState = MV_LEFT;
        state = ST_MOVE;
    }
#ifdef _debug
    else if(inputs[PIN_Q_R] && inputs[PIN_Q_C] && inputs[PIN_Q_L])
#else
    else if(!(inputs[PIN_Q_R]) && !(inputs[PIN_Q_C]) && !(inputs[PIN_Q_L]))
#endif
    {
        moveState = MV_REVERSE;
        state = ST_MOVE;
    }
    else
    {
        state = ST_DECIDE;
    }
}

void Process_Move(void* data)
{
    Move[moveState](0);
}

void Move_Left(void* data)
{
    drive_speed(SPEED, -SPEED);
    pause(DELAY);
    drive_speed(SPEED, SPEED);
    high(LED_HB);
    low(LED_1);
    state = ST_OVER_LINE;
}

void Move_Right(void* data)
{
    drive_speed(-SPEED, SPEED);
    pause(DELAY);
    drive_speed(SPEED, SPEED);
    high(LED_1);
    low(LED_HB);
    state = ST_OVER_LINE;
}

void Move_Reverse(void *data)
{
    drive_speed(-8, -SPEED);
    pause(DELAY);
    drive_speed(SPEED, SPEED);
    high(LED_HB);
    high(LED_1);
    state = ST_OVER_LINE;
}

void Process_OverLine(void* data)
{
#ifdef _debug
    if(!(inputs[PIN_Q_L]) && !(inputs[PIN_Q_C]) && !(inputs[PIN_Q_R]))
#else
    if(inputs[PIN_Q_L] && inputs[PIN_Q_C] && inputs[PIN_Q_R])
#endif
    {
        /* high(LED_1); */
        state = ST_DECIDE;
    }
    else
    {
        /* low(LED_1); */
        state = ST_OVER_LINE;
    }
}

void Robot_HeartBeat(void)
{
    ++hbCount;
    if (hbCount > HB_MAX)
    {
        hbLast ^= 1U;
        Toggle[hbLast](LED_HB);
        hbCount = 0;
    }
}
