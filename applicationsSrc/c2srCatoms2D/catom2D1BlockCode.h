/*
 * catom2D1BlockCode.h
 *
 *  Created on: 25 Nov 2015
 *      Author: Andre Naz
 */

#ifndef CATOM2D1BLOCKCODE_H_
#define CATOM2D1BLOCKCODE_H_

#include "robots/catoms2D/catoms2DBlock.h"
#include "robots/catoms2D/catoms2DBlockCode.h"
#include "robots/catoms2D/catoms2DSimulator.h"

#include "simulationParameters.h"
#include "map.h"
#include "c2sr.h"

class Catoms2D1BlockCode : public Catoms2D::Catoms2DBlockCode {
 public:
  static SimulationParameters simParams;

  Scheduler *scheduler;
  Catoms2D::Catoms2DBlock *catom2D;

  Catoms2D1BlockCode (Catoms2D::Catoms2DBlock *host);
  ~Catoms2D1BlockCode ();

  // Distributed map construction
  Map* map;

  // C2SR
  C2SR* c2sr;

  void startup() override;
  void processLocalEvent(EventPtr pev) override;

  void setSimulationParameters();
  void setCommunicationRate();
  void setMotionSpeed();

  void scheduleC2SRStart();

  static BlockCode *buildNewBlockCode(BuildingBlock *host);
};

#endif /* CATOM2DBLOCKCODE_H_ */
