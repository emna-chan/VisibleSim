#include "clock.h"
#include "scheduler.h"
#include "buildingBlock.h"

namespace BaseSimulator {

//===========================================================================================================
//
//          Clock  (class)
//
//===========================================================================================================

uint64_t Clock::getTime() {
    return getTime(getScheduler()->now());
}

//===========================================================================================================
//
//          PerfectBlock  (class)
//
//===========================================================================================================
  
PerfectClock::PerfectClock() {}

PerfectClock::~PerfectClock() {}

uint64_t PerfectClock::getTime(uint64_t simTime) {
  return simTime;
}
  
uint64_t PerfectClock::getSchedulerTimeForLocalTime(uint64_t localTime) {
  return localTime;
}
 
}


