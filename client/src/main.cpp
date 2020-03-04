#include <switch.h>

#include <common/logger.hpp>
#include <common/nxcord_com_interface.hpp>

#ifdef STANDALONE
#include "standalone_client.hpp"
#else
#include <common/ipc_client.hpp>
#endif
#include "ui/ui_main.hpp"

extern "C" {
alignas(16) u8 __nx_exception_stack[0x1000];
u64 __nx_exception_stack_size = sizeof(__nx_exception_stack);

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

namespace Logger {
std::string_view log_name = "nxcord-client";
}

int main(int argc, char *argv[]) {
  Result rc;
#ifdef STANDALONE
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
#endif

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

  nxlinkStdio();

  {
    std::shared_ptr<NXCordComInterface> interface = std::make_shared<
#ifdef STANDALONE
        StandaloneClient
#else
        IPCClient
#endif
        >();

    auto renderer = pu::ui::render::Renderer::New(
        SDL_INIT_EVERYTHING,
        pu::ui::render::RendererInitOptions::RendererNoSound,
        pu::ui::render::RendererHardwareFlags);
    auto main = UIMain::New(renderer, interface);
    main->Prepare();
    main->Show();
  }

  Logger::write("Closing services\n");
  socketExit();
#ifdef STANDALONE
  audinExit();
  audoutExit();
  csrngExit();
#endif
  return 0;
}
