/*
 * tests.c
 *
 *  Created on: Sep 23, 2015
 *      Author: ericrudisill
 */

#include <cph.h>
#include <decawave.h>
#include <deca_regs.h>

#ifdef MAIN_TEST

#define SPI_BUF_LENGTH 20

static uint8_t spi_buffer[SPI_BUF_LENGTH];

static bool _int_hit = false;

void extint_handle(void)
{
	bool status = port_pin_get_input_level(PIN_PA00);
	port_pin_set_output_level(LED_PIN, !status);
	_int_hit = true;
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


typedef struct {
	uint8 channel;
	uint8 prf;
	uint8 datarate;
	uint8 preambleCode;
	uint8 preambleLength;
	uint8 pacSize;
	uint8 nsSFD;
	uint16 sfdTO;
} chConfig_t;

//Configuration for DecaRanging Modes (8 default use cases selectable by the switch S1 on EVK)
static chConfig_t chConfig[8] = {
//mode 1 - S1: 7 off, 6 off, 5 off
		{ 2,              // channel
				DWT_PRF_16M,    // prf
				DWT_BR_110K,    // datarate
				3,             // preambleCode
				DWT_PLEN_1024,  // preambleLength
				DWT_PAC32,      // pacSize
				1,       // non-standard SFD
				(1025 + 64 - 32) //SFD timeout
		},
		//mode 2
		{ 2,              // channel
				DWT_PRF_16M,    // prf
				DWT_BR_6M8,    // datarate
				3,             // preambleCode
				DWT_PLEN_128,   // preambleLength
				DWT_PAC8,       // pacSize
				0,       // non-standard SFD
				(129 + 8 - 8) //SFD timeout
		},
		//mode 3
		{ 2,              // channel
				DWT_PRF_64M,    // prf
				DWT_BR_110K,    // datarate
				9,             // preambleCode
				DWT_PLEN_1024,  // preambleLength
				DWT_PAC32,      // pacSize
				1,       // non-standard SFD
				(1025 + 64 - 32) //SFD timeout
		},
		//mode 4
		{ 2,              // channel
				DWT_PRF_64M,    // prf
				DWT_BR_6M8,    // datarate
				9,             // preambleCode
				DWT_PLEN_128,   // preambleLength
				DWT_PAC8,       // pacSize
				0,       // non-standard SFD
				(129 + 8 - 8) //SFD timeout
		},
		//mode 5
		{ 5,              // channel
				DWT_PRF_16M,    // prf
				DWT_BR_110K,    // datarate
				3,             // preambleCode
				DWT_PLEN_1024,  // preambleLength
				DWT_PAC32,      // pacSize
				1,       // non-standard SFD
				(1025 + 64 - 32) //SFD timeout
		},
		//mode 6
		{ 5,              // channel
				DWT_PRF_16M,    // prf
				DWT_BR_6M8,    // datarate
				3,             // preambleCode
				DWT_PLEN_128,   // preambleLength
				DWT_PAC8,       // pacSize
				0,       // non-standard SFD
				(129 + 8 - 8) //SFD timeout
		},
		//mode 7
		{ 5,              // channel
				DWT_PRF_64M,    // prf
				DWT_BR_110K,    // datarate
				9,             // preambleCode
				DWT_PLEN_1024,  // preambleLength
				DWT_PAC32,      // pacSize
				1,       // non-standard SFD
				(1025 + 64 - 32) //SFD timeout
		},
		//mode 8
		{ 5,              // channel
				DWT_PRF_64M,    // prf
				DWT_BR_6M8,    // datarate
				9,             // preambleCode
				DWT_PLEN_128,   // preambleLength
				DWT_PAC8,       // pacSize
				0,       // non-standard SFD
				(129 + 8 - 8) //SFD timeout
		} };



static uint8 buffer[40] = { 0, 0 };

static void print_status(void) {
	uint32 status = 0;
	uint32 status1 = 0;

	status = dwt_read32bitreg(SYS_STATUS_ID);
	status1 = dwt_read32bitoffsetreg(SYS_STATUS_ID, 1);            // read status register
	dwt_readfromdevice(RX_BUFFER_ID, 0, 2, buffer);
	printf("SYS_STATUS status %02X %08X, byte 0 %02X%02X\r\n", status1 >> 24, status, buffer[0], buffer[1]);

	status = dwt_read32bitreg(SYS_MASK_ID);
	printf("SYS_MASK             %08X    ", status);

//	memset(buffer, 0xaa, FS_CTRL_LEN);
//	dwt_readfromdevice(FS_CTRL_ID, 0, FS_CTRL_LEN, buffer);
//	printf("FS_PLLCFG %08X  FS_PLLTUNE %02X  FS_XTALT %02X\r\n", *((uint32_t*) &buffer[0x07]), (uint8_t) (buffer[0x0b]),
//			(uint8_t) (buffer[0x0e]));

	status = dwt_read32bitoffsetreg(RF_CONF_ID, RF_STATUS_OFFSET);
	printf("RF_STATUS %08X\r\n", status);

	printf("_int_hit: %02X\r\n", _int_hit);

	printf("\r\n");
}

uint32 decawave_init(void) {
	uint32 devID;
	instanceConfig_t instConfig;
	int i, result;

	SPI_ConfigFastRate(SPI_BaudRatePrescaler_16); //max SPI before PLLs configured is ~4M

	i = 10;

	//this is called here to wake up the device (i.e. if it was in sleep mode before the restart)
	devID = instancereaddeviceid();
	printf("devID %08X\r\n", devID);
	//if the read of devide ID fails, the DW1000 could be asleep
	if (DWT_DEVICE_ID != devID) {
		port_SPIx_clear_chip_select();  //CS low
		Sleep(1); //200 us to wake up then waits 5ms for DW1000 XTAL to stabilise
		port_SPIx_set_chip_select();  //CS high
		Sleep(7);

		devID = instancereaddeviceid();
		printf("devID %08X\r\n", devID);
		// SPI not working or Unsupported Device ID
		if (DWT_DEVICE_ID != devID) {
			return (-1);
		}
		//clear the sleep bit - so that after the hard reset below the DW does not go into sleep
		dwt_softreset();
	}

	//reset the DW1000 by driving the RSTn line low
	reset_DW1000();

	result = instance_init();
	if (0 > result)
		return (-2); // Some failure has occurred

	SPI_ConfigFastRate(SPI_BaudRatePrescaler_4); //increase SPI to max
	devID = instancereaddeviceid();
	printf("devID %08X\r\n", devID);

	if (DWT_DEVICE_ID != devID)   // Means it is NOT MP device
			{
		// SPI not working or Unsupported Device ID
		return (-3);
	}

	instancesetrole(ANCHOR);     // Set this instance role

	instance_init_s(ANCHOR);

	int _xx_dr_mode = 0x02;		// Mode 3

	instConfig.channelNumber = chConfig[_xx_dr_mode].channel;
	instConfig.preambleCode = chConfig[_xx_dr_mode].preambleCode;
	instConfig.pulseRepFreq = chConfig[_xx_dr_mode].prf;
	instConfig.pacSize = chConfig[_xx_dr_mode].pacSize;
	instConfig.nsSFD = chConfig[_xx_dr_mode].nsSFD;
	instConfig.sfdTO = chConfig[_xx_dr_mode].sfdTO;
	instConfig.dataRate = chConfig[_xx_dr_mode].datarate;
	instConfig.preambleLen = chConfig[_xx_dr_mode].preambleLength;

	instance_config(&instConfig);                  // Set operating channel etc

	instancesettagsleepdelay(POLL_SLEEP_DELAY, BLINK_SLEEP_DELAY); //set the Tag sleep time

	// NOTE: this is the delay between receiving the blink and sending the ranging init message
	// The anchor ranging init response delay has to match the delay the tag expects
	// the tag will then use the ranging response delay as specified in the ranging init message
	// use this to set the long blink response delay (e.g. when ranging with a PC anchor that wants to use the long response times != 150ms)
	instancesetblinkreplydelay(FIXED_REPLY_DELAY);

	//set the default response delays
	instancesetreplydelay(FIXED_REPLY_DELAY, 0);

	return 0;
}

void decawave_test(void) {
	spi_peripheral_init();
	decawave_init();
}

void blinky_test(void) {
	uint8_t counter = 0;

	system_init();

	cph_millis_init();

	cph_stdio_init();

	printf("\r\n\r\nBLINKY TEST\r\n");

	spi_peripheral_init();

	while (1) {

		// Device ID
		spi_buffer[0] = 0x00;
		spi_buffer[1] = 0x00;
		spi_buffer[2] = 0x00;
		spi_buffer[3] = 0x00;
		spi_buffer[4] = 0x00;

		port_SPIx_set_chip_select();

		readfromspi_serial(1,spi_buffer, 4, &spi_buffer[1]);

		port_SPIx_clear_chip_select();

		printf("+%02X%02X%02X%02X%02X\r\n", spi_buffer[0],spi_buffer[1],spi_buffer[2],spi_buffer[3],spi_buffer[4]);


		cph_millis_delay(500);
		counter++;
		port_pin_toggle_output_level(LED_PIN);
	}
}

void main_test(void) {
	system_init();

	cph_millis_init();

	cph_stdio_init();

//	extint_init();

	system_interrupt_enable_global();

	decawave_test();

//	dwt_rxenable(0);

	printf("\r\nTEST TEST TEST\r\n");

	while (1) {
 		print_status();

		cph_millis_delay(500);
		port_pin_toggle_output_level(LED_PIN);
		cph_millis_delay(500);
		port_pin_toggle_output_level(LED_PIN);
	}
}

#endif
