#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <switch.h>

#ifndef APPLET
#define HEAP_SIZE 0x40000
extern "C"
{
// Adjust size as needed.
#define INNER_HEAP_SIZE 0x40'000

u32 __nx_applet_type = AppletType_None;
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char nx_inner_heap[INNER_HEAP_SIZE];

void __libnx_initheap(void)
{
    void *addr = nx_inner_heap;
    size_t size = nx_inner_heap_size;

    // Newlib
    extern char *fake_heap_start;
    extern char *fake_heap_end;

    fake_heap_start = (char *)addr;
    fake_heap_end = (char *)addr + size;
}

alignas(16) u8 __nx_exception_stack[0x1000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
__attribute__((weak)) u32 __nx_exception_ignoredebug = 1;
}
#endif

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
