#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdint.h>

typedef enum {
    ASSERT_APP_LEVEL,
    ASSERT_DRIVER_LEVEL,
    ASSERT_PERIPHERAL_LEVEL
} assert_type_e;

#define ASSERT(expr, type) \
    do { \
        if (!(expr)) { \
            uint32_t pc; \
            asm volatile("mov %0, pc\n" : "=r" (pc)); \
            assert_handler(pc, type); \
        } \
    } while (0);

/**
 * @brief logs where assert took place and toggles LED corresponding to type
 * 
 * @param[in] pc_val uint32_t current program counter value
 * @param[in] type  uint32_t defines type of assert
 */
void assert_handler(uint32_t pc_val, assert_type_e type);

#endif