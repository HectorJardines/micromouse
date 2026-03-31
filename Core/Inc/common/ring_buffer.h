#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

struct ring_buffer
{
    uint8_t *buf;
    uint8_t buf_size;
    uint8_t elem_size;
    uint8_t head;
    uint8_t tail;
};

#define RING_BUFFER(name, size, type, storage)                                                     \
    static_assert(size < UINT8_MAX);                                                               \
    storage uint8_t name##_buffer[size * sizeof(type)] = { 0 };                                    \
    storage struct ring_buffer name = { .buf = name##_buffer,                                   \
                                        .buf_size = size,                                       \
                                        .elem_size = sizeof(type) }                             \

#define STATIC_RING_BUFFER(name, size, type) RING_BUFFER(name, size, type, static)

/**
 * @brief pushes data value into ring_buffer struct
 * 
 * Uses memcpy to copy bytes from data pointer into the ring_buffer struct's
 * underlying memory buffer. pops the most oldest value from the buffer if
 * the buffer is full.
 * 
 * @param[in] rb struct ring_bufffer *
 * @param[in] data const void *
 */
void ring_buffer_push(struct ring_buffer *rb, const void *data);

/**
 * @brief removes the oldest element from the ring buffer
 * 
 * This function pops the oldest element from the ring_buffer struct, rb.
 * Uses memcpy to copy the value of the oldest element into the 
 * data pointer. Checks that the ring_buffer is not empty before removing.
 * 
 * @param[in] rb struct ring_bufffer *
 * @param[in] data const void *
 */
void ring_buffer_pop(struct ring_buffer *rb, void *data);

/**
 * @brief copies the element at the tail of the list into the data pointer
 * 
 * 
 * @param[in] rb struct ring_bufffer *
 * @param[in] data const void *
 */
void ring_buffer_peek_tail(const struct ring_buffer *rb, void *data);

/**
 * @brief copies the element at the head of the ring buffer into data pointer
 * 
 * @param[in] rb struct ring_bufffer *
 * @param[in] data const void *
 */
void ring_buffer_peek_head(const struct ring_buffer *rb, void *data, uint8_t offset);

/**
 * @brief Returns whether the ring buffer is full or not
 * 
 * @param[in] rb struct ring_bufffer *
 */
bool ring_buffer_full(const struct ring_buffer *rb);

/**
 * @brief Returns whether the ring buffer is empty or not
 * 
 * @param[in] rb struct ring_bufffer *
 */
bool ring_buffer_empty(const struct ring_buffer *rb);

/**
 * @brief Returns the number of elements in the ring buffer
 * 
 * @param[in] rb struct ring_bufffer *
 */
uint8_t ring_buffer_count(const struct ring_buffer *rb);

#endif