#pragma once
#include <sleepy_discord/asio_schedule.h>

#include <map>

#include "discord_client.hpp"

class DiscordScheduleHandler : public SleepyDiscord::GenericScheduleHandler {
 private:
  struct ScheduledFunction {
    SleepyDiscord::TimedTask function;
    time_t scheduled_time;
    time_t previous_time = 0;
    bool enabled;
  };

  size_t _schedule_counter = 0;
  std::map<size_t, ScheduledFunction> _scheduled_functions;

 public:
  SleepyDiscord::Timer schedule(SleepyDiscord::TimedTask code,
                                const time_t milliseconds) override;

  void tick();
  void clear();
};
