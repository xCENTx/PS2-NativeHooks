#include <debug.h>
#include <kernel.h>
#include <sifrpc.h>

#define SOCOM_ELF_PATH "cdrom0:\\SCUS_971.34;1"
#define SOCOM_MENU_OPTION "-m"
#define SOCOM_MENU_RESOURCE "dlgReturnFromNTGUI2.rdr"

void wait(int seconds)
{
    for (int i = 0; i < seconds: i++)
        DelayThread(1000000)
}

int main(void)
{
    char *args[] =
    {
        SOCOM_MENU_OPTION,
        SOCOM_MENU_RESOURCE,
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