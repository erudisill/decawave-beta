/*
 * decawave.c
 *
 *  Created on: Jun 30, 2015
 *      Author: ericrudisill
 */

#include <decawave.h>

#define SOFTWARE_VER_STRING  "Version 0.01    "

int is_tag = 0x00;
int use_fast2wr = 0x00;
int use_long_blink_delay = 0x00;
int use_dr_mode = 0x02;  // Mode 3
//uint8_t eui64[] = { 0xDE, 0xCA, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };
uint8_t eui64[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0xCA, 0xDE };

int instance_anchaddr = 0; //0 = 0xDECA020000000001; 1 = 0xDECA020000000002; 2 = 0xDECA020000000003
int instance_mode = ANCHOR;
//int instance_mode = TAG;
//int instance_mode = TAG_TDOA;
//int instance_mode = LISTENER;
int dr_mode = 0;

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
chConfig_t chConfig[8] = {
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

#if (DR_DISCOVERY == 0)
//Tag address list
uint64 tagAddressList[3] =
{
	0xDECA010000001001,         // First tag
	0xDECA010000000002,// Second tag
	0xDECA010000000003// Third tag
};

//Anchor address list
uint64 anchorAddressList[ANCHOR_LIST_SIZE] =
{
	0xDECA020000000001 ,       // First anchor
	0xDECA020000000002 ,// Second anchor
	0xDECA020000000003 ,// Third anchor
	0xDECA020000000004// Fourth anchor
};

//ToF Report Forwarding Address
uint64 forwardingAddress[1] =
{
	0xDECA030000000001
};

// ======================================================
//
//  Configure instance tag/anchor/etc... addresses
//
void addressconfigure(void)
{
	instanceAddressConfig_t ipc;

	ipc.forwardToFRAddress = forwardingAddress[0];
	ipc.anchorAddress = anchorAddressList[instance_anchaddr];
	ipc.anchorAddressList = anchorAddressList;
	ipc.anchorListSize = ANCHOR_LIST_SIZE;
	ipc.anchorPollMask = 0x1; //0x7;              // anchor poll mask

	ipc.sendReport = 1;//1 => anchor sends TOF report to tag
	//ipc.sendReport = 2 ;  //2 => anchor sends TOF report to listener

	instancesetaddresses(&ipc);
}
#endif

#include <string.h>
#include <deca_regs.h>
static uint8 buffer[40] = { 0, 0 };

static void print_status(void) {
	uint32 status = 0;
	uint32 status1 = 0;

	status = dwt_read32bitreg(SYS_STATUS_ID);
	status1 = dwt_read32bitoffsetreg(SYS_STATUS_ID, 1);            // read status register
	dwt_readfromdevice(RX_BUFFER_ID, 0, 2, buffer);
	printf("MAIN status %02X %08X, byte 0 %02X%02X\r\n", status1 >> 24, status, buffer[0], buffer[1]);

	memset(buffer, 0xaa, FS_CTRL_LEN);
	dwt_readfromdevice(FS_CTRL_ID, 0, FS_CTRL_LEN, buffer);
	printf("FS_PLLCFG %08X  FS_PLLTUNE %02X  FS_XTALT %02X\r\n", *((uint32_t*) &buffer[0x07]), (uint8_t) (buffer[0x0b]),
			(uint8_t) (buffer[0x0e]));

	status = dwt_read32bitoffsetreg(RF_CONF_ID, RF_STATUS_OFFSET);
	printf("RF_STATUS %08X\r\n\r\n", status);
}

int decarangingmode(void) {
	return use_dr_mode;
}

uint32 inittestapplication(void) {
	uint32 devID;
	instanceConfig_t instConfig;
	int i, result;

	SPI_ConfigFastRate(SPI_BaudRatePrescaler_16);  //max SPI before PLLs configured is ~4M

	i = 10;

	//this is called here to wake up the device (i.e. if it was in sleep mode before the restart)
	devID = instancereaddeviceid();
	printf("devID %08X\r\n", devID);
	//if the read of devide ID fails, the DW1000 could be asleep
	if (DWT_DEVICE_ID != devID) {
		port_SPIx_clear_chip_select();  //CS low
		Sleep(1);   //200 us to wake up then waits 5ms for DW1000 XTAL to stabilise
		port_SPIx_set_chip_select();  //CS high
		Sleep(7);

//		printf("asleep...wakeup!\r\n");
//		deca_pin_high(DW_WAKEUP_PIO_IDX);
//		Sleep(1);
//		deca_pin_low(DW_WAKEUP_PIO_IDX);
//		Sleep(7);

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

	if (is_tag) {
		instance_mode = TAG;
		led_on(LED_PC7);
	} else {
		instance_mode = ANCHOR;
#if (DR_DISCOVERY == 1)
		led_on(LED_PC6);
#else
		if(instance_anchaddr & 0x1)
		led_on(LED_PC6);

		if(instance_anchaddr & 0x2)
		led_on(LED_PC7);
#endif
	}

	instancesetrole(instance_mode);     // Set this instance role

	if (use_fast2wr) //if fast ranging then initialise instance for fast ranging application
	{
		instance_init_f(instance_mode); //initialise Fast 2WR specific data
		//when using fast ranging the channel config is either mode 2 or mode 6
		//default is mode 2
		dr_mode = decarangingmode();

		if ((dr_mode & 0x1) == 0)
			dr_mode = 1;
	} else {
		instance_init_s(instance_mode);
		dr_mode = decarangingmode();
	}

	instConfig.channelNumber = chConfig[dr_mode].channel;
	instConfig.preambleCode = chConfig[dr_mode].preambleCode;
	instConfig.pulseRepFreq = chConfig[dr_mode].prf;
	instConfig.pacSize = chConfig[dr_mode].pacSize;
	instConfig.nsSFD = chConfig[dr_mode].nsSFD;
	instConfig.sfdTO = chConfig[dr_mode].sfdTO;
	instConfig.dataRate = chConfig[dr_mode].datarate;
	instConfig.preambleLen = chConfig[dr_mode].preambleLength;

	instance_config(&instConfig);                  // Set operating channel etc

#if (DR_DISCOVERY == 0)
	addressconfigure();                            // set up initial payload configuration
#endif
	instancesettagsleepdelay(POLL_SLEEP_DELAY, BLINK_SLEEP_DELAY); //set the Tag sleep time

	//if TA_SW1_2 is on use fast ranging (fast 2wr)
	if (use_fast2wr) {
		//Fast 2WR specific config
		//configure the delays/timeouts
		instance_config_f();
	} else //use default ranging modes
	{
		// NOTE: this is the delay between receiving the blink and sending the ranging init message
		// The anchor ranging init response delay has to match the delay the tag expects
		// the tag will then use the ranging response delay as specified in the ranging init message
		// use this to set the long blink response delay (e.g. when ranging with a PC anchor that wants to use the long response times != 150ms)
		if (use_long_blink_delay) {
			instancesetblinkreplydelay(FIXED_LONG_BLINK_RESPONSE_DELAY);
		} else //this is for ARM to ARM tag/anchor (using normal response times 150ms)
		{
			instancesetblinkreplydelay(FIXED_REPLY_DELAY);
		}

		//set the default response delays
		instancesetreplydelay(FIXED_REPLY_DELAY, 0);
	}

//    return devID;
	return 0;
}

//void process_deca_irq(uint32_t id, uint32_t mask) {
//	printf("deca_irq\r\n");
//	do {
//		instance_process_irq(0);
//	} while (port_CheckEXT_IRQ() == 1);
//}

static volatile uint8_t irq_set = 0;

void process_deca_irq(void) {
	deca_pin_toggle(LED_PIN);
//	instance_process_irq(0);
	irq_set = 0x01;
}

void process_dwRSTn_irq(void) {
	instance_notify_DW1000_inIDLE(1);
}

void decawave_run(void) {
	int i = 0;
	int toggle = 1;
	int ranging = 0;
	uint8 dataseq[40];
	double range_result = 0;
	double avg_result = 0;
	uint8 dataseq1[40];
	uint8 command = 0x0;

	led_off(LED_ALL); //turn off all the LEDs

	peripherals_init();

	spi_peripheral_init();

	Sleep(1000); //wait for LCD to power on

	printf("DECAWAVE        \r\n");
	printf(SOFTWARE_VER_STRING);
	printf("\r\n");

	Sleep(1000);

	port_DisableEXT_IRQ(); //disable ScenSor IRQ until we configure the device

	printf("DECAWAVE  RANGE\r\n");

	led_off(LED_ALL);

	int testresult = inittestapplication();
	if (testresult < 0) {
		led_on(LED_ALL); //to display error....
		printf("ERROR\r\n");
		printf("INIT FAIL %d\r\n", testresult);
		for (;;) {
		}
	}

	//sleep for 5 seconds displaying "Decawave"
	i = 30;
	while (i--) {
		if (i & 1)
			led_off(LED_ALL);
		else
			led_on(LED_ALL);

		Sleep(200);
	}
	i = 0;
	led_off(LED_ALL);

	if (is_tag) {
		instance_mode = TAG;
		printf("TAG\r\n");
	} else {
		instance_mode = ANCHOR;
		printf("ANCHOR\r\n");
#if (DR_DISCOVERY == 1)
		printf("DR_DISCOVER == 1\r\n");
#else
		printf("DR_DISCOVER == 0\r\n");
#endif
	}

	if (instance_mode == TAG) {
		//if TA_SW1_2 is on use fast ranging (fast 2wr)
		if (use_fast2wr) {
			printf("Fast Tag Ranging\r\n");
		} else {
			printf("TAG BLINK %llX\r\n", instance_get_addr());
		}
	} else {
		printf("AWAITING POLL\r\n");
	}

	port_EnableEXT_IRQ(); //enable ScenSor IRQ before starting

	// main loop
	while (1) {

		//ERIC: Delay irq handling...will this work?
		uint32_t bail = 0;
		if (irq_set) {
			do {
				if (bail++ > 2000) {
					printf("BAIL!\r\n");
					Sleep(10);
					for (;;) {
					}
				}
				instance_process_irq(0);
			} while (port_CheckEXT_IRQ() == 1);
			irq_set = 0x00;
		}

		instance_run();

		if (instancenewrange()) {
			ranging = 1;
			//send the new range information to LCD and/or USB
			range_result = instance_get_idist();
#if (DR_DISCOVERY == 0)
			if(instance_mode == ANCHOR)
#endif
			avg_result = instance_get_adist();
			//set_rangeresult(range_result);

			printf("LAST: %4.2f m   ", range_result);
#if (DR_DISCOVERY == 0)
			if(instance_mode == ANCHOR)
			printf("AVG8: %4.2f m", avg_result);
			else
			printf("%llx", instance_get_anchaddr());
#else
			printf("AVG8: %4.2f m\r\n", avg_result);
#endif
		}

		if (ranging == 0) {
			if (instance_mode != ANCHOR) {
				if (instancesleeping()) {
					if (toggle) {
						printf("AWAITING RESPONSE\r\n");
					} else {
						toggle = 1;
						printf("TAG BLINK %llX\r\n", instance_get_addr());
					}
				}

				if (instanceanchorwaiting() == 2) {
					ranging = 1;
					printf("RANGING WITH %016llX\r\n", instance_get_anchaddr());
				}
			} else {
				if (instanceanchorwaiting()) {
					toggle += 2;

					if (toggle > 300000) {
						if (toggle & 0x1) {
							toggle = 0;
							printf("AWAITING POLL\r\n");
						} else {
							toggle = 1;
#if (DR_DISCOVERY == 1)
							printf("DISCOVERY MODE ");
#else
							printf("NON DISCOVERY ");
#endif
							printf("%llX\r\n", instance_get_addr());
						}
//						print_status();
					}

				} else if (instanceanchorwaiting() == 2) {
					printf("RANGING WITH %llX", instance_get_tagaddr());
				}
			}
		}
	}
}


