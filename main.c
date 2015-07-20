#include <stdint.h>
#include "simpletools.h"
#include "abdrive.h"   

#define CYCLES_PER_MS       1000
#define HB_MAX              CYCLES_PER_MS * 10
#define PIN_HB              26
#define PIN_RIGHT_ENCODER   15
#define PIN_LEFT_ENCODER    14
#define PIN_CNY_1           13
#define PIN_CNY_0           12
#define IN_MAX              8
#define COGS_MAX            8

typedef enum
{
    ST_IDLE,
    ST_INPUTS,
    ST_MOVE
} State;

typedef enum
{
    MV_FORWARDS,
    MV_BACKWARDS
} Move;

void Process_Idle(void* data);
void Process_CheckInputs(void* data);
void Process_Move(void* data);
void Move_Toggle(void* data);
void doHeartBeat(void);

uint32_t heartBeatCount = 0U;
uint8_t heartBeatLast = 0U;

int* cog[COGS_MAX];
uint8_t inPins[] = {8, 9, 10, 11, 12, 13, 14, 15};
volatile uint8_t inputs[IN_MAX];

volatile State state = ST_IDLE;
volatile Move lastMove = MV_FORWARDS;

typedef void(*ProcessCB)(void* data);
ProcessCB Process[] = {Process_Idle, Process_CheckInputs, Process_Move};

void(*Toggle[])(int pin) = {high, low};

int main(void)
{
    state = ST_IDLE;

    cog[0] = cog_run(Process_CheckInputs, 128);
    cog[1] = cog_run(Process_Move, 128);
    cog[2] = cog_run(Move_Toggle, 128);

    while(1)
    {
        Process[state];

        doHeartBeat();
    }
    return 0;
}

void Process_Idle(void* data)
{
    pause(8000);
    drive_speed(32, 32);
    state = ST_INPUTS;
}

void Process_CheckInputs(void* data)
{
    uint8_t i;
    for(i = 0; i < IN_MAX; ++i)
    {
        inputs[i] = input(inPins[i]);
    }

    state = ST_MOVE;
}

void Process_Move(void* data)
{
    uint8_t lineSensed = !(inputs[PIN_CNY_0] && inputs[PIN_CNY_1]);
    if(lineSensed)
    {
        Move_Toggle(0);
    }
    
    state = ST_INPUTS;
}

void Move_Toggle(void* data)
{
    if(lastMove == MV_FORWARDS)
    {
        drive_speed(64, 64);
        lastMove == MV_FORWARDS;
    }
    else
    {
        drive_speed(-64, -64);
        lastMove = MV_BACKWARDS;
    }
}

void doHeartBeat(void)
{
    heartBeatCount++;
    if (heartBeatCount > HB_MAX)
    {
        // toogle
        heartBeatLast ^= 1U;
        Toggle[heartBeatLast](PIN_HB);
        heartBeatCount = 0;
    }
}
