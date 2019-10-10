/**
 * @file   coatingBlockCode.cpp
 * @author pthalamy <pthalamy@p3520-pthalamy-linux>
 * @date   Wed Oct  9 16:57:13 2019
 *
 * @brief
 *
 *
 */

#include <iostream>
#include <set>
#include <limits>
#include <algorithm>
#include <unistd.h>

#include "catoms3DWorld.h"
#include "scheduler.h"
#include "events.h"
#include "trace.h"
#include "tDefs.h"
#include "configExporter.h"

#include "teleportationEvents.h"
#include "rotation3DEvents.h"
#include "catoms3DMotionEngine.h"
#include "color.h"

#include "coatingBlockCode.hpp"

using namespace Catoms3D;
using namespace MeshCoating;

CoatingBlockCode::CoatingBlockCode(Catoms3DBlock *host):
    Catoms3DBlockCode(host) {

    if (host) {
        scheduler = getScheduler();
        world = BaseSimulator::getWorld();
        lattice = world->lattice;
        catom = host;
    }
}

CoatingBlockCode::~CoatingBlockCode() {
}

bool CoatingBlockCode::parseUserCommandLineArgument(int argc, char *argv[]) {
    /* Reading the command line */
    if ((argc > 0) && (argv[0][0] == '-')) {
        switch(argv[0][1]) {
            // case 'b':   {
            //     BUILDING_MODE = true;

            //     argc--;
            //     argv++;

            //     return true;
            // } break;
            case '-': {
                string varg = string(argv[0] + 2); // argv[0] without "--"

                if (varg == string("highlight")) HIGHLIGHT_SCAFFOLD = true;
                if (varg == string("csg")) HIGHLIGHT_CSG = true;
                else return false;

                argc--;
                argv++;

                return true;
            }
        }
    }

    return false;
}

void CoatingBlockCode::onAssertTriggered() {
    onBlockSelected();
    catom->setColor(BLACK);
}

void CoatingBlockCode::onBlockSelected() {

    // Debug:
    cout << endl << "--- PRINT MODULE " << *catom << "---" << endl;
    // cout<< "getResourcesForCoatingLayer(0): " << getResourcesForCoatingLayer(0) << endl;
}

void CoatingBlockCode::startup() {
    stringstream info;
    info << "Starting ";

    // Progress Display
    static unsigned int lastNbModules = 0;
    lastNbModules = max(lastNbModules, lattice->nbModules);
    int ts = round(getScheduler()->now() / getRoundDuration());
    if (ts > 0)
        cout << TermColor::BWhite << "\rCurrent Timestep:\t"
             << TermColor::BMagenta << ts
             << TermColor::BWhite << "\t---\tCurrent #Modules:\t"
             << TermColor::BMagenta << lastNbModules
             << TermColor::Reset << std::flush;

    if (scaffoldSeedPos == Cell3DPosition(-1,-1,-1)) {
        scaffoldSeedPos = determineScaffoldSeedPosition();
        // cerr << "scaffoldSeedPos: " << scaffoldSeedPos << endl;
        if (scaffoldSeedPos == Cell3DPosition(-1,-1,-1)) {
            highlightCSGScaffold(true);
            VS_ASSERT_MSG(scaffoldSeedPos != Cell3DPosition(-1,-1,-1), "Cannot find where to place scaffold seed tile. Please check CSG placement.");
        }
    }

    VS_ASSERT_MSG(target, "Target is null, aborting...");


    // Initialize scaffold bounds
    if (X_MAX == numeric_limits<int>::min()) {
        // Initialize Scaffold bounds
        const Cell3DPosition& gs = world->lattice->gridSize;

        Cell3DPosition pos;
        for (short iz = 0; iz < gs[2]; iz++) {
            for (short iy = - iz / 2; iy < gs[1] - iz / 2; iy++) {
                for (short ix = - iz / 2; ix < gs[0] - iz / 2; ix++) {
                    pos.set(ix, iy, iz);

                    if (isInCSG(pos)) {
                        if (pos[0] > X_MAX) X_MAX = pos[0];
                        else if (pos[0] < X_MIN) X_MIN = pos[0];

                        if (pos[1] > Y_MAX) Y_MAX = pos[1];
                        else if (pos[1] < Y_MIN) Y_MIN = pos[1];

                        if (pos[2] > Z_MAX) Z_MAX = pos[2];
                        else if (pos[2] < Z_MIN) Z_MIN = pos[2];
                    }
                }
            }
        }
    }

    rm = new CoatingRuleMatcher(X_MAX - sbSeedPos[0],
                                         Y_MAX - sbSeedPos[1],
                                         Z_MAX - sbSeedPos[2],
                                         X_MIN - sbSeedPos[0],
                                         Y_MIN - sbSeedPos[1],
                                         Z_MIN - sbSeedPos[2],
                                         B,
                                         [this](const Cell3DPosition& pos) {
                                             return isInsideCSGFn(pos);
                                         });

    initialized = true;
    // if (not sandboxInitialized)
    //     initializeSandbox();

    coordinatorPos =
        denorm(rm->getNearestTileRootPosition(norm(catom->position)));

    if (catom->position == spawnLoc) {
        P2PNetworkInterface* spawnPivotItf = catom->getInterface(spawnPivot);

        sendMessage(new CoaTrainRequest(), spawnPivotItf, MSG_DELAY_MC, 0);
    } else if (catom->position == scaffoldSeedPos) {
        spawnLoc = catom->position + Cell3DPosition(-1, -1, -1);
        closingCorner = scaffoldSeedPos + Cell3DPosition(-1, -1, 0);
        spawnPivot = scaffoldSeedPos + Cell3DPosition(0, 0, -2);

        lattice->highlightCell(spawnLoc, MAGENTA);
        lattice->highlightCell(closingCorner, CYAN);
        // lattice->highlightCell(spawnPivot, YELLOW);

        scheduler->schedule(new InterruptionEvent(getScheduler()->now() + getRoundDuration(),
                                                  catom, IT_MODULE_INSERTION));
    } else if (catom->position == spawnPivot) {
        catom->setColor(GREEN);
    }
}

