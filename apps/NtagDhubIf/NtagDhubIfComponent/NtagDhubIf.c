/**
 * @file
 *
 * This app reads the state of the NTAG FD pin (GPIO23 on mangOH yellow). If it is set
 * then the NFC field has been detected. Debouncing is needed and on FD removal the
 * ndef records in the tag pushed to dhub.
 * The app also triggers on a text writes to writeNDEF and writes the text to the tag.
 * The app depends on the arduinoNtag app/stack to perform it actions via a command-line
 * interface.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless, Inc. Use of this work is subject to license.
 */


#include "legato.h"
#include "interfaces.h"
#include <time.h>
#include <sys/time.h>

static struct timeval tNow, tEnd ;
bool debounce = false;

#define NTAG_SIZE               1913
#define RES_PATH_WRITE_NDEF     "writeNDEF"

// Debounce interval in seconds
#define DEBOUNCE_INTERVAL	1
const static struct timeval tLong = {DEBOUNCE_INTERVAL, 0};

static void FdChangeCallback(bool state, void *ctx)
{

    LE_DEBUG("State change %s", state?"TRUE":"FALSE");
    int pin23 = (int) le_gpioPin23_Read();
    LE_DEBUG("Pin23 read value: %d", pin23);

    // Debounce code seems to be needed by the FD-pin circuit
    if(pin23 == 0 && !debounce) {
	LE_DEBUG("pin23 == 0 && !debounce");
	return;
    }
    else if(pin23 == 0 && debounce) {
	LE_DEBUG("pin23 == 0 && debounce");
	gettimeofday(&tNow, NULL);
	if(timercmp(&tNow, &tEnd, >)) {
	    LE_DEBUG("tNow: %ld.%06ld tEnd: %ld.%06ld\n",
		tNow.tv_sec, tNow.tv_usec,
	 	tEnd.tv_sec, tEnd.tv_usec);
	   debounce = false;
	   timerclear(&tEnd);
	}
    }
    else if(pin23 == 1 && !debounce) {
	gettimeofday (&tNow, NULL) ;
	timeradd (&tNow, &tLong, &tEnd) ;
	LE_DEBUG("pin23 == 1 && !debounce \n tNow: %ld.%06ld tLong: %ld.%06ld tEnd: %ld.%06ld\n",
		tNow.tv_sec, tNow.tv_usec,
		tLong.tv_sec, tLong.tv_usec,
		tEnd.tv_sec, tEnd.tv_usec);
	debounce = true;
	system("/legato/systems/current/bin/ntag pushndef");
    }
    else if(pin23 == 1 && debounce) {
	LE_DEBUG("pin23 == 1 && debounce");
	gettimeofday(&tNow, NULL);
	if(timercmp (&tNow, &tEnd, >)) {
	    LE_DEBUG("tNow: %ld.%06ld tEnd: %ld.%06ld\n",
		tNow.tv_sec, tNow.tv_usec,
		tEnd.tv_sec, tEnd.tv_usec);
	   debounce = false;
	   timerclear(&tEnd);
	}
    }
}

static void ConfigureFdGpio()
{
    /*
     *  Assumptions - on the WP76 GPIO23 on a yellow board has certain defaults set
     *		1. active_low - 0
     *		2. edge - none
     *		3. pull - down
     *		4. direction - in
     *  The NT3H2111_2211 by default sets FD to falling on the RF field - i.e. a pull-up
     *  Thus, all we need to change is pull-down to pull-up.
     */

    // Configure the field detection pin over sysfs/Legato
    LE_FATAL_IF(le_gpioPin23_EnablePullUp() != LE_OK,
                "Couldn't configure GPIO23 as a pull-up");

    le_gpioPin23_AddChangeEventHandler(LE_GPIOPIN23_EDGE_BOTH, FdChangeCallback, NULL, 0);
}


static void WriteNDEF(double timestamp, const char *textToWrite, void *contextPtr)
{
    // TODO: Non-portable use of ntag cmd moves.
    char cmdBuf[NTAG_SIZE + 38] = "/legato/systems/current/bin/ntag text " ;

    //LE_INFO("DHUB WriteNDEF called with textToWrite: %s", textToWrite);

    if (strlen(textToWrite) > (NTAG_SIZE - 1))
    {
        LE_INFO("Too large textToWrite: %d!!", strlen(textToWrite));
        return;
    }

    snprintf(cmdBuf + 38, NTAG_SIZE, "\"%s\"", textToWrite);
    //LE_INFO("cmdBuf: %s\n", cmdBuf);
    system(cmdBuf);
}

COMPONENT_INIT
{

    ConfigureFdGpio();

    // Text to write to tag
    LE_ASSERT(LE_OK == dhubIO_CreateOutput(RES_PATH_WRITE_NDEF, DHUBIO_DATA_TYPE_STRING, ""));
    dhubIO_AddStringPushHandler(RES_PATH_WRITE_NDEF, WriteNDEF, NULL);
    dhubIO_MarkOptional(RES_PATH_WRITE_NDEF);

}
