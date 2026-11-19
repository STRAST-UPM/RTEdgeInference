#include <stdint.h>

/* -----------------------------------------------------------------------
 * Target clock
 * -----------------------------------------------------------------------*/
#define SYSTEM_CORE_CLOCK_HZ    400000000u

/* -----------------------------------------------------------------------
 * PLL1 parameters — change M/N/P here, not inside system_clock_config().
 * -----------------------------------------------------------------------
 * Source : HSI64 (64 MHz)
 * M      : pre-divider  → VCO input  = HSI / M
 * N      : multiplier   → VCO output = VCO_in * N
 * P      : P-divider    → SYSCLK     = VCO / P
 * -----------------------------------------------------------------------*/
#define PLL1_M  4u
#define PLL1_N  50u
#define PLL1_P  2u

#define HSI_HZ          64000000u
#define PLL1_VCO_IN_HZ  (HSI_HZ / PLL1_M)
#define PLL1_VCO_HZ     (PLL1_VCO_IN_HZ * PLL1_N)
#define PLL1_SYSCLK_HZ  (PLL1_VCO_HZ    / PLL1_P)

/* -----------------------------------------------------------------------
 * Compile-time sanity checks (STM32H7 RM Rev8 §8.5.4)
 *
 * Wide  VCO (VCOSEL=0): VCO input 1-16 MHz,  VCO output 192-960 MHz
 * Medium VCO (VCOSEL=1): VCO input 2-16 MHz, VCO output 150-420 MHz
 *
 * PLL1_VCO_HZ = 800 MHz → ONLY wide VCO is valid.
 * Setting VCOSEL=1 with VCO > 420 MHz causes PLL lock failure and the
 * firmware hangs forever in wait_until() before producing any UART output.
 * -----------------------------------------------------------------------*/


#define RCC_BASE                0x58024400u
#define RCC_CR                  (*(volatile uint32_t *)(RCC_BASE + 0x000u))
#define RCC_CFGR                (*(volatile uint32_t *)(RCC_BASE + 0x010u))
#define RCC_D1CFGR              (*(volatile uint32_t *)(RCC_BASE + 0x018u))
#define RCC_D2CFGR              (*(volatile uint32_t *)(RCC_BASE + 0x01Cu))
#define RCC_D3CFGR              (*(volatile uint32_t *)(RCC_BASE + 0x020u))
#define RCC_PLLCKSELR           (*(volatile uint32_t *)(RCC_BASE + 0x028u))
#define RCC_PLLCFGR             (*(volatile uint32_t *)(RCC_BASE + 0x02Cu))
#define RCC_PLL1DIVR            (*(volatile uint32_t *)(RCC_BASE + 0x030u))
#define RCC_PLL1FRACR           (*(volatile uint32_t *)(RCC_BASE + 0x034u))
#define RCC_AHB4ENR             (*(volatile uint32_t *)(RCC_BASE + 0x0E0u))
#define RCC_AHB4ENR_PWREN       (1u << 28)

/* -----------------------------------------------------------------------
 * PWR — voltage scaling (D3 domain, always-on)
 *
 * STM32H753 power/frequency table (RM0433 §6.6.2):
 *   VOS3 (reset default) : SYSCLK ≤ 200 MHz
 *   VOS2                 : SYSCLK ≤ 300 MHz
 *   VOS1                 : SYSCLK ≤ 400 MHz   ← required for 400 MHz
 *   VOS0 (boost)         : SYSCLK ≤ 480 MHz (SMPS only; not used here)
 *
 * Failing to raise VOS before switching to the PLL causes silent CPU
 * malfunction — the core appears to boot but produces no UART output
 * (bytes_rx=0). This is the second half of the PLL bring-up bug.
 * -----------------------------------------------------------------------*/
#define PWR_BASE                0x58024800u
#define PWR_CR3                 (*(volatile uint32_t *)(PWR_BASE + 0x00Cu))
#define PWR_CR3_BYPASS          (1u << 0)
#define PWR_CR3_LDOEN           (1u << 1)
#define PWR_CSR1                (*(volatile uint32_t *)(PWR_BASE + 0x004u))
#define PWR_CSR1_ACTVOSRDY      (1u << 13)  /* active VOS ready (current supply OK) */
#define PWR_D3CR                (*(volatile uint32_t *)(PWR_BASE + 0x018u))
#define PWR_D3CR_VOS_Pos        14u
#define PWR_D3CR_VOS_MASK       (3u << PWR_D3CR_VOS_Pos)
#define PWR_D3CR_VOS1           (3u << PWR_D3CR_VOS_Pos)  /* Scale 1: up to 400 MHz */
#define PWR_D3CR_VOSRDY         (1u << 13)

