#include <switch.h>

#include <common/logger.hpp>
#include <nxcord/nxcord_client.hpp>
#include <stratosphere.hpp>

#include "ipc_server.hpp"

extern "C" {
#ifndef APPLICATION
// Adjust size as needed.
#define INNER_HEAP_SIZE 0x100000

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
#ifndef APPLICATION
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

  SocketInitConfig sockConf = {
      .bsdsockets_version = 1,

      .tcp_tx_buf_size = 0x800,
      .tcp_rx_buf_size = 0x1000,
      .tcp_tx_buf_max_size = 0x2EE0,
      .tcp_rx_buf_max_size = 0x2EE0,

      .udp_tx_buf_size = 0x800,
      .udp_rx_buf_size = 0x1000,

      .sb_efficiency = 4,
  };
  rc = socketInitialize(&sockConf);
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

  rc = audinInitialize();
  if (R_FAILED(rc)) {
    fatalThrow(rc);
  }

#ifdef APPLICATION
  nxlinkStdio();
#endif
}

void userAppExit(void) {
  Logger::write("Closing services\n");
  audinExit();
  audoutExit();
  csrngExit();
  socketExit();
  smExit();
}

alignas(16) u8 __nx_exception_stack[0x1000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);
__attribute__((weak)) u32 __nx_exception_ignoredebug = 1;

void __libnx_exception_handler(ThreadExceptionDump *ctx) {
  Logger::write("Crashed with error 0x%x\n", ctx->error_desc);

  for (int i = 0; i < 29; i++) {
    Logger::write("[X%d]: 0x%lx\n", i, ctx->cpu_gprs[i].x);
  }
  Logger::write("fp: 0x%lx\n", ctx->fp.x);
  Logger::write("lr: 0x%lx\n", ctx->lr.x);
  Logger::write("sp: 0x%lx\n", ctx->sp.x);
  Logger::write("pc: 0x%lx\n", ctx->pc.x);

  Logger::write("pstate: 0x%x\n", ctx->pstate);
  Logger::write("afsr0: 0x%x\n", ctx->afsr0);
  Logger::write("afsr1: 0x%x\n", ctx->afsr1);
  Logger::write("esr: 0x%x\n", ctx->esr);

  Logger::write("far: 0x%lx\n", ctx->far.x);
}
}

namespace ams::result {
bool CallFatalOnResultAssertion = true;
}

namespace Logger {
std::string_view log_name = "nxcord-sys";
}

int main(int argc, char **argv) {
#ifdef APPLICATION
  consoleInit(nullptr);
#endif

  {
    Logger::write("Starting new client\n");
    NXCordClient client;
    auto settings = NXCordSettings::New();
    client.loadSettings(settings);
    std::mutex client_mutex;
#ifdef APPLICATION
    client.startConnection();
    bool joined = false;
#else
    IPCServer ipc_server(client, client_mutex);
#endif

    while (appletMainLoop()) {
      {
#ifdef APPLICATION
        if (!joined && client.isConnected()) {
          const std::vector<IPCStruct::DiscordServer> &servers =
              client.getCachedServers();
          if (servers.size() > 0) {
            const std::vector<IPCStruct::DiscordChannel> &channels =
                client.getCachedChannels(servers[0].id);
            for (const auto &channel : channels) {
              if (channel.type == IPCStruct::DiscordChannelType::SERVER_VOICE) {
                client.joinVoiceChannel(channel.serverId, channel.id);
                break;
              }
            }
          }
          joined = true;
        }
#else
        std::scoped_lock lock(client_mutex);
#endif
        client.tick();
      }

#ifdef APPLICATION
      hidScanInput();
      u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
      if (kDown & KEY_B) {
        break;
      }
      consoleUpdate(nullptr);
#else
      svcSleepThread(2e+7);
#endif
    }
  }

  userAppExit();
#ifdef APPLICATION
  consoleExit(nullptr);
#endif

  return 0;
}
