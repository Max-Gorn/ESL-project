#ifndef BLINKER_BUTTON_H
#define BLINKER_BUTTON_H

void setup_button();
void enable_button();
bool button_event_processor();
bool get_double_click_state();
void reset_double_click_state();
#endif