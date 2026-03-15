#ifndef CONTROLLER_LEARN_H
#define CONTROLLER_LEARN_H

#include <stdint.h>

#define LEARN_MAX_REPORT 16

/* input type */
#define CTRL_INPUT_BUTTON  0
#define CTRL_INPUT_AXIS    1
/* input axis */
#define AXIS_NEG 0
#define AXIS_POS 1

/* flag masks */
#define CTRL_FLAG_TYPE 0x01
#define CTRL_FLAG_AXIS 0x02

/* helpers */
#define CTRL_SET_AXIS(map, dir) ((map)->flags = CTRL_FLAG_TYPE | ((dir) << 1))
#define CTRL_SET_BUTTON(map) ((map)->flags = CTRL_INPUT_BUTTON)
#define CTRL_IS_AXIS(map) ((map)->flags & CTRL_FLAG_TYPE)
#define CTRL_IS_BUTTON(map) (!((map)->flags & CTRL_FLAG_TYPE))
#define CTRL_AXIS_DIR(map) (((map)->flags >> 1) & 1)

typedef struct
{
    uint8_t offset;
    uint8_t mask;
    uint8_t threshold;
    uint8_t flags; // bit0-1: input type, bit2: axis_dir
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