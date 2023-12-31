#include "neighborhood.h"
#include "neighborRestriction.h"
#include "robots/catoms3D/catoms3DWorld.h"

#define MSG_TIME 0//rand()%10
#define MSG_TIME_ADD 1//1000+rand()%100

int Neighborhood::numberBlockedModules = 0;
int Neighborhood::numberMessagesToAddBlock = 0;

Neighborhood::Neighborhood(Catoms3D::Catoms3DBlock *c, Reconf *r, SyncNext *sn, SyncPrevious *sp, BlockCodeBuilder bcb)
{
    catom = c;
    reconf = r;
    blockCodeBuilder = bcb;
    syncNext = sn;
    syncPrevious = sp;
}

void Neighborhood::addNeighborToLeft()
{
    Cell3DPosition pos = catom->position.offsetX(-1);
    if (BlockCode::target->isInTarget(pos) && !catom->getInterface(pos)->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDLEFTBLOCK_EVENT_ID, catom->blockId)) {
            getScheduler()->schedule(new AddLeftBlock_event(getScheduler()->now()+MSG_TIME_ADD, catom));
            reconf->nChildren++;
        }
    }
}

void Neighborhood::addNeighborToRight()
{
    Cell3DPosition pos = catom->position.offsetX(1);
    if (BlockCode::target->isInTarget(pos) && !catom->getInterface(pos)->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDRIGHTBLOCK_EVENT_ID, catom->blockId)) {
            getScheduler()->schedule(new AddRightBlock_event(getScheduler()->now()+MSG_TIME_ADD, catom));
            reconf->nChildren++;
        }
    }
}

void Neighborhood::addNextLineNeighbor()
{
    if (BlockCode::target->isInTarget(catom->position.offsetY(1)) &&
        !catom->getInterface(catom->position.offsetY(1))->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDNEXTLINE_EVENT_ID, catom->blockId)
                && !catom->getInterface(catom->position.offsetY(1))->isConnected()) {
            AddNextLine_event *evt = new AddNextLine_event(getScheduler()->now()+MSG_TIME_ADD, catom);
            getScheduler()->schedule(evt);
            reconf->nChildren++;
        }
    }
}

void Neighborhood::addPreviousLineNeighbor()
{
    if (BlockCode::target->isInTarget(catom->position.offsetY(-1)) &&
        !catom->getInterface(catom->position.offsetY(-1))->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDPREVIOUSLINE_EVENT_ID, catom->blockId)
                && !catom->getInterface(catom->position.offsetY(-1))->isConnected()) {
            AddPreviousLine_event *evt = new AddPreviousLine_event(getScheduler()->now()+MSG_TIME_ADD, catom);
            getScheduler()->schedule(evt);
            reconf->nChildren++;
        }
    }
}

void Neighborhood::addNeighborToNextPlane()
{
    Cell3DPosition pos = catom->position.offsetZ(1);
    if (BlockCode::target->isInTarget(pos) && !catom->getInterface(pos)->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDNEXTPLANE_EVENT_ID, catom->blockId)) {
            getScheduler()->schedule(new AddNextPlane_event(getScheduler()->now()+MSG_TIME_ADD, catom));
        }
    }
}

void Neighborhood::addNeighborToPreviousPlane()
{
    Cell3DPosition pos = catom->position.offsetZ(-1);
    if (BlockCode::target->isInTarget(pos) && !catom->getInterface(pos)->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDPREVIOUSPLANE_EVENT_ID, catom->blockId)) {
            getScheduler()->schedule(new AddPreviousPlane_event(getScheduler()->now()+MSG_TIME_ADD, catom));
        }
    }
}

bool Neighborhood::isFirstCatomOfLine()
{
    if (!catom->getInterface(catom->position.offsetX(-1))->isConnected() &&
        !catom->getInterface(catom->position.offsetX(1))->isConnected())
        return true;
    return false;
}

bool Neighborhood::isFirstCatomOfPlane()
{
    if (isFirstCatomOfLine() &&
            !catom->getInterface(catom->position.offsetY(-1))->isConnected() &&
            !catom->getInterface(catom->position.offsetY(1))->isConnected())
        return true;
    return false;
}


void Neighborhood::addNeighbors()
{
    addNext();
    addPrevious();
    addLeft();
    addRight();
    addNextPlane();
}

void Neighborhood::addNextPlane() {
    if (reconf->isPlaneSeed() && reconf->canAddNextPlaneSeed()) {
        addNeighborToNextPlane();
    }
}

