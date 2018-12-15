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

/*** ---------------------------------------------------------- ***/

#define MIN_LIB_VER		"2.1.0"

#define NUMBER_OF_PANELS			12
#define NUMBER_OF_ROWS				2

/* zero is broadcast - only one LED Display can be connected to the bus */
int display_id = 0;

/*** ---------------------------------------------------------- ***/

DL_STATUS status;

void prn_status(char * chPreText)
{
	printf("  (exec) %s: %s\n", chPreText, DL_STATUS2Str(status));
	fflush(stdout);
}

/*** ---------------------------------------------------------- ***/

/*** ========================================================== ***/
/*** === Global text buffer and helper macros ================= ***/
/*** ---------------------------------------------------------- ***/

#define TEXT_LENGTH_MAX		1024

char text_buffer[TEXT_LENGTH_MAX + 1];
int text_buffer_size;

char *p2t; // pointer to the first empty position for adding text
int free_bytes;
int ts; // text size, temporary

/*=== Useful macros ===*/

/** Initialization */
#define INIT() \
	memset(text_buffer, 0, sizeof(text_buffer)); \
	free_bytes = TEXT_LENGTH_MAX; \
	text_buffer_size = 0; \
	p2t = text_buffer;

/** Insert provided C-string at the current position in the text buffer */
#define INS_STR(cstr) \
	strcpy(p2t, cstr); \
	ts = strlen(p2t); \
	p2t += ts; \
	free_bytes -= ts; \
	text_buffer_size = TEXT_LENGTH_MAX - free_bytes;

/** Insert EndOfLine at the current position in the text buffer */
#define INS_EOL() \
	ts = free_bytes; \
	status = DL_Insert_NewLine(p2t, &ts); \
	p2t += ts; \
	free_bytes -= ts; \
	text_buffer_size = TEXT_LENGTH_MAX - free_bytes;

/** Insert active text/time at the current position in the text buffer */
#define INS_TIME(tstamp, tfmt)\
	ts = free_bytes; \
	status = DL_Insert_DateTimeFormated(p2t, &ts, (tstamp), (tfmt)); \
	p2t += ts; \
	free_bytes -= ts; \
	text_buffer_size = TEXT_LENGTH_MAX - free_bytes;

/*** ---------------------------------------------------------- ***/
/*** ========================================================== ***/
/*** ========================================================== ***/

void set_display_configuration(void)
{
	int number_of_panels = NUMBER_OF_PANELS;
	int number_of_rows = NUMBER_OF_ROWS;
	int font = 2;
	int brightness = 5;
	int scroll_start_ms = 1000;
	int scroll_speed_ms = 300;
	int screen_display_time = 5000; //
	int red = 182;
	int green = 255;
	int blue = 0;

	status = DL_DisplaySendConfigRGB(display_id, number_of_panels,
			number_of_rows, font, brightness, scroll_start_ms, scroll_speed_ms,
			screen_display_time, red, green, blue);

	prn_status("DL_DisplaySendConfigRGB");
}

/*** ---------------------------------------------------------- ***/

/** Set default display message, available on startup */
void set_default_message(void)
{
	int number_of_panels = NUMBER_OF_PANELS;
	int number_of_rows = NUMBER_OF_ROWS;
	int font = 2;
	int brightness = 5;
	int scroll_start_ms = 1000;
	int scroll_speed_ms = 500;
	int screen_display_time = 5000; //
	int red = 0;
	int green = 255;
	int blue = 255;

	printf("Test write default message in the %d\n", display_id); /* prints Test UART on ARM */

	INIT();

	INS_STR("DEFAULT MESSAGE");
	INS_EOL();

	status = DL_DisplaySetDefaultRgb(display_id, number_of_panels,
			number_of_rows, font, brightness, scroll_start_ms, scroll_speed_ms,
			screen_display_time, text_buffer, text_buffer_size, red, green,
			blue);

	prn_status("DL_DisplaySetDefaultRgb");
}

/*** ---------------------------------------------------------- ***/

int main(void)
{
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
		time_t tmptime = timestamp;
		// ctime() convert timestamp to (local time) string
		char *timestr = ctime(&tmptime);
		printf("Current LED Display time is= (timestamp= %u) %s\n", timestamp,
				timestr);
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

	set_default_message();

	/* ---------------------------------------------------------- */

	/*
	 * Both configuration and sending text is in one function:
	 * DL_DisplaySendConfigRGB() + DL_DisplaySendText() => DL_DisplayCommonRgb()
	 *
	 * example use separated functions.
	 */

	/*--- configuration ---*/
	set_display_configuration();

	/*--- send text ---*/

	INIT();

	INS_STR("SAMPLE TEXT");
	INS_EOL();
	INS_STR("SECOND LINE");
	INS_EOL();
	INS_STR("NI\xE6 CITY");
	INS_EOL();
	INS_STR("LAST LINE");
	INS_EOL();

	status = DL_DisplaySendText(display_id, text_buffer, text_buffer_size);

	prn_status("DL_DisplaySendText");

	/* ---------------------------------------------------------- */

	puts("Press <ENTER> to reset the display, and show default message!");
	fflush(stdout);
	getchar();

	/* ---------------------------------------------------------- */

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
