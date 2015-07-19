#include <stdint.h>
#include "simpletools.h"

#define CYCLES_PER_MS   1000
#define HB_MAX          CYCLES_PER_MS * 10
#define HB_PIN          26

typedef enum State
{
    ST_IDLE,
    ST_AVOID,
};

typedef void(*ProcessCB)(void* data);

void Process_Avoid(void* data);
void Process_Idle(void* data);
void doHeartBeat(void);

uint32_t heartBeatCount = 0U;
uint8_t heartBeatLast = 0U;

State state = ST_IDLE;

ProcessCB Process[] = {Process_Idle, Process_Avoid};
void(*Toggle[])(int pin) = {high, low};

int main(void)
{
    state = ST_IDLE;

    while(1)
    {
        Process[state];

        doHeartBeat();
    }
    return 0;
}

void Process_Idle(void* data)
{
    pause(10000);
    state = ST_AVOID;
}

void Process_Avoid(void* data)
{
    return;
}

void doHeartBeat(void)
{
    heartBeatCount++;
    if (heartBeatCount > HB_MAX)
    {
        // toogle
        heartBeatLast ^= 1U;
        Toggle[heartBeatLast](HB_PIN);
        heartBeatCount = 0;
    }
}
