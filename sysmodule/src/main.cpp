#include <switch.h>

#include "nxcord_client.h"

extern "C" {
#ifndef APPLET
// Adjust size as needed.
#define INNER_HEAP_SIZE 0x40'000

u32 __nx_applet_type = AppletType_None;
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char nx_inner_heap[INNER_HEAP_SIZE];

void __libnx_initheap(void) {
  void *addr = nx_inner_heap;
  size_t size = nx_inner_heap_size;

  // Newlib
  extern char *fake_heap_start;
  extern char *fake_heap_end;

  fake_heap_start = (char *)addr;
  fake_heap_end = (char *)addr + size;
}
#endif

void userAppInit(void) {
#ifndef APPLET
  // Seems like every thread on the switch needs to sleep for a little
  // or it will block the entire console
  // Specifically in Kosmos Toolbox's case, you need to wait about 0.2 sec
  // or it won't let you turn it on/off the sysmodule after a few tries
  svcSleepThread(2e+8L);
#endif

  Result rc = smInitialize();
  if (R_FAILED(rc)) {
    fatalThrow(rc);
  }

  rc = socketInitializeDefault();
  if (R_FAILED(rc)) {
    fatalThrow(rc);
  }

  rc = csrngInitialize();
  if (R_FAILED(rc)) {
    fatalThrow(rc);
  }

  rc = audoutInitialize();
  if (R_FAILED(rc)) {
    fatalThrow(rc);
  }

#ifdef APPLET
  nxlinkStdio();
#endif
}

void userAppExit(void) {
  printf("Closing services\n");
  audoutExit();
  csrngExit();
  socketExit();
  smExit();
}

alignas(16) u8 __nx_exception_stack[0x1000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
__attribute__((weak)) u32 __nx_exception_ignoredebug = 1;

void __libnx_exception_handler(ThreadExceptionDump *ctx) {
  printf("Sysmodule crashed with error 0x%x\n", ctx->error_desc);
}
}

int main(int argc, char **argv) {
#ifdef APPLET
  consoleInit(NULL);
#endif

  {
    NXCordClient client(TOKEN);

    while (appletMainLoop()) {
      client.tick();
#ifdef APPLET
      hidScanInput();
      u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
      if (kDown & KEY_B) {
        break;
      }
      consoleUpdate(NULL);
#else
      svcSleepThread(2e+7);
#endif
    }
  }

  userAppExit();
#ifdef APPLET
  consoleExit(NULL);
#endif

  return 0;
}
