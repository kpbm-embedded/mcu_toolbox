/* Single-producer/single-consumer byte ring buffer; no hardware access (host-testable). */
#pragma once
#include <stddef.h>
#include <stdint.h>
typedef struct { uint8_t *buf; size_t size; volatile size_t head, tail; } ringbuf_t;
void ringbuf_init(ringbuf_t *rb, uint8_t *storage, size_t size);   /* holds size - 1 bytes */
int ringbuf_put(ringbuf_t *rb, uint8_t byte);                     /* 1 ok, 0 full */
int ringbuf_get(ringbuf_t *rb, uint8_t *byte);                    /* 1 ok, 0 empty */
size_t ringbuf_count(const ringbuf_t *rb);
