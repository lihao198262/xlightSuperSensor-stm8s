#ifndef BUTTON_H_
#define BUTTON_H_

// Key list
typedef enum
{
    keylstCenter,
    keylstUp,
    keylstDown,
    keylstDummy
} keylist_t;

typedef enum button_timer_status_e
{
    BUTTON_STATUS_INIT = 0,
    BUTTON_STATUS_LESS_2S,
    BUTTON_STATUS_MORE_2S,
    BUTTON_STATUS_MORE_5S,
    BUTTON_STATUS_DOUBLE_TRACK
} button_timer_status_t;

typedef enum button_event_e
{
    BUTTON_INVALID = 0,
    BUTTON_SHORT_PRESS,
    BUTTON_DOUBLE_PRESS,
    BUTTON_LONG_HOLD,
    BUTTON_LONG_PRESS,
    BUTTON_VERY_LONG_HOLD,
    BUTTON_VERY_LONG_PRESS,
    DOUBLE_BTN_TRACK
} button_event_t;

#define IS_VALID_BUTTON(x)              ((x) >= keylstCenter && (x) < keylstDummy)

void button_event_handler();
void button_init(void);

#endif // BUTTON_H_
