/*
 * catoms2DEvents.cpp
 *
 *  Created on: 2014 fevrary 1st
 *      Author: Benoît
 */

#include "catoms2DEvents.h"
#include "catoms2DScheduler.h"
#include "catoms2DWorld.h"

const int ANIMATION_DELAY=40000;
const int COM_DELAY=2000;

const double EPS=1E-5;
namespace Catoms2D {

//===========================================================================================================
//
//          MotionStartEvent  (class)
//
//===========================================================================================================

MotionStartEvent::MotionStartEvent(uint64_t t, Catoms2DBlock *block,const Catoms2DBlock *pivotBlock, int sens): BlockEvent(t,block) {
	EVENT_CONSTRUCTOR_INFO();
	eventType = EVENT_MOTION_START;
	pivot = pivotBlock->position;
	angle = sens*M_PI/3.0;
}

MotionStartEvent::MotionStartEvent(MotionStartEvent *ev) : BlockEvent(ev) {
	EVENT_CONSTRUCTOR_INFO();
}

MotionStartEvent::~MotionStartEvent() {
	EVENT_DESTRUCTOR_INFO();
}

void MotionStartEvent::consume() {
	EVENT_CONSUME_INFO();
	Catoms2DScheduler *scheduler = Catoms2D::getScheduler();
	Catoms2DBlock *rb = (Catoms2DBlock *)concernedBlock;
  Catoms2DWorld::getWorld()->disconnectBlock(rb);
  rb->setColor(DARKGREY);
	scheduler->schedule(new MotionStepEvent(scheduler->now() + ANIMATION_DELAY, rb,pivot,angle));
}

const string MotionStartEvent::getEventName() {
	return("MotionStart Event");
}

//===========================================================================================================
//
//          MotionStepEvent  (class)
//
//===========================================================================================================

MotionStepEvent::MotionStepEvent(uint64_t t, Catoms2DBlock *block,const Vecteur &p,double angle2goal): BlockEvent(t,block) {
	EVENT_CONSTRUCTOR_INFO();
	eventType = EVENT_MOTION_STEP;

    pivot = p;
    angle = angle2goal;
}

MotionStepEvent::MotionStepEvent(MotionStepEvent *ev) : BlockEvent(ev) {
	EVENT_CONSTRUCTOR_INFO();
}

MotionStepEvent::~MotionStepEvent() {
	EVENT_DESTRUCTOR_INFO();
}

void MotionStepEvent::consume() {
	EVENT_CONSUME_INFO();
	Catoms2DBlock *rb = (Catoms2DBlock*)concernedBlock;

	Catoms2DScheduler *scheduler = Catoms2D::getScheduler();


    Matrice roty;
    if (angle<M_PI/(3.0*5)) {
        roty.setRotationY(angle);
        Vecteur BA = rb->position - pivot;
        Vecteur BC = roty*BA;
        rb->position = pivot+BC;
        Catoms2DWorld::getWorld()->updateGlData(rb);
        scheduler->schedule(new MotionStopEvent(scheduler->now() + ANIMATION_DELAY, rb));
	} else {
        roty.setRotationY(M_PI/(3.0*5));
        Vecteur BA = rb->position - pivot;
        Vecteur BC = roty*BA;
        rb->position = pivot+BC;
        Catoms2DWorld::getWorld()->updateGlData(rb);
        scheduler->schedule(new MotionStepEvent(scheduler->now() + ANIMATION_DELAY,rb, pivot,angle-M_PI/(3.0*5)));
	}
}

const string MotionStepEvent::getEventName() {
	return("MotionStep Event");
}

//===========================================================================================================
//
//          MotionStepEvent  (class)
//
//===========================================================================================================

MotionStopEvent::MotionStopEvent(uint64_t t, Catoms2DBlock *block): BlockEvent(t,block) {
	EVENT_CONSTRUCTOR_INFO();
	eventType = EVENT_MOTION_STOP;
}

MotionStopEvent::MotionStopEvent(MotionStepEvent *ev) : BlockEvent(ev) {
	EVENT_CONSTRUCTOR_INFO();
}

MotionStopEvent::~MotionStopEvent() {
	EVENT_DESTRUCTOR_INFO();
}

void MotionStopEvent::consume() {
	EVENT_CONSUME_INFO();
	Catoms2DBlock *rb = (Catoms2DBlock*)concernedBlock;
    rb->setColor(YELLOW);

/*Transformer les coordonnées GL en coordonnées grille*/

    Catoms2DWorld *wrld=Catoms2DWorld::getWorld();
    int ix = int(rb->position.pt[0]),
        iy = int(rb->position.pt[1]),
        iz = int(rb->position.pt[2]);
	wrld->setGridPtr(ix,iy,iz,rb);

	stringstream info;
    info.str("");
    info << "connect Block " << rb->blockId;
    getScheduler()->trace(info.str(),rb->blockId,LIGHTBLUE);
	wrld->connectBlock(rb);
    Catoms2DScheduler *scheduler = Catoms2D::getScheduler();
    scheduler->schedule(new MotionEndEvent(scheduler->now() + ANIMATION_DELAY, rb));
}

const string MotionStopEvent::getEventName() {
	return("MotionStop Event");
}

//===========================================================================================================
//
//          MotionEndEvent  (class)
//
//===========================================================================================================

MotionEndEvent::MotionEndEvent(uint64_t t, Catoms2DBlock *block): BlockEvent(t,block) {
	EVENT_CONSTRUCTOR_INFO();
	eventType = EVENT_MOTION_END;
}

MotionEndEvent::MotionEndEvent(MotionEndEvent *ev) : BlockEvent(ev) {
	EVENT_CONSTRUCTOR_INFO();
}

MotionEndEvent::~MotionEndEvent() {
	EVENT_DESTRUCTOR_INFO();
}

void MotionEndEvent::consume() {
	EVENT_CONSUME_INFO();
	Catoms2DBlock *rb = (Catoms2DBlock*)concernedBlock;
	concernedBlock->blockCode->processLocalEvent(EventPtr(new MotionEndEvent(date+COM_DELAY,rb)));
}

const string MotionEndEvent::getEventName() {
	return("MotionEnd Event");
}


} // Catoms2D namespace
