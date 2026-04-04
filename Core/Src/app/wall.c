#include "wall.h"
#include "Inc/drivers/ir_interface.h"

static bool initialized = false;
void wall_init(void) {
    ASSERT(!initialized, ASSERT_APP_LEVEL);
    ir_init();
    initialized = true;
}

wall_info_t wall_detect(void) {
    
}