#define RCC_APB1LENR            (*(volatile uint32_t *)(RCC_BASE + 0x0E8u))
#define RCC_APB1LENR_USART3EN   (1u << 18)
#define RCC_D2CCIP2R            (*(volatile uint32_t *)(RCC_BASE + 0x054u))
/* USART3 kernel clock selector (USART234578SEL bits [2:0]). */
#define RCC_D2CCIP2R_USART28_MSK (7u << 0)
#define RCC_D2CCIP2R_USART28_HSI (3u << 0)  /* 0b011 = HSI kernel clock */

/* GPIOD and USART3 for boot beacon (PD8 = USART3_TX, AF7) */
#define GPIOD_BASE_DG           0x58020C00u
#define GPIOD_MODER_DG          (*(volatile uint32_t *)(GPIOD_BASE_DG + 0x00u))
#define GPIOD_AFRH_DG           (*(volatile uint32_t *)(GPIOD_BASE_DG + 0x24u))
#define RCC_AHB4ENR_GPIODEN     (1u << 3)
#define USART3_BASE_DG          0x40004800u
#define USART3_CR1_DG           (*(volatile uint32_t *)(USART3_BASE_DG + 0x00u))
#define USART3_BRR_DG           (*(volatile uint32_t *)(USART3_BASE_DG + 0x0Cu))
#define USART3_ISR_DG           (*(volatile uint32_t *)(USART3_BASE_DG + 0x1Cu))
#define USART3_TDR_DG           (*(volatile uint32_t *)(USART3_BASE_DG + 0x28u))
#define USART_CR1_UE_DG         (1u << 0)
#define USART_CR1_TE_DG         (1u << 3)
#define USART_ISR_TXE_DG        (1u << 7)
/* BRR for 115200 baud with HSI64: (64000000 + 57600) / 115200 = 556 */
#define BOOT_BEACON_BRR         556u
/* Keep beacons visible long enough for host monitor attach after reset. */
#define BOOT_BEACON_REPEATS      64u
#define BOOT_BEACON_DELAY_ITERS  2000u

#define FLASH_BASE_ADDR         0x52002000u
#define FLASH_ACR               (*(volatile uint32_t *)(FLASH_BASE_ADDR + 0x000u))

#define RCC_CR_HSION            (1u << 0)
#define RCC_CR_HSIRDY           (1u << 2)
#define RCC_CR_HSIDIV_MASK      (3u << 3)
#define RCC_CR_PLL1ON           (1u << 24)
#define RCC_CR_PLL1RDY          (1u << 25)

#define RCC_CFGR_SW_MASK        (7u << 0)
#define RCC_CFGR_SW_PLL1        (3u << 0)
#define RCC_CFGR_SWS_MASK       (7u << 3)
#define RCC_CFGR_SWS_PLL1       (3u << 3)

/* On STM32H7 in this project bring-up, PLLSRC field must stay at reset value
 * for HSI. Writing +2 here keeps PLL1 from locking on H753ZI. */
#define RCC_PLLCKSELR_PLLSRC_HSI (0u << 0)
#define RCC_PLLCKSELR_DIVM1_Pos  4u

/* RCC_PLLCFGR_PLL1VCOSEL (bit 1) must remain CLEAR (=0) for wide VCO range.
 * 0 = wide range (192-960 MHz), 1 = medium range (150-420 MHz).
 * With PLL1_VCO_HZ = 800 MHz ONLY wide VCO is valid — do NOT set this bit. */
#define RCC_PLLCFGR_PLL1VCOSEL   (1u << 1)
#define RCC_PLLCFGR_PLL1RGE_Pos  2u
#define RCC_PLLCFGR_DIVP1EN      (1u << 16)
#define RCC_PLLCFGR_DIVQ1EN      (1u << 17)
#define RCC_PLLCFGR_DIVR1EN      (1u << 18)

