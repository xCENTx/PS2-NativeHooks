#include <debug.h>
#include <kernel.h>
#include <sifrpc.h>

#define SOCOM_ELF_PATH "cdrom0:\\SCUS_971.34;1"

typedef signed char RDR_COMMANDS;
enum RDR_COMMANDS
{
    RDR_INTRO,
    RDR_EXIT,
    RDR_LOAD,
    RDR_MP_FINAL,
    RDR_MP_ROUND,
    RDR_NET_ABANDONED,
    RDR_NET_ABORT,
    RDR_NET_ERROR,
    RDR_NET_RETURN,
    RDR_RETURN_FROM_NET_GUI

};

char* launch_arg[] =
{
    "dlgIntroScreen.rdr",   // -- normal launch
    "dlgExitState.rdr",     // -- black screen
    "dlgLoad.rdr",          // -- black screen "press triangle button to return to the lobby"
    "dlgMultiplayerFinal.rdr",  
    "dlgMultiplayerRound.rdr",
    "dlgNetAbandoned.rdr",
    "dlgNetAbort.rdr",
    "dlgNetError.rdr",
    "dlgNetReturn.rdr",
    "dlgReturnFromNTGUI2.rdr" // -- launches to mulitplayer menu 
};

char* launch_cmd[] = 
{
    "-m",
    "--AP",
    "--ammo",
    "--cdsounds",
    "--nosounds",
    "--displaysounds",
    "--debugmusic",
    "--ffire",
    "--nofireanim",
    "--noimpactanim",
    "--nowepintersection",
    "--ailog",
    "--noenemy",
    "--noalpha",
    "--nobravo",
    "--aid",
    "--isolate",        // <character>
    "--noaifilter",
    "--player_grid",
    "--player_grid",    // <number>
    "--noobj",
    "--nodie",
    "--nosee",
    "--log",
    "--dumplog",
    "--nomenu",
    "--menu",
    "--invert_pitch",
    "--pad2",
    "--squirm",
    "--two_player",
    "--no_radar",
    "--radar",
    "--multi",
    "--medius",
    "--nomedius",
    "--voice",
    "--voiced",
    "--noghost",
    "--e3seal",
    "--e3terr",
    "--dlgrdr",
    "--nodi",
    "--noalt",
    "--noint",
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