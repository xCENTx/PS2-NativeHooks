#ifndef SOCOM_GAME_H
#define SOCOM_GAME_H

#include "structs.h"


#define SOCOM_ELF_PATH "cdrom0:\\SCUS_972.05;1"

// ------------------------------------------------------------
// Globals
// ------------------------------------------------------------


// ------------------------------------------------------------
// Native functions
// ------------------------------------------------------------


// ------------------------------------------------------------
// Constants
// ------------------------------------------------------------

char* launch_cmd[] =
{
    "-d",
    "-p",
    "-m",
    "-V",
    "-h",
    "-f",
    "-F",
    "-R",
    "-A",
    "-a",
    "-s",
    "-z",
    "-Z",
};

char* load_arg[] =
{
    "dlgExitState.rdr",
    "dlgGameLobby.rdr",
    "dlgIntroScreen.rdr",
    "dlgLoad.rdr",
    "dlgMultiplayerFinal.rdr",  
    "dlgMultiplayerRound.rdr",
    "dlgNetAbort.rdr",
    "dlgNetError.rdr"
};

char* launch_flag[] =
{
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

#endif