void CoatingBlockCode::processReceivedMessage(MessagePtr msg,
                                                   P2PNetworkInterface *sender) {
    stringstream info;

    switch (msg->type) {
        // ALL MOVED TO HANDLEABLE MESSAGES
        default:
            cout << "Unknown Generic Message Type" << endl;
            assert(false);
            break;
    }

}

void CoatingBlockCode::processLocalEvent(EventPtr pev) {
    MessagePtr message;
    stringstream info;

    if (BUILDING_MODE
        and scheduler->getState() == Scheduler::State::PAUSED)
        return; // Disable any module or blockcode alteration

    switch (pev->eventType) {
        case EVENT_RECEIVE_MESSAGE: {
            message =
                (std::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message;

            if (message->isMessageHandleable()) {
                std::shared_ptr<HandleableMessage> hMsg =
                    (std::static_pointer_cast<HandleableMessage>(message));

                console << " received " << hMsg->getName() << " from "
                        << message->sourceInterface->hostBlock->blockId
                        << " at " << getScheduler()->now() << "\n";
                hMsg->handle(this);
            } else {
                P2PNetworkInterface * recv_interface = message->destinationInterface;

                // Handover to global message handler
                processReceivedMessage(message, recv_interface);
            }
        } break;

        case EVENT_ADD_NEIGHBOR: {
            // getScheduler()->trace(" ADD_NEIGHBOR ", catom->blockId, MAGENTA);
            break;
        }

        case EVENT_REMOVE_NEIGHBOR:
        case EVENT_PIVOT_ACTUATION_START:
        case EVENT_PIVOT_ACTUATION_END:
        case EVENT_ROTATION3D_START:
        case EVENT_TAP:
            break;

        case EVENT_ROTATION3D_END: {
            getScheduler()->trace(" ROTATION3D_END ", catom->blockId, MAGENTA);

            if (catom->position[2] < scaffoldSeedPos[2]) {
                if (lattice->isFree(catom->position + Cell3DPosition(0,0,1))) {
                    scheduleRotationTo(catom->position + Cell3DPosition(0,0,1));
                } else {
                    scheduleRotationTo(catom->position + Cell3DPosition(-1,-1,2));
                }
            } else {
                if (isInCoatingLayer(catom->position, 0)) {
                    catom->setColor(ORANGE);
                } else {
                    // Move around border until encountering a coating position

                    Cell3DPosition potentialOpenSlot;
                    if (hasOpenCoatingSlotNeighbor(0, potentialOpenSlot)) {
                        stringstream info;
                        info << " moving to open coating slot at " << potentialOpenSlot;
                        scheduler->trace(info.str(),catom->blockId,BLUE);

                        scheduleRotationTo(potentialOpenSlot);
                    } else {
                        vector<CWDir> cwdPosCustom = getCWDirectionsFrom(lastCWDir);
                        for (int i = 0; i < NumCWDirs; i++) {
                            const Cell3DPosition& cwdPos = CWDPos[cwdPosCustom[i]];
                            const Cell3DPosition& tPos = cwdPos + catom->position;

                            stringstream info;
                            info << " considering " << tPos
                                 << "(" << CWDir_to_string(cwdPosCustom[i]) << ")";
                            scheduler->trace(info.str(),catom->blockId,ORANGE);

                            if (Catoms3DMotionEngine::canMoveTo(catom, tPos)) {
                                lastCWDir = cwdPosCustom[i];

                                stringstream info;
                                info << " moving to next position around border: "
                                     << tPos << "(" << CWDir_to_string(lastCWDir) << ")";
                                scheduler->trace(info.str(),catom->blockId,WHITE);

                                scheduleRotationTo(tPos);
                                break;
                            }
                        }
                    }

                }
            }
        } break;

        case EVENT_INTERRUPTION: {
            std::shared_ptr<InterruptionEvent> itev =
                std::static_pointer_cast<InterruptionEvent>(pev);

            switch(itev->mode) {

                case IT_MODULE_INSERTION: {
                    if (lattice->isFree(spawnLoc))
                        world->addBlock(0, buildNewBlockCode, spawnLoc, YELLOW);

                    getScheduler()->schedule(
                        new InterruptionEvent(getScheduler()->now() +
                                              3 * (getRoundDuration()),
                                              catom, IT_MODULE_INSERTION));
                    break;
                }
            }
        }
    }
}


const Cell3DPosition
CoatingBlockCode::norm(const Cell3DPosition& pos) {
    return pos - sbSeedPos;
}

const Cell3DPosition
CoatingBlockCode::denorm(const Cell3DPosition& pos) {
    return pos + sbSeedPos;
}

bool CoatingBlockCode::isInsideCSGFn(const Cell3DPosition& pos) const {
    return isInCSG(denorm(pos));
}

void CoatingBlockCode::initializeSandbox() {
    highlightCSGScaffold();

    const Cell3DPosition& ulb = world->lattice->getGridUpperBounds();
    for (int x = sbSeedPos[0]; x < ulb[0]; x+=B) {
        for (int y = sbSeedPos[1]; y < ulb[1]; y+=B) {
            const Cell3DPosition& trPos = Cell3DPosition(x, y, sbSeedPos[2]);

            for (int i = 0; i < XBranch; i++) {
                Cell3DPosition pos = trPos;
                for (int j = 0; j < 3; j++) {
                    pos += rm->getIncidentTipRelativePos((BranchIndex)i);

                    if (lattice->isInGrid(pos)) {
                        world->addBlock(0, buildNewBlockCode, pos, GREY);
                    }
                }
            }

            if (trPos != sbSeedPos) { // or i != ZBranch)
                Cell3DPosition futureTRPos = trPos
                    + rm->getEntryPointRelativePos(Z_EPL);

                if (lattice->isInGrid(futureTRPos) and rm->isInCSGScaffold(norm(trPos)))
                    world->addBlock(0, buildNewBlockCode, futureTRPos, YELLOW);
            }
        }
    }
}

Cell3DPosition CoatingBlockCode::determineScaffoldSeedPosition() {
    // const Cell3DPosition& glb = world->lattice->getGridLowerBounds();
    const Cell3DPosition& ulb = world->lattice->getGridUpperBounds();

    Cell3DPosition pos;
    const Cell3DPosition &sctPos = sbSeedPos;

    // Scan base for lowest XY
    for (short ix = sctPos[0]; ix < ulb[0]; ix+=B) {
        for (short iy = sctPos[1]; iy < ulb[1]; iy+=B) {
            // MAYBE: Only diagonal needs to be considered
            if (ix == iy) {
                pos.set(ix, iy, sctPos[2]); // 3 is grid base over scaffold
                // lattice->highlightCell(pos, ORANGE);
                if (isInCSG(pos)) {
                    // lattice->highlightCell(pos, MAGENTA);
                    return pos;
                }
            }
        }
    }

    return Cell3DPosition(-1, -1, -1);
}

void CoatingBlockCode::highlightCSGScaffold(bool debug) {
    // target->highlight();

    // Initialize Target Object Preview
    const Cell3DPosition& gs = world->lattice->gridSize;
    Cell3DPosition pos;
    for (short iz = 0; iz < gs[2]; iz++) {
        for (short iy = - iz / 2; iy < gs[1] - iz / 2; iy++) {
            for (short ix = - iz / 2; ix < gs[0] - iz / 2; ix++) {
                pos.set(ix, iy, iz);

                if (debug) {
                    if (isInCSG(pos))
                        lattice->highlightCell(pos, RED);
                } else {

                    if (not rm->isInCSGScaffold(norm(pos))
                        // or (rm->isInCSGScaffold(norm(pos)) and not rm->isInCSGScaffold
                        //     (rm->getTileRootPositionForMeshPosition(norm(pos)))))
                        )
                        continue;

                    if (HIGHLIGHT_CSG and isInCSG(pos))
                        lattice->highlightCell(pos, RED);

                    if (HIGHLIGHT_SCAFFOLD and rm->isInCSGScaffold(norm(pos)))
                        lattice->highlightCell(pos, WHITE);
                }
            }
        }
    }
}

void
CoatingBlockCode::scheduleRotationTo(const Cell3DPosition& pos, Catoms3DBlock* pivot) {
    try {
        if (not pivot) pivot = Catoms3DMotionEngine::findMotionPivot(catom, pos);

        // OUTPUT << "mvmt: " << round((scheduler->now()) / getRoundDuration()) << "\t" << endl;
        // cout << "[t-" << scheduler->now() << "] rotation scheduled" << endl;
        scheduler->schedule(new Rotation3DStartEvent(getScheduler()->now(),
                                                     catom, pivot, pos,
                                                     RotationLinkType::HexaFace, false));
#ifdef INTERACTIVE_MODE
        awaitKeyPressed();
#endif
    } catch (const NoAvailableRotationPivotException& e_piv) {
        // cerr << e_piv.what();
        // cerr << "target position: " << pos << endl;
        cerr << "Couldn't find pivot for moving module at " << catom->position
             << " to " << pos << endl;
        catom->setColor(BLUE);
    } catch (std::exception const& e) {
        cerr << "exception: " << e.what() << endl;
    }

}

int CoatingBlockCode::getCoatingLayer(const Cell3DPosition& pos) const {
    return pos[2] - scaffoldSeedPos[2];
}


bool CoatingBlockCode::hasOpenCoatingSlotNeighbor(const int layer,
                                                  Cell3DPosition &openSlot) const {
    for (const Cell3DPosition& p : lattice->getFreeNeighborCells(catom->position)) {
        if (getCoatingLayer(p) == layer and isInCoatingLayer(p, layer)
            and (not (p == closingCorner)
                 or (p == closingCorner
                     and catom->position == (closingCorner + Cell3DPosition(-1, 0, 1))))) {
            openSlot = p;
            return true;
        }
    }

    return false;
}

bool CoatingBlockCode::isInCoatingLayer(const Cell3DPosition& pos, const int layer) const {
    if (layer != getCoatingLayer(pos))
        return false;

    for (const Cell3DPosition& p : lattice->getNeighborhood(pos)) {
        if (p[2] == pos[2] and isInCSG(p)) return true;
    }

    for (const Cell3DPosition& pRel : diagNeighbors) {
        if (isInCSG(pRel + pos)) return true;
    }

    return false;
}

const vector<CWDir> CoatingBlockCode::getCWDirectionsFrom(const CWDir cwd) const {
    vector<CWDir> cwDirs;

    // cout << CWDir_to_string(cwd) << endl;

    for (int cwi = cwd; cwi < NumCWDirs; cwi++)
        cwDirs.push_back((CWDir)cwi);
    for (int cwi = 0; cwi < cwd; cwi++)
        cwDirs.push_back((CWDir)cwi);

    stringstream info;
    for (int i = 0; i < NumCWDirs; i++)
        info << CWDir_to_string(cwDirs[i]) << " " ;
    scheduler->trace(info.str(),catom->blockId,BLUE);


    return cwDirs;
}

string CoatingBlockCode::CWDir_to_string(const CWDir d) const {
    switch(d) {
        case FrontLeft: return "FrontLeft";
        case Front: return "Front";
        case FrontRight: return "FrontRight";
        case Right: return "Right";
        case RearRight: return "RearRight";
        case Rear: return "Rear";
        case RearLeft: return "RearLeft";
        case Left: return "Left";
    }

    return "err";
}

int CoatingBlockCode::getResourcesForCoatingLayer(const int currentLayer) {
    // Virtually navigate around the object to count necessary resources
    int count = 0;
    Cell3DPosition currentPos = closingCorner;
    CWDir lastCwd = FrontLeft;

    do {
        // cout << currentPos << endl;
        // lattice->highlightCell(currentPos, RED);
        // usleep(1000000);
        // lattice->unhighlightCell(currentPos, RED);

        vector<CWDir> cwdPosCustom = getCWDirectionsFrom(lastCwd);
        for (int i = 0; i < NumCWDirs; i++) {
            const Cell3DPosition& cwdPos = CWDPos[cwdPosCustom[i]];
            // cout << "cwd:" << cwdPos << endl;
            const Cell3DPosition& vPos = cwdPos + currentPos;

            if (isInCoatingLayer(vPos, currentLayer)) {
                lastCwd = cwdPosCustom[i];
                currentPos = vPos;
                count++;
                break;
            }
        }
    } while (currentPos != closingCorner);


    return count;
}
