#include "controller_learn.h"

#define AXIS_DELTA_MIN   20
#define JITTER_THRESHOLD 2

typedef struct
{
    uint8_t report_len;

    uint8_t idle[LEARN_MAX_REPORT];
    uint8_t sample[LEARN_MAX_REPORT];

} learn_state_t;

static learn_state_t __xdata learn;

uint8_t controller_learning_active = 0;
uint8_t controller_learning_step = 0;


/* ------------------------------------------------ */
/* reset learning state */
/* ------------------------------------------------ */

void controller_learn_reset(void)
{
    uint8_t i;

    learn.report_len = 0;

    for(i = 0; i < LEARN_MAX_REPORT; i++)
    {
        learn.idle[i] = 0;
        learn.sample[i] = 0;
    }
}


/* ------------------------------------------------ */
/* capture idle state */
/* ------------------------------------------------ */

void controller_learn_capture_idle(uint8_t *report, uint8_t len)
{
    uint8_t i;

    if(len > LEARN_MAX_REPORT)
        len = LEARN_MAX_REPORT;

    learn.report_len = len;

    for(i = 0; i < len; i++)
        learn.idle[i] = report[i];
}


/* ------------------------------------------------ */
/* capture active sample */
/* ------------------------------------------------ */

void controller_learn_capture_sample(uint8_t *report)
{
    uint8_t i;

    for(i = 0; i < learn.report_len; i++)
        learn.sample[i] = report[i];
}


/* ------------------------------------------------ */
/* analyze difference */
/* ------------------------------------------------ */

uint8_t controller_learn_analyze(control_map_t *map)
{
    uint8_t i;

    int16_t delta;
    int16_t best_delta = 0;
    uint8_t best_index = 0xFF;

    /* ------------------------------------------------ */
    /* PASS 1: detect AXIS movement                     */
    /* ------------------------------------------------ */

    for(i = 0; i < learn.report_len; i++)
    {
        delta = (int16_t)learn.sample[i] - (int16_t)learn.idle[i];

        /* ignore tiny jitter */
        if(delta > -2 && delta < 2)
            continue;

        if(delta > best_delta || delta < -best_delta)
        {
            best_delta = delta;
            best_index = i;
        }
    }

    if(best_index != 0xFF)
    {
        if(best_delta > 20 || best_delta < -20)
        {
            map->offset = best_index;

            CTRL_SET_AXIS(map, (best_delta < 0) ? AXIS_NEG : AXIS_POS);

            /* center = idle value */
            map->threshold = learn.idle[best_index];
            map->mask = 0;

            return 1;
        }
    }

    /* ------------------------------------------------ */
    /* PASS 2: detect BUTTON changes                    */
    /* ------------------------------------------------ */

    for(i = 0; i < learn.report_len; i++)
    {
        uint8_t idle = learn.idle[i];
        uint8_t sample = learn.sample[i];

        uint8_t diff = idle ^ sample;

        if(diff == 0)
            continue;

        /* single bit change ? button */

        if(diff && !(diff & (diff - 1)))
        {
            map->offset = i;
            CTRL_SET_BUTTON(map);
            map->mask = diff;
            map->threshold = 0;

            return 1;
        }
    }

    return 0;
}

/* ------------------------------------------------ */
/* learning process */
/* ------------------------------------------------ */

void controller_learn_process(uint8_t *report, uint8_t len)
{
    static control_map_t map;

    switch(controller_learning_step)
    {
        case 0:

            controller_learn_capture_idle(report, len);

            break;

        default:

            controller_learn_capture_sample(report);

            if(controller_learn_analyze(&map))
            {
                /* mapping gevonden */
            }

            break;
    }
}