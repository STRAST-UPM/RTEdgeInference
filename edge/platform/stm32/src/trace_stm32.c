#include "trace.h"
#include "platform_api.h"
#include "stm32_hw_params.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

/* Minimal register-level USART3 TX on STM32H7xx (NUCLEO virtual COM over ST-LINK). */
#define RCC_BASE              0x58024400u
#define RCC_CR                (*(volatile uint32_t *)(RCC_BASE + 0x00u))
#define RCC_AHB4ENR           (*(volatile uint32_t *)(RCC_BASE + 0xE0u))
#define RCC_APB1LENR          (*(volatile uint32_t *)(RCC_BASE + 0xE8u))
#define RCC_D2CCIP2R          (*(volatile uint32_t *)(RCC_BASE + 0x54u))

#define GPIOD_BASE            0x58020C00u
#define GPIOD_MODER           (*(volatile uint32_t *)(GPIOD_BASE + 0x00u))
#define GPIOD_AFRH            (*(volatile uint32_t *)(GPIOD_BASE + 0x24u))

#define USART3_BASE           0x40004800u
#define USART3_CR1            (*(volatile uint32_t *)(USART3_BASE + 0x00u))
#define USART3_BRR            (*(volatile uint32_t *)(USART3_BASE + 0x0Cu))
#define USART3_ISR            (*(volatile uint32_t *)(USART3_BASE + 0x1Cu))
#define USART3_TDR            (*(volatile uint32_t *)(USART3_BASE + 0x28u))

#define RCC_AHB4ENR_GPIODEN   (1u << 3)
#define RCC_APB1LENR_USART3EN (1u << 18)

#define RCC_CR_HSION          (1u << 0)
#define RCC_CR_HSIRDY         (1u << 2)

/* USART3 kernel clock selector (USART234578SEL bits [2:0]). */
#define RCC_D2CCIP2R_USART28SEL_Pos   0u
#define RCC_D2CCIP2R_USART28SEL_Msk   (0x7u << RCC_D2CCIP2R_USART28SEL_Pos)
#define RCC_D2CCIP2R_USART28SEL_HSI   (0x3u << RCC_D2CCIP2R_USART28SEL_Pos)

#define HSI_CLOCK_HZ          64000000u

#define USART_CR1_UE          (1u << 0)
#define USART_CR1_TE          (1u << 3)
#define USART_ISR_TXE_TXFNF   (1u << 7)

static int s_trace_ready = 0;

static uint32_t irq_save_disable(void) {
    uint32_t primask;
    __asm volatile ("mrs %0, primask" : "=r" (primask));
    __asm volatile ("cpsid i" ::: "memory");
    return primask;
}

static void irq_restore(uint32_t primask) {
    __asm volatile ("msr primask, %0" :: "r" (primask) : "memory");
}

void trace_init(int baud_rate) {
    if (baud_rate <= 0) {
        baud_rate = 115200;
    }

    RCC_AHB4ENR |= RCC_AHB4ENR_GPIODEN;
    RCC_APB1LENR |= RCC_APB1LENR_USART3EN;

    /* Force USART3 kernel clock source to HSI for deterministic baud setup. */
    RCC_CR |= RCC_CR_HSION;
    while ((RCC_CR & RCC_CR_HSIRDY) == 0u) {
    }
    RCC_D2CCIP2R = (RCC_D2CCIP2R & ~RCC_D2CCIP2R_USART28SEL_Msk)
        | RCC_D2CCIP2R_USART28SEL_HSI;

    /* PD8 -> USART3_TX (AF7): MODER8=10, AFRH0=0111 */
    GPIOD_MODER &= ~(0x3u << (8u * 2u));
    GPIOD_MODER |= (0x2u << (8u * 2u));
    GPIOD_AFRH &= ~(0xFu << 0u);
    GPIOD_AFRH |= (0x7u << 0u);

    USART3_CR1 = 0u;
    uint32_t usart_clk_hz = HSI_CLOCK_HZ;
    USART3_BRR = (usart_clk_hz + ((uint32_t)baud_rate / 2u)) / (uint32_t)baud_rate;
    USART3_CR1 = USART_CR1_UE | USART_CR1_TE;

    s_trace_ready = 1;
}

static void usart3_write_byte(uint8_t byte) {
    while ((USART3_ISR & USART_ISR_TXE_TXFNF) == 0u) {
    }
    USART3_TDR = (uint32_t)byte;
}

static void uart_write_str(const char *s) {
    if (!s) {
        return;
    }
    while (*s != '\0') {
        usart3_write_byte((uint8_t)*s);
        s++;
    }
}

static void uart_write_u64(uint64_t v) {
    char tmp[21];
    int i = 0;
    if (v == 0u) {
        usart3_write_byte((uint8_t)'0');
        return;
    }
    while (v > 0u && i < (int)sizeof(tmp)) {
        tmp[i++] = (char)('0' + (v % 10u));
        v /= 10u;
    }
    while (i > 0) {
        usart3_write_byte((uint8_t)tmp[--i]);
    }
}

static void uart_write_i32(int32_t v) {
    if (v < 0) {
        usart3_write_byte((uint8_t)'-');
        uart_write_u64((uint64_t)(-(int64_t)v));
    } else {
        uart_write_u64((uint64_t)v);
    }
}

static void uart_write_csv_sep(void) {
    usart3_write_byte((uint8_t)',');
}

void trace_emit(uint32_t ev, int32_t model, uint64_t tu, uint64_t a, uint64_t b, uint64_t c) {
    if (!s_trace_ready) {
        return;
    }

    uint32_t primask = irq_save_disable();
    uint64_t ts = platform_now_us();
    uart_write_u64(ts);
    uart_write_csv_sep();
    uart_write_u64((uint64_t)ev);
    uart_write_csv_sep();
    uart_write_i32(model);
    uart_write_csv_sep();
    uart_write_u64(tu);
    uart_write_csv_sep();
    uart_write_u64(a);
    uart_write_csv_sep();
    uart_write_u64(b);
    uart_write_csv_sep();
    uart_write_u64(c);
    uart_write_str("\n");
    irq_restore(primask);
}