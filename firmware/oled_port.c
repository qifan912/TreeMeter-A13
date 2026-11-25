#include "stm32f10x.h"
#include "oled_port.h"

#define SCL (1U << 6)
#define SDA (1U << 7)

/* Software I2C, PB6/PB7 open-drain. The OLED module must have pull-ups.
   DWT timing follows SystemCoreClock; SysTick remains owned by mainD.c. */
static void delay_us(unsigned int us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = (SystemCoreClock / 1000000U) * us;
    while ((uint32_t)(DWT->CYCCNT - start) < ticks) { }
}

void oled_port_delay_ms(unsigned int ms)
{
    while (ms-- != 0U) delay_us(1000U);
}

static unsigned int scl_high(void)
{
    uint32_t start;
    GPIOB->BSRR = SCL;
    start = DWT->CYCCNT;
    while ((GPIOB->IDR & SCL) == 0U)
    {
        if ((uint32_t)(DWT->CYCCNT - start) >=
            (SystemCoreClock / 1000000U) * 100U) return 2U;
    }
    delay_us(5U);
    return 0U;
}

static unsigned int stop(void)
{
    unsigned int status;
    GPIOB->BRR = SCL;
    GPIOB->BRR = SDA;
    delay_us(5U);
    status = scl_high();
    GPIOB->BSRR = SDA;
    delay_us(5U);
    if ((GPIOB->IDR & (SCL | SDA)) != (SCL | SDA)) return 2U;
    return status;
}

unsigned int oled_port_init(void)
{
    unsigned int i;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    RCC->APB2ENR |= (1U << 3); /* GPIOB */
    GPIOB->BSRR = SCL | SDA;
    GPIOB->CRL = (GPIOB->CRL & ~((0xFU << 24) | (0xFU << 28))) |
                  (0x6U << 24) | (0x6U << 28); /* OD, 2 MHz */
    oled_port_delay_ms(100U); /* module power/reset settling */
    if (scl_high() != 0U) return 2U;
    /* Release a slave left mid-byte after an MCU-only reset. */
    for (i = 0U; i < 9U && (GPIOB->IDR & SDA) == 0U; i++)
    {
        GPIOB->BRR = SCL;
        delay_us(5U);
        if (scl_high() != 0U) return 2U;
    }
    return stop();
}

static unsigned int start(void)
{
    GPIOB->BSRR = SDA;
    delay_us(5U);
    if (scl_high() != 0U || (GPIOB->IDR & SDA) == 0U) return 2U;
    GPIOB->BRR = SDA;
    delay_us(5U);
    GPIOB->BRR = SCL;
    return 0U;
}

static unsigned int write_byte(uint8_t byte)
{
    unsigned int i, status;
    for (i = 0U; i < 8U; i++)
    {
        if ((byte & 0x80U) != 0U) GPIOB->BSRR = SDA;
        else GPIOB->BRR = SDA;
        delay_us(5U);
        if (scl_high() != 0U) return 2U;
        GPIOB->BRR = SCL;
        byte = (uint8_t)(byte << 1);
    }
    GPIOB->BSRR = SDA; /* release SDA for slave ACK */
    delay_us(5U);
    if (scl_high() != 0U) return 2U;
    status = ((GPIOB->IDR & SDA) == 0U) ? 0U : 1U;
    GPIOB->BRR = SCL;
    delay_us(5U);
    return status;
}

unsigned int oled_port_tx(uint8_t address, const uint8_t *bytes, unsigned int count)
{
    unsigned int i, status, stop_status;
    status = start();
    if (status == 0U) status = write_byte((uint8_t)(address << 1));
    for (i = 0U; status == 0U && i < count; i++) status = write_byte(bytes[i]);
    stop_status = stop();
    return (status != 0U) ? status : stop_status;
}
