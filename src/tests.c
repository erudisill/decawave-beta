/*
 * tests.c
 *
 *  Created on: Sep 23, 2015
 *      Author: ericrudisill
 */

#include <cph.h>


#ifdef MAIN_TEST

static void configure_spi(void);


#define SPI_BUF_LENGTH 20

static volatile uint8_t spi_buffer[SPI_BUF_LENGTH];
static struct spi_module spi_master_instance;
static struct spi_slave_inst spi_slave;


static void configure_spi(void) {
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

void extint_handle(void)
{
	bool status = port_pin_get_input_level(PIN_PA00);
	port_pin_set_output_level(LED_PIN, !status);
}

void extint_init(void) {

	struct extint_chan_conf config_chan;
	extint_chan_get_config_defaults(&config_chan);

	config_chan.gpio_pin = PIN_PA00;
	config_chan.gpio_pin_mux = PIN_PA00A_EIC_EXTINT0;
	config_chan.gpio_pin_pull = EXTINT_PULL_NONE;
	config_chan.detection_criteria = EXTINT_DETECT_BOTH;

	extint_chan_set_config(0, &config_chan);

	extint_register_callback(extint_handle, 0, EXTINT_CALLBACK_TYPE_DETECT);

	extint_chan_enable_callback(0, EXTINT_CALLBACK_TYPE_DETECT);
}

void main_test(void) {
	system_init();

	cph_millis_init();

	cph_stdio_init();

//	configure_spi();

	extint_init();

	system_interrupt_enable_global();

	port_pin_set_output_level(LED_PIN, true);

	uint8_t counter = 0;

	printf("\r\n\r\nSTART\r\n");

	while (1) {

//		// Device ID
//		spi_buffer[0] = 0x00;
//		spi_buffer[1] = 0x00;
//		spi_buffer[2] = 0x00;
//		spi_buffer[3] = 0x00;
//		spi_buffer[4] = 0x00;
//
//		spi_select_slave(&spi_master_instance, &spi_slave, true);
//
//		spi_write_buffer_wait(&spi_master_instance, (const uint8_t *) spi_buffer, 1);
//		spi_read_buffer_wait(&spi_master_instance, (uint8_t *) &spi_buffer[1], 4, 0xff);
//
//		spi_select_slave(&spi_master_instance, &spi_slave, false);

		printf(":%02X%02X%02X%02X%02X %02X\r\n", spi_buffer[0], spi_buffer[1], spi_buffer[2], spi_buffer[3], spi_buffer[4], counter);

		cph_millis_delay(500);
		counter++;
//		port_pin_toggle_output_level(LED_PIN);
	}

}

#endif
