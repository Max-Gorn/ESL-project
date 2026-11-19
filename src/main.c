#include <stdbool.h>
#include "app_config.h"
#include "blinker.h"

static const int blinks_count_list[] = ID; // ID: #xxxx

int main(void)
{
    setup();
    while (true)
    {
        blink_with_id(blinks_count_list);
    }
}
