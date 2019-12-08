#include <iostream>
#include <switch.h>

int main(int argc, char **argv)
{
#ifdef APPLET
    consoleInit(NULL);
#endif
    printf("Hello World\n");

    while (appletMainLoop()) {
#ifdef APPLET
        consoleUpdate(NULL);
#else
        svcSleepThread(3e+7L);
#endif
    }

#ifdef APPLET
    consoleExit(NULL);
#endif

    return 0;
}
