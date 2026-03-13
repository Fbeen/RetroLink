#ifndef CONTROLLER_LEARN_H
#define CONTROLLER_LEARN_H

#include <stdint.h>

#define LEARN_MAX_REPORT 16

typedef enum
{
    CTRL_INPUT_NONE = 0,
    CTRL_INPUT_BUTTON,
    CTRL_INPUT_AXIS
} ctrl_input_type_t;

#define AXIS_NEG 0
#define AXIS_POS 1

typedef struct
{
    uint8_t offset;
    uint8_t mask;
    uint8_t threshold;
    ctrl_input_type_t type;
    uint8_t axis_dir;
} control_map_t;

/* learning state flags */
extern uint8_t controller_learning_active;
extern uint8_t controller_learning_step;

/* last report */
extern uint8_t __xdata g_last_report[];
extern uint8_t g_last_report_len;

/* API */
void controller_learn_reset(void);
void controller_learn_capture_idle(uint8_t *report, uint8_t len);
void controller_learn_capture_sample(uint8_t *report);
uint8_t controller_learn_analyze(control_map_t *map);
void controller_learn_process(uint8_t *report, uint8_t len);

#endif