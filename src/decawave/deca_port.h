/*
 * deca_port.h
 *
 *  Created on: Sep 23, 2015
 *      Author: ericrudisill
 */

#ifndef SRC_DECAWAVE_DECA_PORT_H_
#define SRC_DECAWAVE_DECA_PORT_H_

#include <cph.h>
#include "deca_device_api.h"

#define Sleep(x)			cph_millis_delay(x)
#define deca_millis()		cph_get_millis()

#define deca_pin_high(x)		port_pin_set_output_level(x,true)
#define deca_pin_low(x)			port_pin_set_output_level(x,false)
#define deca_pin_toggle(x)		port_pin_toggle_output_level(x)

#define led_off(x)				port_pin_set_output_level(LED_PIN, true)
#define led_on(x)				port_pin_set_output_level(LED_PIN, false)

typedef enum
{
    LED_PC6,
    LED_PC7,
    LED_PC8,
    LED_PC9,
    LED_ALL,
    LEDn
} led_t;



#define SPI_BaudRatePrescaler_4		5000000UL
#define SPI_BaudRatePrescaler_16	1000000UL


void peripherals_init(void);
void spi_peripheral_init(void);
void SPI_ConfigFastRate(uint32_t baudrate);
void port_SPIx_clear_chip_select(void);
void port_SPIx_set_chip_select(void);
void port_DisableEXT_IRQ(void);
void port_EnableEXT_IRQ(void);
bool port_CheckEXT_IRQ(void);

void setup_DW1000RSTnIRQ(int enable);
void reset_DW1000(void);

extern void process_deca_irq(void);
extern void process_dwRSTn_irq(void);

decaIrqStatus_t decamutexon(void);
void decamutexoff(decaIrqStatus_t s);


int writetospi_serial(uint16_t headerLength, const uint8_t *headerBuffer, uint32_t bodylength, const uint8_t *bodyBuffer);
int readfromspi_serial(uint16_t headerLength, const uint8_t *headerBuffer, uint32_t readlength, uint8_t *readBuffer);
#define writetospi		writetospi_serial
#define readfromspi		readfromspi_serial


#endif /* SRC_DECAWAVE_DECA_PORT_H_ */
