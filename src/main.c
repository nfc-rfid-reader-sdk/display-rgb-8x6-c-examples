/*
 ============================================================================
 Name        : display-rgb-8x6-examples-c.c
 Author      : srkos
 Version     :
 Copyright   : Digital Logic, www.d-logic.net
 ============================================================================
 */

#include <led_display.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#	include <windows.h>
#else
#	include <unistd.h>
#	define Sleep(ms)		usleep((ms)*1000)
#endif

#define MIN_LIB_VER		"2.1.0"

DL_STATUS status;

/* zero is broadcast - only one LED Display can be connected to the bus */
int display_id = 0;

void prn_status(char * chPreText)
{
	printf("  (exec) %s: %s\n", chPreText, DL_STATUS2Str(status));
	fflush(stdout);
}

int main(void)
{
	int i;

	puts("Test LED display library API functions.");

	printf("Minimal library version is: ");
	puts(MIN_LIB_VER);

	/*--- Get library version information : string method ---*/

	printf("Library version (string variant): %s\n", DL_GetLibraryVersionStr());

	/*--- On the first, open communication port ---*/

	status = DL_OpenPort();

	prn_status("Open LED Display communication port");

	if (DL_OK != status)
	{
		puts("Exit from test on error.");

		return 0;
	}

	/* ---------------------------------------------------------- */

	printf("The (hard-coded) LED Display ID used for communication is [%d].\n",
			display_id);

	/* ---------------------------------------------------------- */

	/*--- Get basic information about LED Display ---*/
	int fw_version;
	int hw_version;

	status = DL_GetDisplayFirmwareVersion(display_id, &fw_version, &hw_version);

	prn_status("DL_GetDisplayVersion");

	if (DL_OK == status)
	{
		printf("LED Display firmware version= %i | "
				"hardware type/version= 0x%04X.\n", fw_version, hw_version);
	}

	char strSN[9] = "";
	int intID;

	status = DL_GetDisplaySerial(display_id, strSN, &intID);

	prn_status("DL_GetDisplaySerial");

	if (DL_OK == status)
	{
		printf("LED Display serial number is [%s].\n"
				"LED Display ID is [%d] (display setup)\n", strSN, intID);
	}

	/*--- Get the display time ---*/

	uint32_t timestamp;

	status = DL_DisplayTimeGet(display_id, &timestamp);

	prn_status("DL_DisplayTimeGet");

	if (DL_OK == status)
	{
		printf("Current LED Display time is= (timestamp= %u) %s\n", timestamp,
				ctime((const time_t *) &timestamp));

	// ctime() convert timestamp to (local time) string
	}

	/*--- Set the display time and check ---*/

	timestamp = time(0); // now

	status = DL_DisplayTimeSet(display_id, timestamp);

	prn_status("DL_DisplayTimeSet");

	if (DL_OK == status)
	{
		printf("LED Display time is set to: (timestamp= %u) %s\n", timestamp,
				ctime((const time_t *) &timestamp));
	}

	/*--- check time again ---*/

	status = DL_DisplayTimeGet(display_id, &timestamp);

	prn_status("DL_DisplayTimeGet");

	if (DL_OK == status)
	{
		printf("Current LED Display time is= (timestamp= %u) %s\n", timestamp,
				ctime((const time_t *) &timestamp));
	}

	/* ---------------------------------------------------------- */

	/*
	 * Both configuration and sending text is in one function:
	 * DL_DisplaySendConfigRGB() + DL_DisplaySendText() = DL_DisplayCommonRgb()
	 *
	 * example use separated functions.
	 */

	/*--- configuration ---*/

	int number_of_panels = 12;
	int number_of_rows = 4;
	int font = 2;
	int brightness = 10;
	int scroll_start_ms = 1000;
	int scroll_speed_ms = 300;
	int screen_display_time = 5000; //
	int red = 50;
	int green = 100;
	int blue = 150;

	status = DL_DisplaySendConfigRGB(display_id, number_of_panels,
			number_of_rows, font, brightness, scroll_start_ms, scroll_speed_ms,
			screen_display_time, red, green, blue);

	prn_status("DL_DisplaySendConfigRGB");

	/*--- send text, multiple times... ---*/

	char *text_content = "SAMPLE TEXT LINE";
	int text_len = strlen(text_content);

	status = DL_DisplaySendText(display_id, text_content, text_len);

	prn_status("DL_DisplaySendText");

	/* ---------------------------------------------------------- */

	/*--- Set default message, store in display, available after restart ---*/

	char *default_msg = "DEFAULT MESSAGE";
	int default_msg_len = strlen(default_msg);

	printf("Test write default message in the %d\n", display_id); /* prints Test UART on ARM */

	status = DL_DisplaySetDefaultRgb(display_id, number_of_panels,
			number_of_rows, font, brightness, scroll_start_ms, scroll_speed_ms,
			screen_display_time, default_msg, default_msg_len, red, green,
			blue);

	/* ---------------------------------------------------------- */

	puts("Wait for 10 seconds, then the display will reset !");
	fflush(stdout);

	for (i = 0; i < 10; ++i)
	{
		printf(".");
		fflush(stdout);

		Sleep(1000);
	}
	printf("\n");

	status = DL_DisplayReset(display_id);

	prn_status("DL_DisplayReset");

	puts("The default message shows on the display ?!");

	/*--- Close communication port ---*/

	status = DL_ClosePort();

	prn_status("Close LED Display communication port");

	/* ---------------------------------------------------------- */

	puts("Test done.");

	return EXIT_SUCCESS;
}
