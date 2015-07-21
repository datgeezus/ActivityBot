#include <stdint.h>
#include "simpletools.h"
#include "abdrive.h"   

#define CYCLES_PER_MS       100
#define HB_MAX              CYCLES_PER_MS * 10
#define PIN_HB              26
#define PIN_SENSE           27

#define PIN_RIGHT_ENCODER   7 // 15
#define PIN_LEFT_ENCODER    6 // 14
#define PIN_PING            2 // 10
#define PIN_CNY_1           1 // 9
#define PIN_CNY_0           0 // 8
#define IN_MAX              8

#define COGS_MAX            8

typedef enum
{
    ST_MOVE,
    ST_OVER_LINE
} State;

typedef enum
{
    MV_FORWARDS,
    MV_BACKWARDS
} Move;

void Init();
void CheckInputs(void* data);

void Process_Move(void* data);
void Process_OverLine(void* data);

void Move_Toggle(void* data);
void doHeartBeat(void);

uint32_t heartBeatCount = 0U;
uint8_t heartBeatLast = 0U;

int* cog[COGS_MAX];
uint8_t inPins[] = {8, 9, 10, 11, 12, 13, 14, 15};
volatile uint8_t inputs[IN_MAX];

volatile State state = ST_MOVE;
volatile Move lastMove = MV_FORWARDS;

typedef void(*ProcessCB)(void* data);
ProcessCB Process[] = {Process_Move, Process_OverLine};

void(*Toggle[])(int pin) = {low, high};

int main(void)
{
    state = ST_MOVE;

    //cog[0] = cog_run(Process_CheckInputs, 128);
    //cog[1] = cog_run(Process_Move, 128);
    //cog[2] = cog_run(Move_Toggle, 128);

    Init();

    while(1)
    {
        CheckInputs(0);
        Process[state](0);

        doHeartBeat();
    }
    return 0;
}

void Init(void* data)
{
    pause(8000);
    drive_speed(64, 64);
    /* high(PIN_SENSE); */
    /* low(PIN_HB); */
}

void CheckInputs(void* data)
{
    uint8_t i;
    for(i = 0; i < IN_MAX; ++i)
    {
        inputs[i] = input(inPins[i]);
    }
    
    /* Toggle[inputs[PIN_CNY_0]](PIN_SENSE); */
    /* Toggle[inputs[PIN_CNY_1]](PIN_HB); */
    //state = ST_MOVE;
}

void Process_Move(void* data)
{
    if(!(inputs[PIN_CNY_0]) && !(inputs[PIN_CNY_1]))
    {
        Move_Toggle(0);
    }
}

void Move_Toggle(void* data)
{
    if(MV_BACKWARDS == lastMove)
    {
        drive_speed(64, 64);
        /* high(PIN_SENSE); */
        /* low(PIN_HB); */
        lastMove = MV_FORWARDS;
    }
    else
    {
        drive_speed(-64, -64);
        /* low(PIN_SENSE); */
        /* high(PIN_HB); */
        lastMove = MV_BACKWARDS;
    }

    state = ST_OVER_LINE;
}

void Process_OverLine(void* data)
{
    if(inputs[PIN_CNY_0] && inputs[PIN_CNY_1])
    {
        high(PIN_SENSE);
        state = ST_MOVE;
    }
    else
    {
        low(PIN_SENSE);
        state = ST_OVER_LINE;
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