#define RCC_D1CFGR_HPRE_MASK     (0xFu << 0)
#define RCC_D1CFGR_D1PPRE_MASK   (0x7u << 4)
#define RCC_D1CFGR_D1CPRE_MASK   (0xFu << 8)
#define RCC_D2CFGR_D2PPRE1_MASK  (0x7u << 4)
#define RCC_D2CFGR_D2PPRE2_MASK  (0x7u << 8)
#define RCC_D3CFGR_D3PPRE_MASK   (0x7u << 4)

/* Prescaler encodings for STM32H7 clock tree registers. */
#define RCC_D1CFGR_HPRE_DIV2     (0x8u << 0)
#define RCC_D1CFGR_D1PPRE_DIV2   (0x4u << 4)
#define RCC_D1CFGR_D1CPRE_DIV1   (0x0u << 8)
#define RCC_D2CFGR_D2PPRE1_DIV2  (0x4u << 4)
#define RCC_D2CFGR_D2PPRE2_DIV2  (0x4u << 8)
#define RCC_D3CFGR_D3PPRE_DIV2   (0x4u << 4)

#define FLASH_ACR_LATENCY_MASK   0xFu
#define FLASH_ACR_LATENCY_7WS    7u

uint32_t SystemCoreClock = SYSTEM_CORE_CLOCK_HZ;

static void wait_until(volatile uint32_t *reg, uint32_t mask, uint32_t value) {
    while ((*reg & mask) != value) {
    }
}

/*
 * boot_beacon — send one ASCII character via USART3 (PD8, 115200 baud, HSI64)
 * before any clock change takes effect.  Safe to call from SystemInit because
 * it uses only volatile hardware registers (no static state, no libc).
 * Clocks enabled here are idempotent with trace_init() called later.
 */
static void boot_beacon(char c) {
    uint32_t i;
    uint32_t j;

    /* Enable GPIOD and USART3 clocks */
    RCC_AHB4ENR |= RCC_AHB4ENR_GPIODEN;
    RCC_APB1LENR |= RCC_APB1LENR_USART3EN;
    /* Force USART3 kernel clock to HSI (stays correct even after PLL switch) */
    RCC_D2CCIP2R = (RCC_D2CCIP2R & ~RCC_D2CCIP2R_USART28_MSK) | RCC_D2CCIP2R_USART28_HSI;
    /* PD8 -> USART3_TX: alternate function mode (10) + AF7 */
    GPIOD_MODER_DG = (GPIOD_MODER_DG & ~(3u << 16u)) | (2u << 16u);
    GPIOD_AFRH_DG  = (GPIOD_AFRH_DG  & ~(0xFu <<  0u)) | (7u <<  0u);
    /* (Re)configure USART3 each time to handle first call correctly */
    USART3_CR1_DG = 0u;
    USART3_BRR_DG = BOOT_BEACON_BRR;
    USART3_CR1_DG = USART_CR1_UE_DG | USART_CR1_TE_DG;
    /* Transmit the beacon repeatedly so host capture does not miss startup bytes. */
    for (i = 0u; i < BOOT_BEACON_REPEATS; ++i) {
        while ((USART3_ISR_DG & USART_ISR_TXE_DG) == 0u) {}
        USART3_TDR_DG = (uint32_t)(uint8_t)c;
        for (j = 0u; j < BOOT_BEACON_DELAY_ITERS; ++j) {
            __asm volatile("nop");
        }
    }
    while ((USART3_ISR_DG & USART_ISR_TXE_DG) == 0u) {}
}

