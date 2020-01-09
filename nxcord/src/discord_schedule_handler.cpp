#include <chrono>
#include <common/utils.hpp>
#include <nxcord/discord_schedule_handler.hpp>
#include <set>

SleepyDiscord::Timer DiscordScheduleHandler::schedule(
    SleepyDiscord::TimedTask code, const time_t milliseconds) {
  size_t id = _schedule_counter++;
  ScheduledFunction scheduled_function = {std::move(code), milliseconds,
                                          Utils::current_time_millis(), true};
  _scheduled_functions[id] = scheduled_function;
  return SleepyDiscord::Timer([this, id]() {
    auto it = this->_scheduled_functions.find(id);
    if (it != this->_scheduled_functions.end()) {
      it->second.enabled = false;
    }
  });
}

void DiscordScheduleHandler::tick() {
  std::set<size_t> disabled_schedules;
  for (auto& pair : _scheduled_functions) {
    if (!pair.second.enabled) {
      disabled_schedules.insert(pair.first);
      continue;
    }

    time_t current_time = Utils::current_time_millis();
    time_t diff = current_time - pair.second.previous_time;
    if (pair.second.scheduled_time <= diff) {
      pair.second.enabled = false;
      disabled_schedules.insert(pair.first);
      pair.second.function();
    } else {
      pair.second.scheduled_time -= diff;
      pair.second.previous_time = current_time;
    }
  }

  // Remove disabled functions
  for (size_t key : disabled_schedules) {
    _scheduled_functions.erase(key);
  }
}

void DiscordScheduleHandler::clear() { _scheduled_functions.clear(); }