void Neighborhood::addLeft() {
    // Position exists and is free.
    if(BlockCode::target->isInTarget(catom->position.offsetX(-1)) &&
       !catom->getInterface(catom->position.offsetX(-1))->isConnected()) {
        // Check if can block previous line or not.
        if (!BlockCode::target->isInTarget(catom->position.offsetY(-1).offsetX(-1))
                || !catom->getInterface(catom->position.offsetY(-1))->isConnected()
                || reconf->canFillLeft) {

            // Do not block previous floor
            if (reconf->floor == 0 || reconf->canFillWest() || reconf->parentPlaneFinished) {
                if (syncNext->needSyncToLeft())
                    syncNext->sync();
                else
                    addNeighborToLeft();
            }
        }
    }
}

void Neighborhood::addRight() {
    // Position exists and is free.
    if(BlockCode::target->isInTarget(catom->position.offsetX(1)) &&
       !catom->getInterface(catom->position.offsetX(1))->isConnected()) {
        // Check if can block previous line or not.
        if (!BlockCode::target->isInTarget(catom->position.offsetY(1).offsetX(1))
            || !catom->getInterface(catom->position.offsetY(1))->isConnected()
            || reconf->canFillRight) {

            // Do not block previous floor
            if (reconf->floor == 0 || reconf->canFillEast() || reconf->parentPlaneFinished) {
                if (syncPrevious->needSyncToRight()) {
                    syncPrevious->sync();
                }
                else {
                    addNeighborToRight();
                }
            }
        }
    }
}

void Neighborhood::addNext() {
    if (BlockCode::target->isInTarget(catom->position.offsetY(1)) &&
            reconf->isSeedNext() && ((reconf->confirmNorthLeft && reconf->confirmNorthRight) || reconf->floor == 0 || reconf->parentPlaneFinished) && !syncNext->needSyncToRight()) {
        addNextLineNeighbor();
    }
}
void Neighborhood::addPrevious() {
    if (BlockCode::target->isInTarget(catom->position.offsetY(-1)) &&
        reconf->isSeedPrevious() && ((reconf->confirmSouthLeft && reconf->confirmSouthRight) || reconf->floor == 0 || reconf->parentPlaneFinished) && !syncPrevious->needSyncToLeft()) {
        addPreviousLineNeighbor();
    }
}

void Neighborhood::sendResponseMessageToAddLeft() {
    if (catom->getInterface(catom->position.offsetY(1))->isConnected() &&
            catom->getInterface(catom->position.offsetX(-1))->isConnected()) {
        CanFillLeftResponse_message *msg = new CanFillLeftResponse_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(catom->position.offsetY(1))));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendResponseMessageToAddRight() {
    if (catom->getInterface(catom->position.offsetX(1))->isConnected() &&
            catom->getInterface(catom->position.offsetY(-1))->isConnected()) {
        CanFillRightResponse_message *msg = new CanFillRightResponse_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(catom->position.offsetY(-1))));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::addAllNeighbors()
{
    for (int i = 0; i < 12; i++) {
        Cell3DPosition neighborGridPos = catom->position;
        int *neighborPosPointer = (catom->position[2]%2) ? NeighborRestriction::neighborDirectionsOdd[i] : NeighborRestriction::neighborDirectionsEven[i];

        neighborGridPos.pt[0] += neighborPosPointer[0];
        neighborGridPos.pt[1] += neighborPosPointer[1];
        neighborGridPos.pt[2] += neighborPosPointer[2];

        addNeighbor(neighborGridPos);
    }
}

bool Neighborhood::addFirstNeighbor()
{
    for (int i = 0; i < 12; i++) {
        Cell3DPosition neighborGridPos = catom->position;
        int *neighborPosPointer = (catom->position[2]%2) ? NeighborRestriction::neighborDirectionsOdd[i] : NeighborRestriction::neighborDirectionsEven[i];

        neighborGridPos.pt[0] += neighborPosPointer[0];
        neighborGridPos.pt[1] += neighborPosPointer[1];
        neighborGridPos.pt[2] += neighborPosPointer[2];

        if (addNeighbor(neighborGridPos))
            return true;
    }
    return false;
}

