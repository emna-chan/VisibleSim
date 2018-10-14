/**
 * @file   meshAssemblyBlockCode.cpp
 * @author pthalamy <pthalamy@p3520-pthalamy-linux>
 * @date   Mon Oct  1 10:42:29 2018
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>
#include <set>

#include "catoms3DWorld.h"
#include "scheduler.h"
#include "events.h"
#include "trace.h"
#include "tDefs.h"

#include "teleportationEvents.h"
#include "rotation3DEvents.h"

#include "meshAssemblyBlockCode.hpp"

using namespace Catoms3D;
using namespace MeshCoating;

uint MeshAssemblyBlockCode::X_MAX;
uint MeshAssemblyBlockCode::Y_MAX;
uint MeshAssemblyBlockCode::Z_MAX;
constexpr std::array<Cell3DPosition, 6> MeshAssemblyBlockCode::incidentTipRelativePos;
constexpr Cell3DPosition MeshAssemblyBlockCode::meshSeedPosition;
bID id = 1;

MeshAssemblyBlockCode::MeshAssemblyBlockCode(Catoms3DBlock *host):
    Catoms3DBlockCode(host) {
    scheduler = getScheduler();
    world = BaseSimulator::getWorld();
    lattice = world->lattice;   
    catom = host;
    
    const Cell3DPosition& ub = lattice->getGridUpperBounds();
    // Round down mesh dimensions to multiple of B
    // TODO: Adapt to CSG
    X_MAX = ub[0] - (B - ub[0] % B); 
    Y_MAX = ub[1] - (B - ub[1] % B);
    Z_MAX = ub[2] - (B - ub[2] % B);

    ruleMatcher = new MeshRuleMatcher(X_MAX - meshSeedPosition[0],
                                      Y_MAX - meshSeedPosition[1],
                                      Z_MAX - meshSeedPosition[2], B);
}

MeshAssemblyBlockCode::~MeshAssemblyBlockCode() {
}

void MeshAssemblyBlockCode::onBlockSelected() {
// Debug:
    // (1) Print details of branch growth plan and previous round            
    cout << "Growth Plan: [ ";
    for (int i = 0; i < 6; i++)
        cout << catomsReqByBranch[i] << ", ";
    cout << " ]" << endl;

    cout << "Last Round: [ ";
    for (int i = 0; i < 6; i++)
        cout << fedCatomOnLastRound[i] << ", ";
    cout << " ]" << endl;

    cout << "Open Positions: [ ";
    for (int i = 0; i < 6; i++)
        cout << endl << "\t\t  "
             <<(openPositions[i] ? openPositions[i]->config_print() : "NULL") << ", ";
    cout << " ]" << endl;

    
    cout << "Targets for Entry Points: [ ";
    for (int i = 0; i < 4; i++)
        cout << endl << "\t\t  " << targetForEntryPoint[i].config_print() << ", ";
    cout << " ]" << endl;

    // catom->setColor(debugColorIndex++);

    cout << "branch: " << branch << endl;
    cout << "coordinatorPos: " << coordinatorPos << endl;
}

void MeshAssemblyBlockCode::startup() {
    stringstream info;
    info << "Starting ";

    // Do stuff
    if (ruleMatcher->isTileRoot(norm(catom->position))) {
        // Switch role
        role = Coordinator;
        coordinatorPos = catom->position;

        // Make incoming vertical branch tips appear already in place if at ground level
        if (catom->position[2] == meshSeedPosition[2]) {
            for (int i = 0; i < XBranch; i++)
                world->addBlock(++id, buildNewBlockCode,
                                catom->position + incidentTipRelativePos[i], ORANGE);
        }
        
        // Determine how many branches need to grow from here
        // and initialize growth data structures
        catomsReqByBranch[ZBranch] = ruleMatcher->
            shouldGrowZBranch(norm(catom->position)) ? B - 1 : -1;
        catomsReqByBranch[RevZBranch] = ruleMatcher->
            shouldGrowRevZBranch(norm(catom->position)) ? B - 1 : -1;
        catomsReqByBranch[LeftZBranch] = ruleMatcher->
            shouldGrowLeftZBranch(norm(catom->position)) ? B - 1 : -1;       
        catomsReqByBranch[RightZBranch] = ruleMatcher->
            shouldGrowRightZBranch(norm(catom->position)) ? B - 1 : -1;
        catomsReqByBranch[XBranch] = ruleMatcher->
            shouldGrowXBranch(norm(catom->position)) ? B - 1 : -1;        
        catomsReqByBranch[YBranch] = ruleMatcher->
            shouldGrowYBranch(norm(catom->position)) ? B - 1 : -1;

        // TODO: The spawning behavior and entry points might have to be different depending on whether or not a tile root is on a border
        if (catomsReqByBranch[ZBranch] > 0 and catomsReqByBranch[RevZBranch] > 0
            and catomsReqByBranch[LeftZBranch] > 0 and catomsReqByBranch[RightZBranch] > 0)
            hasToGrowFourDiagBranches = true;
        
        // Compute the corresponding list of cells to be filled
        updateOpenPositions();
        
        // Schedule next growth iteration (at t + MOVEMENT_DURATION (?) )
        getScheduler()->schedule(
            new InterruptionEvent(getScheduler()->now(),
                                  catom, IT_MODE_TILEROOT_ACTIVATION));
        console << "Scheduled Coordinator IT" << "\n";
    } else if (ruleMatcher->isVerticalBranchTip(norm(catom->position))) {            
        role = ActiveBeamTip; // nothing to be done, wait for tPos requests
        
        // Add z = B to ensure that level -1 catoms are handled properly
        short bi = ruleMatcher->determineBranchForPosition(
            norm(catom->position + Cell3DPosition(0,0,B)));
        VS_ASSERT_MSG(bi >= 0 and bi < N_BRANCHES, "cannot determine branch.");
        branch = static_cast<BranchIndex>(bi);
        coordinatorPos = catom->position - incidentTipRelativePos[branch];
    } else {
        role = FreeAgent;

        for (const Cell3DPosition& nPos : lattice->getActiveNeighborCells(catom->position)) {
            if (ruleMatcher->isVerticalBranchTip(norm(nPos))) {
                // Module is delegate coordinator
                P2PNetworkInterface* nItf = catom->getInterface(nPos);
                VS_ASSERT(nItf);
                sendMessage(new RequestTargetCellMessage(catom->position), nItf,
                            MSG_DELAY_MC, 0);
                return;
            }
        }
        
        VS_ASSERT_MSG(false, "meshAssembly: spawned module cannot be without a delegate coordinator in its vicinity.");        
    }
}

const Cell3DPosition
MeshAssemblyBlockCode::norm(const Cell3DPosition& pos) {    
    return pos - meshSeedPosition;
}

void MeshAssemblyBlockCode::processReceivedMessage(MessagePtr msg,
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

void MeshAssemblyBlockCode::processLocalEvent(EventPtr pev) {
    MessagePtr message;
    stringstream info;
	
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

        case EVENT_ROTATION3D_END:
            console << "Rotation to " << catom->position << " over" << "\n";
        case EVENT_TELEPORTATION_END: {            
            BranchIndex bi = ruleMatcher->
                getBranchIndexForNonRootPosition(norm(targetPosition));
                                
            if (catom->position == targetPosition) {
                //TODO: PLAN AGAIN TO AVOID COLLISIONS (e.g. last incoming branch tip)
                role = PassiveBeam;
                catom->setColor(BLUE);
                
                const Cell3DPosition& nextPos =
                    catom->position + ruleMatcher->getBranchUnitOffset(bi);

                // Coordinate to let the last arrived branch continue the construction
                if (ruleMatcher->isTileRoot(norm(nextPos))
                    and lattice->isFree(nextPos)
                    and catom->position[2] == meshSeedPosition[2]) {

                    if (incidentBranchesToRootAreComplete(nextPos)) {
#ifdef INTERACTIVE_MODE
                        lattice->unhighlightCell(nextPos);
                        cout << "Ready to insert tile root at " << nextPos << endl;
                        awaitKeyPressed();
#endif
                        addNewGroundTileRoot(nextPos);                        
                    } else {
#ifdef INTERACTIVE_MODE
                        cout << "Some branches are missing around " << nextPos << endl;
                        lattice->highlightCell(nextPos, YELLOW);
                        awaitKeyPressed();
#endif
                    }
                }
            } else {
                Cell3DPosition nextHop, nextPivotPos;
                if (lattice->cellsAreAdjacent(catom->position, targetPosition))
                    nextHop = targetPosition;
                else {
                    // Deduce next position

                    if (bi > 3)
                        nextHop = catom->position
                            + ruleMatcher->getBranchUnitOffset(bi);

                    else if (bi == ZBranch) {
                        // FIXME: STATIC RULES
                        if (coordinatorPos == meshSeedPosition) {
                            if (catom->position[0] < coordinatorPos[0])
                                nextHop = catom->position + Cell3DPosition(1,0,0);
                            else
                                nextHop = catom->position + ruleMatcher->getBranchUnitOffset(bi);
                        } else {
                            if (catom->position[2] == coordinatorPos[2])
                                nextHop = catom->position + Cell3DPosition(0,0,1);
                            else if (catom->position[0] < coordinatorPos[0])
                                nextHop = catom->position + Cell3DPosition(1,0,0);
                            else
                                nextHop = catom->position + ruleMatcher->getBranchUnitOffset(bi);                                
                        }

                    } else if (bi == RevZBranch) {
                        // FIXME: STATIC RULES                    
                        if (catom->position[2] < coordinatorPos[2])
                            nextHop = catom->position + Cell3DPosition(-1,-1,1);
                        else if (catom->position[0] == coordinatorPos[0])
                            nextHop = catom->position + Cell3DPosition(-1,0,0);
                        else
                            nextHop = catom->position + ruleMatcher->getBranchUnitOffset(bi);                                

                    } else if (bi == LeftZBranch) {
                        if (ruleMatcher->isOnXBorder(norm(coordinatorPos))
                            or ruleMatcher->isOnYOppBorder(norm(coordinatorPos))) {
                            nextHop = catom->position + ruleMatcher->getBranchUnitOffset(bi);
                        }
                    } else if (bi == RightZBranch) {
                        if (ruleMatcher->isOnYBorder(norm(coordinatorPos))
                            or ruleMatcher->isOnXOppBorder(norm(coordinatorPos))) {
                            nextHop = catom->position + ruleMatcher->getBranchUnitOffset(bi);
                        }

                    } else {
                        throw NotImplementedException("routing non XYZ branches");
                    }
                }

                try {
                    scheduler->schedule(
                        new Rotation3DStartEvent(getScheduler()->now(), catom, nextHop));
                } catch (const NoAvailableRotationPivotException& e_piv) {
                    cerr << e_piv.what();
                    cerr << "target position: " << nextHop << endl;
                    catom->setColor(RED);
                    VS_ASSERT(false);
                } catch (std::exception const& e) {
                    cerr << "exception: " << e.what() << endl;
                    VS_ASSERT(false);
                }
                
#ifdef INTERACTIVE_MODE
                awaitKeyPressed();
#endif
            }
        } break;            
            
        case EVENT_TAP: {
        } break;

        case EVENT_INTERRUPTION: {            
            std::shared_ptr<InterruptionEvent> itev =
                std::static_pointer_cast<InterruptionEvent>(pev);

            console << "IT Triggered, mode: " << itev->mode << "\n";
            
            switch(itev->mode) {

                case IT_MODE_TILEROOT_ACTIVATION: {
                    // if (counter++ > 2) return;
                    
                    int numInsertedCatoms = 0;
                    // Policy: Prioritize horizontal growth                    
                    for (int i = 5; i >= 0; i--) {
                        if (numInsertedCatoms < 2                            
                            
                            and (i != RevZBranch or
                                 (i == RevZBranch and (
                                     (not ruleMatcher->isOnXOppBorder(norm(catom->position))
                                      and !ruleMatcher->isOnYOppBorder(norm(catom->position)))
                                     or (ruleMatcher->isOnXOppBorder(norm(catom->position))
                                         and not fedCatomOnLastRound[XBranch])
                                     or (ruleMatcher->isOnYOppBorder(norm(catom->position))
                                         and not fedCatomOnLastRound[YBranch]))
                                     )
                                )

                            and (i != ZBranch or
                                 (i == ZBranch and catomsReqByBranch[RevZBranch] == -1))

                            and (i != LeftZBranch or
                                 (i == LeftZBranch and (
                                     (ruleMatcher->isOnXBorder(norm(catom->position))
                                      and catomsReqByBranch[ZBranch] <= 0)
                                     or (ruleMatcher->isOnYOppBorder(norm(catom->position))
                                         and catomsReqByBranch[RevZBranch] <= 0))
                                     )
                                )

                            and (i != RightZBranch or
                                 (i == RightZBranch and (
                                     (ruleMatcher->isOnYBorder(norm(catom->position))
                                      and catomsReqByBranch[ZBranch] == 0)
                                     or (ruleMatcher->isOnXOppBorder(norm(catom->position))
                                         and catomsReqByBranch[RevZBranch] <= 0))
                                     )
                                )
                            
                            and handleNewCatomInsertion((BranchIndex)i)) {
                            
                            numInsertedCatoms++;
                            fedCatomOnLastRound[i] = true;
                        } else {
                            fedCatomOnLastRound[i] = false;
                        }
                    }
                        
                    // OR we could have an array<BranchIndex, 4> floorNPosTargets, that
                    //  keeps track of where the most recently spawned catom on a given
                    //  neighbor position should go. Actually we are taking a decision
                    //  twice otherwise, with the current model.
                    // for (int i = 0; i < N_BRANCHES - 2; i++) fedCatomOnLastRound[i] = false;
                    VS_ASSERT_MSG(numInsertedCatoms <= 2, "more than two catoms inserted in single round");
                    
                    // cout << "[t-" << scheduler->now() << "] Round Summary: [ ";
                    // for (int i = 0; i < 6; i++) cout << fedCatomOnLastRound[i] << ", ";
                    // cout << " ]" << endl;
                    // cout << "(" << catom->blockId << ") Not spawning any catom this round" << endl;

                    getScheduler()->schedule(
                        new InterruptionEvent(getScheduler()->now() +
                                              (getRoundDuration() * (1 + (catom->position[0] * 0.05))),
                                              catom, IT_MODE_TILEROOT_ACTIVATION));
                } break;
            }            
        }
    }
}

void MeshAssemblyBlockCode::updateOpenPositions() {
    for (int i = 0; i < N_BRANCHES; i++) {        
        // [1..B], the number of already placed catoms + 1.
        // B means that branch is finished or should not be grown
        int multiplier = B - catomsReqByBranch[i];
        
        if (catomsReqByBranch[i] > 0 and openPositions[i]) {
            *openPositions[i] = Cell3DPosition(catom->position + multiplier * ruleMatcher->
                                               getBranchUnitOffset((BranchIndex)i));
            
        } else if (catomsReqByBranch[i] > 0 and not openPositions[i]) {
            openPositions[i] = new Cell3DPosition(catom->position + multiplier * ruleMatcher->
                                                  getBranchUnitOffset((BranchIndex)i));
        } else openPositions[i] = NULL;
    }
}

short MeshAssemblyBlockCode::getEntryPointDirectionForCell(const Cell3DPosition& pos) {
    short conId = catom->getAbsoluteDirection(pos);
    // cout << conId << endl;
    return conId > 7 ? conId - 8 : -1; // There are 8 others connectors/dirs numbered <<<< 
} 

short MeshAssemblyBlockCode::getEntryPointDirectionForBranch(BranchIndex bi) {
    switch(bi) {
        case ZBranch: return Dir8;
        case XBranch: return Dir9;
        case YBranch: return Dir11;
        case RevZBranch: return Dir10;
        case LeftZBranch: return Dir8;
        case RightZBranch: return Dir11;
        default: VS_ASSERT_MSG(false, "Invalid branch index"); 
    }

    return -1; // unreachable
}

Cell3DPosition MeshAssemblyBlockCode::getEntryPointForBranch(BranchIndex bi) {
    switch(bi) {
        case ZBranch:
            if (catomsReqByBranch[RevZBranch] == -1)
                return Cell3DPosition(0,0,-1);
            else
                throw NotImplementedException("getEP for non XYZ branches");
    
        case XBranch: return Cell3DPosition(2,0,-1);
        case YBranch: return Cell3DPosition(0,1,-1);
        case RevZBranch: return Cell3DPosition(1,1,-1);
        case LeftZBranch:
            if (ruleMatcher->isOnXBorder(norm(catom->position)))
                return Cell3DPosition(0,0,-1);
            else if (ruleMatcher->isOnYOppBorder(norm(catom->position)))
                return Cell3DPosition(1,1,-1);
            else VS_ASSERT(false);                
        case RightZBranch:
            if (ruleMatcher->isOnYBorder(norm(catom->position)))
                return Cell3DPosition(0,1,-1);
            else if (ruleMatcher->isOnXOppBorder(norm(catom->position)))
                return Cell3DPosition(0,1,-1);
            else VS_ASSERT(false);                
        default:
            cerr << "invalid branch index: " << bi << endl;;
            VS_ASSERT(false); 
    }

    return Cell3DPosition(-1,-1,-1); // unreachable
}

bool MeshAssemblyBlockCode::handleNewCatomInsertion(BranchIndex bi) {
    if (catomsReqByBranch[bi] > 0 
        // FIXME:
        and not (fedCatomOnLastRound[bi] or bi == ZBranch or bi == RevZBranch
                 or bi == LeftZBranch or bi == RightZBranch or bi == YBranch)
        ) {
        // FIXME: What if cell has module?
        const Cell3DPosition& entryPos =
            catom->position + getEntryPointForBranch(bi);
        VS_ASSERT(lattice->isFree(entryPos));

        // Introduce new catom
        console << "introduced catom at " << entryPos << "for " << bi << "\n";
        world->addBlock(++id, buildNewBlockCode, entryPos, ORANGE);
        
        // Set target position for introduced catom
        VS_ASSERT(openPositions[bi]);
        short epd = getEntryPointDirectionForCell(entryPos);
        // cout << entryPos << " -> epd: " << epd << endl;
        targetForEntryPoint[getEntryPointDirectionForCell(entryPos)] = *openPositions[bi];

        // Update open position for that branch
        catomsReqByBranch[bi]--;
        if (catomsReqByBranch[bi] == 0) openPositions[bi] = NULL;
        else *openPositions[bi] += ruleMatcher->getBranchUnitOffset(bi);
                            
        return true;
    }
    
    return false;
}

bool MeshAssemblyBlockCode::
incidentBranchesToRootAreComplete(const Cell3DPosition& pos) {
    VS_ASSERT(ruleMatcher->isInMesh(norm(pos))
              and ruleMatcher->isTileRoot(norm(pos)));

    for (int i = 0; i < N_BRANCHES; i++) {
        if (!isIncidentBranchTipInPlace(pos, static_cast<BranchIndex>(i))) return false;
    }

    return true;
}

bool MeshAssemblyBlockCode::
isIncidentBranchTipInPlace(const Cell3DPosition& trp, BranchIndex bi) {
    const Cell3DPosition& tipp = trp + incidentTipRelativePos[bi];
    return (not ruleMatcher->isInMesh(norm(tipp)))
        or lattice->cellHasBlock(tipp);
}

void MeshAssemblyBlockCode::addNewGroundTileRoot(const Cell3DPosition& pos) {
    // The tile root itself
    world->addBlock(++id, buildNewBlockCode, pos, RED);

    // Tips of incoming vertical branches to tile root
    // for (int i = 0; i < XBranch; i++)
    //     world->addBlock(++id, buildNewBlockCode, pos + incidentTipRelativePos[i], ORANGE);
}
