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
#include <cph.h>

#include <decawave.h>

int main(void) {

	system_init();

	cph_millis_init();
	cph_stdio_init();

	system_interrupt_enable_global();

#ifdef MAIN_TEST
	main_test();
#else

	decawave_run();

//	while (1) {
//		printf("HELLO\r\n");
//		port_pin_toggle_output_level(LED_PIN);
//		cph_millis_delay(500);
//	}

#endif

}
