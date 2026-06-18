#pragma once

#include <Arduino.h>

#include "../SIM7080/SIM7080.h"

class TimeService {
 public:
  void configureLocalTimezone();
  void initializeNtpTime();
  void syncFromSIM(SIM7080& sim7080);  // NOLINT(runtime/references)
  uint64_t currentTimestampMs() const;

 private:
  static constexpr const char* kNtpServer1 = "149.143.87.22";
  static constexpr const char* kNtpServer2 = "82.65.248.56";
  static constexpr const char* kLocalTimeZone = "CET-1CEST,M3.5.0/2,M10.5.0/3";
};
