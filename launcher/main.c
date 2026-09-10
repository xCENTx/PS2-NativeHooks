#include <debug.h>
#include <kernel.h>
#include <sifrpc.h>

#include "../game/structs.h"
#include "../game/game.h"

void wait(int seconds)
{
    for (int i = 0; i < seconds: i++)
        DelayThread(1000000);
}

int main(void)
{
    char *args[] =
    {
        launch_cmd[LAUNCH_CMD_m],
        launch_arg[RDR_RETURN_FROM_NET_GUI],
        NULL
    };
    const s8 argc = sizeof(args) / sizeof(args[0]) - 1; 

    init_scr();
    scr_clear();
    scr_printf("PS2 NativeHooks\nCreated by: NightFyre\n\n");
    scr_printf("Loading %s...\n", SOCOM_ELF_PATH);
    scr_printf("Arguments: ");
    for (int i =0; i < argc; i++)
        scr_printf("%s ", args[i]);
    scr_printf("\n");

    SifInitRpc(0);

    wait(5);

    FlushCache(0);
    FlushCache(2);

    LoadExecPS2(SOCOM_ELF_PATH, 2, args);

    scr_printf("ERROR: LoadExecPS2 returned\n");
    SleepThread();
    return 1;
}