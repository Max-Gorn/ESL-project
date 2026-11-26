#ifndef BLINKER_BUTTON_H
#define BLINKER_BUTTON_H

typedef enum button_click {
    NONE,
    CLICK,
    HOLDING,
    DOUBLE_CLICK
} button_click_t;

void setup_button();
button_click_t read_button_state();

#endif