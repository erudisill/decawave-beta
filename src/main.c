/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>

void configure_stdio(void);
void configure_spi(void);

struct usart_module usart_instance;

#define SPI_BUF_LENGTH 20
volatile uint8_t spi_buffer[SPI_BUF_LENGTH];
struct spi_module spi_master_instance;
struct spi_slave_inst spi_slave;

void configure_stdio(void) {

	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);

	config_usart.baudrate = STDIO_BAUD;
	config_usart.mux_setting = STDIO_MUX;
	config_usart.pinmux_pad0 = STDIO_PAD0;
	config_usart.pinmux_pad1 = STDIO_PAD1;
	config_usart.pinmux_pad2 = STDIO_PAD2;
	config_usart.pinmux_pad3 = STDIO_PAD3;

	stdio_serial_init(&usart_instance, STDIO_HW, &config_usart);
	usart_enable(&usart_instance);
}

void configure_spi(void) {
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

volatile uint32_t g_ul_ms_ticks = 0;
void SysTick_Handler(void) {
	g_ul_ms_ticks++;
}
static void mdelay(uint32_t ticks) {
	uint32_t current;
	current = g_ul_ms_ticks;
	while ((g_ul_ms_ticks - current) < ticks)
		;
}

int main(void) {
	system_init();

	configure_stdio();

	configure_spi();

	system_interrupt_enable_global();

	SysTick_Config(system_cpu_clock_get_hz() / 1000);

	while (1) {

		// Device ID
		spi_buffer[0] = 0x00;
		spi_buffer[1] = 0x00;
		spi_buffer[2] = 0x00;
		spi_buffer[3] = 0x00;
		spi_buffer[4] = 0x00;

		spi_select_slave(&spi_master_instance, &spi_slave, true);

		spi_write_buffer_wait(&spi_master_instance, (const uint8_t *) spi_buffer, 1);
		spi_read_buffer_wait(&spi_master_instance, (uint8_t *) &spi_buffer[1], 4, 0xff);

		spi_select_slave(&spi_master_instance, &spi_slave, false);

		printf(":%02X%02X%02X%02X%02X\r\n", spi_buffer[0], spi_buffer[1], spi_buffer[2], spi_buffer[3], spi_buffer[4]);

		mdelay(500);
		port_pin_toggle_output_level(LED_PIN);
	}

}