static void system_clock_config(void) {
    boot_beacon('A');  /* reached system_clock_config */

    /* Start from HSI64 / 1 before touching PLL settings. */
    RCC_CR |= RCC_CR_HSION;
    wait_until(&RCC_CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY);
    RCC_CR &= ~RCC_CR_HSIDIV_MASK; /* HSI / 1 */
    boot_beacon('B');

    /* Keep SYSCLK on HSI while reprogramming PLL1. */
    RCC_CFGR &= ~RCC_CFGR_SW_MASK;
    wait_until(&RCC_CFGR, RCC_CFGR_SWS_MASK, 0u);
    boot_beacon('C');

    /* Ensure power domain is ready for high-frequency operation. */
    RCC_AHB4ENR |= RCC_AHB4ENR_PWREN;
    boot_beacon('1');
    PWR_CR3 |= PWR_CR3_LDOEN;
    PWR_CR3 &= ~PWR_CR3_BYPASS;
    boot_beacon('2');
    wait_until(&PWR_CSR1, PWR_CSR1_ACTVOSRDY, PWR_CSR1_ACTVOSRDY);
    boot_beacon('3');
    PWR_D3CR = (PWR_D3CR & ~PWR_D3CR_VOS_MASK) | PWR_D3CR_VOS1;
    boot_beacon('4');
    wait_until(&PWR_D3CR, PWR_D3CR_VOSRDY, PWR_D3CR_VOSRDY);
    boot_beacon('5');

    /* Set conservative bus prescalers before switching SYSCLK to PLL1. */
    RCC_D1CFGR = (RCC_D1CFGR & ~(RCC_D1CFGR_HPRE_MASK | RCC_D1CFGR_D1PPRE_MASK | RCC_D1CFGR_D1CPRE_MASK))
               | RCC_D1CFGR_HPRE_DIV2 | RCC_D1CFGR_D1PPRE_DIV2 | RCC_D1CFGR_D1CPRE_DIV1;
    RCC_D2CFGR = (RCC_D2CFGR & ~(RCC_D2CFGR_D2PPRE1_MASK | RCC_D2CFGR_D2PPRE2_MASK))
               | RCC_D2CFGR_D2PPRE1_DIV2 | RCC_D2CFGR_D2PPRE2_DIV2;
    RCC_D3CFGR = (RCC_D3CFGR & ~RCC_D3CFGR_D3PPRE_MASK) | RCC_D3CFGR_D3PPRE_DIV2;
    boot_beacon('6');

    /* Program FLASH wait states for 400 MHz SYSCLK. */
    FLASH_ACR = (FLASH_ACR & ~FLASH_ACR_LATENCY_MASK) | FLASH_ACR_LATENCY_7WS;
    boot_beacon('7');

    /* Reconfigure PLL1 from HSI64: M=4, N=50, P=2 -> 400 MHz SYSCLK. */
    RCC_CR &= ~RCC_CR_PLL1ON;
    wait_until(&RCC_CR, RCC_CR_PLL1RDY, 0u);
    boot_beacon('8');

    RCC_PLLCKSELR = RCC_PLLCKSELR_PLLSRC_HSI | (PLL1_M << RCC_PLLCKSELR_DIVM1_Pos);
    /* VCOSEL=0 (wide range 192-960 MHz) — do NOT set bit 1; VCO=800 MHz exceeds medium range.
     * RGE=0b11 → VCO input 8-16 MHz; with M=4: VCO_in = 64/4 = 16 MHz (upper boundary, valid). */
    RCC_PLLCFGR = (RCC_PLLCFGR & ~(RCC_PLLCFGR_PLL1VCOSEL | (0x3u << RCC_PLLCFGR_PLL1RGE_Pos)))
                | (0x3u << RCC_PLLCFGR_PLL1RGE_Pos)
                | RCC_PLLCFGR_DIVP1EN | RCC_PLLCFGR_DIVQ1EN | RCC_PLLCFGR_DIVR1EN;

    RCC_PLL1DIVR = ((PLL1_N - 1u) << 0)
                 | ((PLL1_P - 1u) << 9)
                 | ((2u - 1u) << 16)
                 | ((2u - 1u) << 24);
    RCC_PLL1FRACR = 0u;
    boot_beacon('9');

    RCC_CR |= RCC_CR_PLL1ON;
    wait_until(&RCC_CR, RCC_CR_PLL1RDY, RCC_CR_PLL1RDY);
    boot_beacon('a');

    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MASK) | RCC_CFGR_SW_PLL1;
    wait_until(&RCC_CFGR, RCC_CFGR_SWS_MASK, RCC_CFGR_SWS_PLL1);
    boot_beacon('D');

    SystemCoreClock = PLL1_SYSCLK_HZ;
    boot_beacon('E');
}

void SystemCoreClockUpdate(void) {
    SystemCoreClock = PLL1_SYSCLK_HZ;
}

void SystemInit(void) {
    volatile uint32_t *cpacr = (uint32_t *)0xE000ED88u;
    volatile uint32_t *vtor = (uint32_t *)0xE000ED08u;

    *cpacr |= (0xFu << 20);
    *vtor = 0x08000000u;

    system_clock_config();
}
