#ifndef MICHI_DAILY_SCHEDULE_H
#define MICHI_DAILY_SCHEDULE_H
#include <stdint.h>
namespace Michi {
// America/Mexico_City: UTC-6 sin cambio estacional desde 2022.
static const int32_t UTC_OFFSET_SECONDS = -6 * 60 * 60;
struct DailySchedule {
  bool configured = false;
  bool enabled = false;
  int onHour = 0, onMinute = 0, offHour = 0, offMinute = 0;
  bool valid() const {
    return configured && onHour >= 0 && onHour < 24 && offHour >= 0 && offHour < 24 &&
      onMinute >= 0 && onMinute < 60 && offMinute >= 0 && offMinute < 60 &&
      (onHour != offHour || onMinute != offMinute);
  }
  bool operator==(const DailySchedule &other) const {
    return configured == other.configured && enabled == other.enabled && onHour == other.onHour &&
      onMinute == other.onMinute && offHour == other.offHour && offMinute == other.offMinute;
  }
};
struct Decision { bool turnOn; int64_t eventAt; };
inline Decision latestEvent(const DailySchedule &s, int64_t utcSeconds) {
  const int64_t day = (utcSeconds + UTC_OFFSET_SECONDS) / 86400;
  int64_t on = day * 86400 + s.onHour * 3600 + s.onMinute * 60 - UTC_OFFSET_SECONDS;
  int64_t off = day * 86400 + s.offHour * 3600 + s.offMinute * 60 - UTC_OFFSET_SECONDS;
  if (on > utcSeconds) on -= 86400;
  if (off > utcSeconds) off -= 86400;
  return {on > off, on > off ? on : off};
}
class ScheduleEngine {
  int64_t appliedEvent = 0;
 public:
  void reset() { appliedEvent = 0; }
  bool due(const DailySchedule &s, int64_t utcSeconds, Decision &decision) const {
    if (!s.enabled || !s.valid() || utcSeconds < 1700000000LL) return false;
    decision = latestEvent(s, utcSeconds);
    return decision.eventAt != appliedEvent;
  }
  void applied(const Decision &decision) { appliedEvent = decision.eventAt; }
};
}
#endif
