//--------------------------------------------------------------------------------------------------
/**
 * Implementation of the Octave(tm) screen.
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//--------------------------------------------------------------------------------------------------

#include "cmdLine.h"
#include "octaveScreen.h"
#include "mainMenu.h"
#include "instantGratification.h"


//--------------------------------------------------------------------------------------------------
/**
 * Fetch the cellular module's IMEI.
 */
//--------------------------------------------------------------------------------------------------
static const char* GetImei
(
    void
)
//--------------------------------------------------------------------------------------------------
{
    // Buffer to hold the IMEI (which is normally only 15 characters long).
    static char Imei[32] = "unknown";

    le_result_t result = le_info_GetImei(Imei, sizeof(Imei));

    if (result != LE_OK)
    {
        (void)snprintf(Imei, sizeof(Imei), "ERROR (%s)", LE_RESULT_TXT(result));
    }

    return Imei;
}


//--------------------------------------------------------------------------------------------------
/**
 * Fetch the SIM's ICCID.
 */
//--------------------------------------------------------------------------------------------------
static const char* GetIccid
(
    void
)
//--------------------------------------------------------------------------------------------------
{
    le_sim_Id_t simId = le_sim_GetSelectedCard();

    // Buffer to hold the ICCID (which is normally up to 20 characters long).
    static char Iccid[32] = "unknown";

    if (!le_sim_IsPresent(simId))
    {
        (void)snprintf(Iccid, sizeof(Iccid), "ERROR: SIM card not detected");
    }
    else
    {
        le_result_t result = le_sim_GetICCID(simId, Iccid, sizeof(Iccid));

        if (result != LE_OK)
        {
            (void)snprintf(Iccid, sizeof(Iccid), "ERROR (%s)", LE_RESULT_TXT(result));
        }
    }

    return Iccid;
}


//--------------------------------------------------------------------------------------------------
/**
 * Enable instant gratification interaction features.
 */
//--------------------------------------------------------------------------------------------------
static void EnableInstantGratification
(
    void* contextPtr
)
//--------------------------------------------------------------------------------------------------
{
    instantGratification_Enable();
    cmdLine_Refresh();
}

//--------------------------------------------------------------------------------------------------
/**
 * Disable instant gratification interaction features.
 */
//--------------------------------------------------------------------------------------------------
static void DisableInstantGratification
(
    void* contextPtr
)
//--------------------------------------------------------------------------------------------------
{
    instantGratification_Disable();
    cmdLine_Refresh();
}


//--------------------------------------------------------------------------------------------------
/**
 * Draw the Octave screen.
 */
//--------------------------------------------------------------------------------------------------
static void Draw
(
    void
)
//--------------------------------------------------------------------------------------------------
{
    printf("You may need the following information to register\n"
           "your device with the Sierra Wireless Octave IoT data\n"
           "orchestration service:\n"
           "\n"
           "   IMEI:  %s\n"
           "   ICCID: %s\n"
           "\n",
           GetImei(),
           GetIccid());

    if (instantGratification_IsEnabled())
    {
        printf("Interactive out-of-box experience is ENABLED.\n"
               "\n"
               "To improve your experience with Octave, you may wish to\n"
               "disable the interactive mangOH Yellow out-of-box\n"
               "experience features (blinking lights and button control).\n");

        cmdLine_MenuMode();
        cmdLine_AddMenuEntry("DISABLE out-of-box experience", DisableInstantGratification, NULL);
    }
    else
    {
        printf("Interactive out-of-box experience is DISABLED.\n"
               "\n"
               "The interactive mangOH Yellow out-of-box experience\n"
               "features (blinking lights and button control) are\n"
               "currently disabled. Press 1 to re-enable these features.\n");

        cmdLine_MenuMode();
        cmdLine_AddMenuEntry("ENABLE out-of-box experience", EnableInstantGratification, NULL);
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * Leave the Octave screen.
 */
//--------------------------------------------------------------------------------------------------
static void Leave
(
    void
)
//--------------------------------------------------------------------------------------------------
{
    mainMenu_Enter();
}


//--------------------------------------------------------------------------------------------------
/**
 * Screen object for the screen.
 */
//--------------------------------------------------------------------------------------------------
static const Screen_t Screen =
{
    .drawFunc = Draw,
    .leaveFunc = Leave,
    .title = "Octave(tm)"
};


//--------------------------------------------------------------------------------------------------
/**
 * Initialize the module.
 */
//--------------------------------------------------------------------------------------------------
void octaveScreen_Init
(
    void
)
//--------------------------------------------------------------------------------------------------
{
    // Nothing needs to be done here.
}


//--------------------------------------------------------------------------------------------------
/**
 * Enter the screen.
 */
//--------------------------------------------------------------------------------------------------
void octaveScreen_Enter
(
    void
)
//--------------------------------------------------------------------------------------------------
{
    cmdLine_SetCurrentScreen(&Screen);
}
