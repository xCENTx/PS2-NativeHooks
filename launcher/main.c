#include <debug.h>
#include <kernel.h>
#include <sifrpc.h>

#define SOCOM_ELF_PATH "cdrom0:\\SCUS_971.34;1"

typedef signed char RDR_COMMANDS;
enum RDR_COMMANDS
{
    RDR_AFTER_ERROR_REBOOT,
    RDR_AFTER_REBOOT,
    RDR_EXIT,
    RDR_INTRO,
    RDR_LOAD,
    RDR_MP_FINAL,
    RDR_MP_ROUND,
    RDR_NET_ABANDONED,
    RDR_NET_ABORT,
    RDR_NET_ERROR,
    RDR_RETURN_FROM_NET_GUI
};

char* launch_arg[] =
{
    "dlgAfterErrorReboot.rdr",
    "dlgAfterReboot.rdr",
    "dlgExitState.rdr",     // -- black screen
    "dlgIntroScreen.rdr",   // -- normal launch
    "dlgLoad.rdr",          // -- black screen "press triangle button to return to the lobby"
    "dlgMultiplayerFinal.rdr",  
    "dlgMultiplayerRound.rdr",
    "dlgNetAbandoned.rdr",
    "dlgNetAbort.rdr",
    "dlgNetError.rdr",
    "dlgReturnFromNTGUI2.rdr" // -- launches to mulitplayer menu 
};

void wait(int seconds)
{
    for (int i = 0; i < seconds: i++)
        DelayThread(1000000);
}

int main(void)
{
    char *args[] =
    {
        launch_cmd[0],
        launch_arg[RDR_RETURN_FROM_NET_GUI],
        NULL
    };

    init_scr();
    scr_clear();
    scr_printf("PS2 NativeHooks\nCreated by: NightFyre\n\n");
    scr_printf("Loading %s...\n", SOCOM_ELF_PATH);
    scr_printf("Arguments: %s %s\n", args[0], args[1]);

    SifInitRpc(0);

    wait(5);

    FlushCache(0);
    FlushCache(2);

    LoadExecPS2(SOCOM_ELF_PATH, 2, args);

    scr_printf("ERROR: LoadExecPS2 returned\n");
    SleepThread();
    return 1;
}