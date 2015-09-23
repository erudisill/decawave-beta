/*
 * deca_port.c
 *
 *  Created on: Sep 23, 2015
 *      Author: ericrudisill
 */

#include <cph.h>
#include "deca_port.h"

static struct spi_module spi_master_instance;
static struct spi_slave_inst spi_slave;


void spi_peripheral_init(void) {
	struct spi_config config_spi_master;
	struct spi_slave_inst_config slave_dev_config;

	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = DW_SPI_SS_PIN;
	spi_attach_slave(&spi_slave, &slave_dev_config);

	spi_get_config_defaults(&config_spi_master);
	// DOPO: 0x00						DIPO: 0x03
	// PA16/SERCOM1[0]: MOSI			PA19/SERCOM1[]3: MISO
	config_spi_master.mux_setting = DW_SPI_MUX;
	config_spi_master.pinmux_pad0 = DW_SPI_PAD0;
	config_spi_master.pinmux_pad1 = DW_SPI_PAD1;
	config_spi_master.pinmux_pad2 = DW_SPI_PAD2;
	config_spi_master.pinmux_pad3 = DW_SPI_PAD3;
	config_spi_master.transfer_mode = DW_SPI_TRANSFER_MODE;

	spi_init(&spi_master_instance, DW_SPI_HW, &config_spi_master);

	spi_enable(&spi_master_instance);

}

void SPI_ConfigFastRate(uint32_t baudrate) {
	spi_disable(&spi_master_instance);
	spi_set_baudrate(&spi_master_instance, baudrate);
	spi_enable(&spi_master_instance);
}

void port_SPIx_clear_chip_select(void) {
	spi_select_slave(&spi_master_instance, &spi_slave, false);
}

void port_SPIx_set_chip_select(void) {
	spi_select_slave(&spi_master_instance, &spi_slave, true);
}

void port_DisableEXT_IRQ(void) {
	extint_chan_disable_callback(DW_IRQ_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

void port_EnableEXT_IRQ(void) {
	extint_chan_enable_callback(DW_IRQ_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

bool port_CheckEXT_IRQ(void) {
	return extint_chan_is_detected(DW_IRQ_LINE);
}

void peripherals_init(void) {

	struct extint_chan_conf config_chan;
	extint_chan_get_config_defaults(&config_chan);

	config_chan.gpio_pin = DW_IRQ_PIN;
	config_chan.gpio_pin_mux = DW_IRQ_PIN_MUX;
	config_chan.gpio_pin_pull = DW_IRQ_PIN_PULL;
	config_chan.detection_criteria = DW_IRQ_PIN_DETECT;

	extint_chan_set_config(DW_IRQ_LINE, &config_chan);

	extint_register_callback(process_deca_irq, DW_IRQ_LINE, EXTINT_CALLBACK_TYPE_DETECT);

	extint_chan_enable_callback(DW_IRQ_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

void setup_DW1000RSTnIRQ(int enable) {
	if (enable) {
		struct extint_chan_conf config_chan;
		extint_chan_get_config_defaults(&config_chan);

		config_chan.gpio_pin = DW_RST_PIN;
		config_chan.gpio_pin_mux = DW_RST_PIN_MUX;
		config_chan.gpio_pin_pull = DW_RST_PIN_PULL;
		config_chan.detection_criteria = DW_RST_PIN_DETECT;

		extint_chan_set_config(DW_RST_LINE, &config_chan);

		// No need to unregister since we're always using the same callback. ASF is okay with that.
		extint_register_callback(process_dwRSTn_irq, DW_RST_LINE, EXTINT_CALLBACK_TYPE_DETECT);

		extint_chan_enable_callback(DW_RST_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	}
	else {
		extint_chan_disable_callback(DW_RST_LINE, EXTINT_CALLBACK_TYPE_DETECT);
		struct port_config config_port = DW_RST_PIN_CONFIG;
		port_pin_set_config(DW_RST_PIN, &config_port);
	}
}

void reset_DW1000(void) {
	struct port_config config_port = DW_RST_PIN_CONFIG;

	// Safety
	extint_chan_disable_callback(DW_RST_LINE, EXTINT_CALLBACK_TYPE_DETECT);

	// Pull the pin low
	config_port.input_pull = PORT_PIN_PULL_DOWN;
	port_pin_set_config(DW_RST_PIN, &config_port);

	// Now release it
	config_port.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(DW_RST_PIN, &config_port);


	// Why sleep here?  Left over from original code.
	Sleep(1);
}

