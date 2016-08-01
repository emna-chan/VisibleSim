/*
 * clock.h
 *
 *  Created on: 26 march 2015
 *      Author: andre
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <cstdint>

using namespace std;

namespace BaseSimulator {

//class BuildingBlock;

/**
 * \brief Abstract class parent of any clock
 * Simuate RTC (Real-Time Counter) behaviour
 */
class Clock {
public:

  /**
   * @brief Clock destructor.
   */
  virtual ~Clock() {};
  
  /**
   * @brief returns the local time for the simulator time in parameter.
   * @para simTime Simulator time for which the local time is requested.
   */ 
  virtual uint64_t getTime(uint64_t simTime) = 0;

  /**
   * @brief returns the local time for the current simulator time in parameter.
   */ 
  uint64_t getTime();
  
  /**
   * @brief returns the simulator time for the local time in parameter.
   * @para localTime Local time for which the simulator time is requested.
   */
  virtual uint64_t getSchedulerTimeForLocalTime(uint64_t localTime) = 0;
};

/**
 * \brief Perfect Clock Class. At any time, local time and simulated time are equal.
 */
class PerfectClock : public Clock {
public:

  /**
   * @brief PerfectClock default constructor.
   */
  PerfectClock();

  /**
   * @brief PerfectClock destructor.
   */
  ~PerfectClock();
  
  uint64_t getTime(uint64_t simTime);
  uint64_t getSchedulerTimeForLocalTime(uint64_t localTime);
};


 
}

#endif // CLOCK_H_
