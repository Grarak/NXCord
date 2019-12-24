#include <chrono>

#include "discord_schedule_handler.h"

DiscordScheduleHandler::DiscordScheduleHandler(DiscordClient* client)
    : _client(client) {}

SleepyDiscord::Timer DiscordScheduleHandler::schedule(
    SleepyDiscord::TimedTask code, const time_t milliseconds) {
  size_t id = _schedule_counter++;
  ScheduledFunction scheduled_function = {
      std::move(code), milliseconds, _client->getEpochTimeMillisecond(), true};
  _scheduled_functions[_schedule_counter] = scheduled_function;
  return SleepyDiscord::Timer([this, id]() {
    auto it = this->_scheduled_functions.find(id);
    if (it != this->_scheduled_functions.end()) {
      it->second.enabled = false;
    }
  });
}

void DiscordScheduleHandler::tick() {
  for (auto& pair : _scheduled_functions) {
    if (!pair.second.enabled) {
      continue;
    }

    time_t current_time = _client->getEpochTimeMillisecond();
    time_t diff = current_time - pair.second.previous_time;
    if (pair.second.scheduled_time <= diff) {
      pair.second.enabled = false;
      pair.second.function();
    } else {
      pair.second.scheduled_time -= diff;
    }
    pair.second.previous_time = current_time;
  }

  // Remove disabled functions
  auto it = _scheduled_functions.begin();
  while (it != _scheduled_functions.end()) {
    if (!it->second.enabled) {
      it = _scheduled_functions.erase(it);
    } else {
      ++it;
    }
  }
}

void DiscordScheduleHandler::clear() { _scheduled_functions.clear(); }
