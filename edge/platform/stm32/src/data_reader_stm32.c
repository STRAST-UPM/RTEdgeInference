#include "data_reader.h"

#include <stdint.h>

#include "config.h"

#if USE_SERIAL_READER

#include <stdlib.h>
#include <string.h>

/* USART3 RX on PD9 (NUCLEO virtual COM RX). */
#define RCC_BASE                           0x58024400u
#define RCC_CR                             (*(volatile uint32_t *)(RCC_BASE + 0x00u))
#define RCC_AHB4ENR                        (*(volatile uint32_t *)(RCC_BASE + 0xE0u))
#define RCC_APB1LENR                       (*(volatile uint32_t *)(RCC_BASE + 0xE8u))
#define RCC_D2CCIP2R                       (*(volatile uint32_t *)(RCC_BASE + 0x54u))

#define GPIOD_BASE                         0x58020C00u
#define GPIOD_MODER                        (*(volatile uint32_t *)(GPIOD_BASE + 0x00u))
#define GPIOD_AFRH                         (*(volatile uint32_t *)(GPIOD_BASE + 0x24u))

#define USART3_BASE                        0x40004800u
#define USART3_CR1                         (*(volatile uint32_t *)(USART3_BASE + 0x00u))
#define USART3_BRR                         (*(volatile uint32_t *)(USART3_BASE + 0x0Cu))
#define USART3_ISR                         (*(volatile uint32_t *)(USART3_BASE + 0x1Cu))
#define USART3_RDR                         (*(volatile uint32_t *)(USART3_BASE + 0x24u))

#define NVIC_ISER1                         (*(volatile uint32_t *)0xE000E104u)

#define RCC_AHB4ENR_GPIODEN                (1u << 3)
#define RCC_APB1LENR_USART3EN              (1u << 18)

#define RCC_CR_HSION                       (1u << 0)
#define RCC_CR_HSIRDY                      (1u << 2)

/* USART3 kernel clock selector (USART234578SEL bits [2:0]). */
#define RCC_D2CCIP2R_USART28SEL_Pos         0u
#define RCC_D2CCIP2R_USART28SEL_Msk         (0x7u << RCC_D2CCIP2R_USART28SEL_Pos)
#define RCC_D2CCIP2R_USART28SEL_HSI         (0x3u << RCC_D2CCIP2R_USART28SEL_Pos)

#define HSI_CLOCK_HZ                       64000000u

#define USART_CR1_UE                       (1u << 0)
#define USART_CR1_RE                       (1u << 2)
#define USART_CR1_TE                       (1u << 3)
#define USART_CR1_RXNEIE_RXFNEIE           (1u << 5)
#define USART_ISR_RXNE_RXFNE               (1u << 5)

#define USART3_IRQN                         39u

#define RX_RING_SIZE                        2048u

static int s_reader_ready = 0;
static char s_line_buf[1024];
static size_t s_line_len = 0;
static int s_line_overflow = 0;
static volatile uint8_t s_rx_ring[RX_RING_SIZE];
static volatile uint16_t s_rx_head = 0;
static volatile uint16_t s_rx_tail = 0;

static inline uint16_t ring_next(uint16_t idx) {
    return (uint16_t)((idx + 1u) & (RX_RING_SIZE - 1u));
}

static inline void ring_push(uint8_t ch) {
    uint16_t head = s_rx_head;
    uint16_t next = ring_next(head);

    if (next == s_rx_tail) {
        /* Overflow: drop oldest byte to keep newest data. */
        s_rx_tail = ring_next(s_rx_tail);
    }

    s_rx_ring[head] = ch;
    s_rx_head = next;
}

static inline int ring_pop(uint8_t *out) {
    uint16_t tail = s_rx_tail;
    if (tail == s_rx_head) {
        return 0;
    }

    *out = s_rx_ring[tail];
    s_rx_tail = ring_next(tail);
    return 1;
}

void USART3_IRQHandler(void) {
    while ((USART3_ISR & USART_ISR_RXNE_RXFNE) != 0u) {
        ring_push((uint8_t)(USART3_RDR & 0xFFu));
    }
}

static void serial_reader_init(void) {
    if (s_reader_ready) {
        return;
    }

    RCC_AHB4ENR |= RCC_AHB4ENR_GPIODEN;
    RCC_APB1LENR |= RCC_APB1LENR_USART3EN;

    /* Force USART3 kernel clock source to HSI for deterministic baud setup. */
    RCC_CR |= RCC_CR_HSION;
    while ((RCC_CR & RCC_CR_HSIRDY) == 0u) {
    }
    RCC_D2CCIP2R = (RCC_D2CCIP2R & ~RCC_D2CCIP2R_USART28SEL_Msk)
        | RCC_D2CCIP2R_USART28SEL_HSI;

    /* PD9 -> USART3_RX (AF7): MODER9=10, AFRH1=0111 */
    GPIOD_MODER &= ~(0x3u << (9u * 2u));
    GPIOD_MODER |= (0x2u << (9u * 2u));
    GPIOD_AFRH &= ~(0xFu << 4u);
    GPIOD_AFRH |= (0x7u << 4u);

    /* Keep TX enabled (trace_init configures it) and ensure RX is enabled too. */
    USART3_BRR = (HSI_CLOCK_HZ + ((uint32_t)SERIAL_BAUD / 2u)) / (uint32_t)SERIAL_BAUD;
    USART3_CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE_RXFNEIE);
    NVIC_ISER1 = (1u << (USART3_IRQN - 32u));

    s_reader_ready = 1;
}

size_t read_events(event_t *buffer, size_t max_count) {
    serial_reader_init();

    /*
     * Non-blocking path: consume bytes already captured by ISR. If no complete
     * line is available yet, return 0 and continue on next TU.
     */
    uint8_t ch;
    while (ring_pop(&ch) != 0) {
        if (ch == '\r') {
            continue;
        }

        if (ch == '\n') {
            if (s_line_overflow != 0) {
                s_line_len = 0;
                s_line_overflow = 0;
                continue;
            }
            if (s_line_len == 0u) {
                continue;
            }

            s_line_buf[s_line_len] = '\0';

            size_t count = 0;
            char *tok = strtok(s_line_buf, ",");
            while (tok != NULL && count < max_count) {
                long v = strtol(tok, NULL, 10);
                buffer[count++] = (event_t)v;
                tok = strtok(NULL, ",");
            }

            s_line_len = 0;
            return count;
        }

        if (s_line_overflow != 0) {
            continue;
        }

        if (s_line_len + 1u < sizeof(s_line_buf)) {
            s_line_buf[s_line_len++] = (char)ch;
        } else {
            /* Drop overly long line until next newline resynchronizes. */
            s_line_len = 0;
            s_line_overflow = 1;
        }
    }

    return 0;
}

#else

#include "memory_events.h"

#include <string.h>

static size_t s_mem_index = 0;

size_t read_events(event_t *buffer, size_t max_count) {
    if (s_mem_index >= memory_events_count) {
        return 0;
    }

    size_t len = memory_events_lengths[s_mem_index];
    if (len > max_count) {
        len = max_count;
    }

    memcpy(buffer, memory_events[s_mem_index], len * sizeof(event_t));
    s_mem_index++;
    return len;
}

#endif
