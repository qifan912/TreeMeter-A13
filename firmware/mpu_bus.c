#include "stm32f10x.h"
#include "mpu_bus.h"

#define SCL (1U << 8)
#define SDA (1U << 9)

/* Software I2C, PB8/PB9 open-drain. The OLED module must have pull-ups.
   DWT timing follows SystemCoreClock; SysTick remains owned by mainD.c. */
static void delay_us(unsigned int us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = (SystemCoreClock / 1000000U) * us;
    while ((uint32_t)(DWT->CYCCNT - start) < ticks) { }
}

void mpu_delay_ms(unsigned int ms)
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

unsigned int mpu_bus_init(void)
{
    unsigned int i;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    RCC->APB2ENR |= (1U << 3); /* GPIOB */
    GPIOB->BSRR = SCL | SDA;
    GPIOB->CRH = (GPIOB->CRH & ~((0xFU << 0) | (0xFU << 4))) |
                  (0x6U << 0) | (0x6U << 4); /* OD, 2 MHz */
    mpu_delay_ms(100U); /* module power/reset settling */
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

static unsigned int read_byte(uint8_t *value, unsigned int last)
{
    unsigned int i;
    uint8_t v = 0U;
    GPIOB->BSRR = SDA;
    for (i=0U; i<8U; i++) {
        delay_us(5U);
        if (scl_high()!=0U) return 2U;
        v=(uint8_t)((v<<1) | ((GPIOB->IDR & SDA)!=0U ? 1U:0U));
        GPIOB->BRR=SCL;
    }
    if (last) GPIOB->BSRR=SDA; else GPIOB->BRR=SDA;
    delay_us(5U);
    if (scl_high()!=0U) return 2U;
    GPIOB->BRR=SCL;
    delay_us(5U);
    GPIOB->BSRR=SDA;
    *value=v;
    return 0U;
}
unsigned int mpu_bus_write(uint8_t addr,uint8_t reg,uint8_t value)
{
    unsigned int s=start(), end;
    if (!s) s=write_byte((uint8_t)(addr<<1));
    if (!s) s=write_byte(reg);
    if (!s) s=write_byte(value);
    end=stop();
    return s ? s:end;
}
unsigned int mpu_bus_read(uint8_t addr,uint8_t reg,uint8_t *data,unsigned int n)
{
    unsigned int i,s=start(),end;
    if (!s) s=write_byte((uint8_t)(addr<<1));
    if (!s) s=write_byte(reg);
    if (!s) s=start(); /* repeated START */
    if (!s) s=write_byte((uint8_t)((addr<<1)|1U));
    for (i=0U; !s && i<n; i++) s=read_byte(&data[i],i+1U==n);
    end=stop();
    return s ? s:end;
}