bool Neighborhood::addNeighbor(Cell3DPosition pos)
{
    Catoms3D::Catoms3DWorld *world = Catoms3D::Catoms3DWorld::getWorld();
    NeighborRestriction neighbors;
    if (world->lattice->isFree(pos) && BlockCode::target->isInTarget(pos)) {
        Color c = BlockCode::target->getTargetColor(pos);
        if (neighbors.isPositionBlockable(pos))
            world->addBlock(0, blockCodeBuilder, pos, c, 0, false);
        else if (neighbors.isPositionBlocked(pos)) {
            world->addBlock(0, blockCodeBuilder, pos, c, 0, false);
            numberBlockedModules++;
            cout << "number of blocked modules = " << numberBlockedModules << endl;
            cout << "---- ERROR ----\nPosition " << pos << " blocked" << endl;
            world->lattice->highlightCell(pos, RED);

            //std::this_thread::sleep_for(std::chrono::milliseconds(100000));
        }
        else {
            world->addBlock(0, blockCodeBuilder, pos, c, 0, false);
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        world->linkBlock(pos);
        return true;
    }
    return false;
}


void Neighborhood::checkDependencies() {
    if (catom->position[2]%2) {
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(2))))
            reconf->confirmNorthLeft = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(2).offsetX(1))))
            reconf->confirmNorthRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetX(-1))))
            reconf->confirmWestLeft = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(1).offsetX(-1))))
            reconf->confirmWestRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(-1).offsetX(1))))
            reconf->confirmSouthLeft = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(-1))))
            reconf->confirmSouthRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(1).offsetX(2))))
            reconf->confirmEastLeft = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetX(2))))
            reconf->confirmEastRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetX(-1).offsetY(-1))))
            reconf->canFillNextFloor = true;
    }
    else {
        if (!BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(1).offsetX(-1)))
            reconf->confirmNorthLeft = true;
        if (!BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(1)))
            reconf->confirmNorthRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetX(-2).offsetY(-1))))
            reconf->confirmWestLeft = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetX(-2))))
            reconf->confirmWestRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(-2))))
            reconf->confirmSouthLeft = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetY(-2).offsetX(-1))))
            reconf->confirmSouthRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetX(1))))
            reconf->confirmEastLeft = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetZ(-1).offsetX(1).offsetY(-1))))
            reconf->confirmEastRight = true;
        if (!(BlockCode::target->isInTarget(catom->position.offsetX(1).offsetY(1))))
            reconf->canFillNextFloor = true;
    }
}

void Neighborhood::sendMessageToNextPlaneNorthLeft() {
    if (catom->getInterface(5)->isConnected()) {
        NextPlaneConfirmationNorthLeft_message *msg = new NextPlaneConfirmationNorthLeft_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(5)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageToNextPlaneNorthRight() {
    if (catom->getInterface(4)->isConnected()) {
        NextPlaneConfirmationNorthRight_message *msg = new NextPlaneConfirmationNorthRight_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(4)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageToNextPlaneWestLeft() {
    if (catom->getInterface(2)->isConnected()) {
        NextPlaneConfirmationWestLeft_message *msg = new NextPlaneConfirmationWestLeft_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(2)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageToNextPlaneWestRight() {
    if (catom->getInterface(5)->isConnected()) {
        NextPlaneConfirmationWestRight_message *msg = new NextPlaneConfirmationWestRight_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(5)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageToNextPlaneSouthLeft() {
    if (catom->getInterface(3)->isConnected()) {
        NextPlaneConfirmationSouthLeft_message *msg = new NextPlaneConfirmationSouthLeft_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(3)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageToNextPlaneSouthRight() {
    if (catom->getInterface(2)->isConnected()) {
        NextPlaneConfirmationSouthRight_message *msg = new NextPlaneConfirmationSouthRight_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(2)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageToNextPlaneEastLeft() {
    if (catom->getInterface(4)->isConnected()) {
        NextPlaneConfirmationEastLeft_message *msg = new NextPlaneConfirmationEastLeft_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(4)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageToNextPlaneEastRight() {
    int nextPlaneConnector;
    if (catom->position[2]%2)
        nextPlaneConnector = 3;
    else
        nextPlaneConnector = 3;
    if (catom->getInterface(nextPlaneConnector)->isConnected()) {
        NextPlaneConfirmationEastRight_message *msg = new NextPlaneConfirmationEastRight_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(nextPlaneConnector)));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendMessageCanFillNextFloor() {
    int interfaceNumber;
    if (catom->position[2]%2)
        interfaceNumber = 1;
    else
        interfaceNumber = 7;

    if (catom->getInterface(interfaceNumber)->isConnected()) {
        CanFillNextFloor_message *msg = new CanFillNextFloor_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(interfaceNumber)));
        numberMessagesToAddBlock++;
    }
}
