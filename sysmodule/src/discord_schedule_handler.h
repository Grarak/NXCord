#pragma once
#include <sleepy_discord/asio_schedule.h>

#include <map>

class DiscordScheduleHandler : public SleepyDiscord::GenericScheduleHandler {
 private:
  struct ScheduledFunction {
    SleepyDiscord::TimedTask function;
    time_t scheduled_time;
    bool enabled;
  };

  size_t _schedule_counter = 0;
  time_t _previous_time = 0;
  std::map<size_t, ScheduledFunction> _scheduled_functions;

 public:
  SleepyDiscord::Timer schedule(SleepyDiscord::TimedTask code,
                                const time_t milliseconds) override;

  void tick();
};
