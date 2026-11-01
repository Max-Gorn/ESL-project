#include <stdbool.h>
#include "blinker.h"

#define BLINK_DELAY_MS 500

static const int blinks_count_list[] = { 6, 6, 1, 1 }; // ID: #xxxx

int main(void)
{
    setup();
    while (true)
    {
        blink_with_id(blinks_count_list, BLINK_DELAY_MS);
    }
}
