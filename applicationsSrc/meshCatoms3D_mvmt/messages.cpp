/**
 * @file   messages.cpp
 * @author pthalamy <pthalamy@p3520-pthalamy-linux>
 * @date   Tue Jul 10 14:13:13 2018
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>
#include <sstream>

#include "messages.hpp"

#include "meshCatoms3DBlockCode_mvmt.hpp" //FIXME:

void Catoms3DMotionEngineMessage::handle(BaseSimulator::BlockCode* bc) {
    handle(*static_cast<MeshCatoms3DBlockCode*>(bc)->engine);
}

// END FIXME:

void FindPathMessage::handle(Catoms3DMotionEngine& engine) {   
    if (!engine.visited) { // Module has not been considered for path planning yet
        if (!engine.searchParent) engine.searchParent = destinationInterface;

        engine.visited = true;
        engine.posToLocate = destination;
        engine.flag[destinationInterface] = true;        
                        
        // Check if adjacent to target and Visit sub-tree otherwise
        if (BaseSimulator::getWorld()->lattice->
            cellsAreAdjacent(engine.catom.position, destination)) {
            short targetCon = engine.catom.getConnectorId(destination);
            // engine.console << "target " << destination << " is on connector "
            //             << targetCon << "\n";
            
            
            // Add self as the next hop of the path
            engine.path.clear();
            engine.addModuleToPath(engine.catom, engine.path, targetCon, targetCon);
            
            engine.bc.sendMessage(new FindPathFoundMessage(engine.path),
                               engine.searchParent, MSG_DELAY, 0);

            // engine.console << "Path to " << destination << " found" << "\n";
        } else {
            P2PNetworkInterface *unprocessedNeighbor =
                engine.getNextUnprocessedInterface();

            if (unprocessedNeighbor) {
                engine.bc.sendMessage(new FindPathMessage(destination),
                                unprocessedNeighbor, MSG_DELAY, 0);
            } else {
                engine.bc.sendMessage(new FindPathNotFoundMessage(),
                                engine.searchParent, MSG_DELAY, 0);
            }

        }
                     
    } else { // Module has already been considered in this phase
        engine.bc.sendMessage(new FindPathIgnoreMessage(),
                               destinationInterface, MSG_DELAY, 0);
    }

}

void FindPathFoundMessage::handle(Catoms3DMotionEngine& engine) {
    short catomDockingConnector =
        engine.catom.getDirection(destinationInterface);
    short pivotDockingConnector = static_cast<Catoms3DBlock*>
        (sourceInterface->hostBlock)->getDirection(sourceInterface); // so-so

    engine.flag[destinationInterface] = true;

    // engine.console << engine.catom.position << " received path: " << "\n";
    // for (auto p : path) OUTPUT << p << endl;
    
    if (engine.searchParent) {
        // Add self as the next hop of the path
        engine.path = path;
        if (!engine.addModuleToPath(engine.catom, engine.path,
                                    pivotDockingConnector, catomDockingConnector))
        {
            awaitKeyPressed();
            assert(false);
        }
        
        engine.bc.sendMessage(new FindPathFoundMessage(engine.path),
                           engine.searchParent, MSG_DELAY, 0);
        
        engine.visited = false;
        engine.catom.setColor(GREEN); // todo
    } else { // isTail, moving module        
        engine.path = this->path;
        if (!engine.computeNextRotation(engine.path)) {
            cout << "Could not compute feasible rotation plan to parent,"
                 << " add myself to path" << endl;
            engine.catom.setColor(RED);

            awaitKeyPressed();
            assert(false);
        } else {
            // engine.growing = true;
            engine.visited = true;
        }
    } 
}

void handleFindPathResponse(Catoms3DMotionEngine& engine,
                            P2PNetworkInterface* sender) {
    P2PNetworkInterface *unprocessedNeighbor =
        engine.getNextUnprocessedInterface();

    engine.flag[sender] = true;

    if (unprocessedNeighbor) {
        engine.bc.sendMessage(new FindPathMessage(engine.posToLocate),
                               unprocessedNeighbor, MSG_DELAY, 0);
    } else {
        if (engine.searchParent) {
            engine.bc.sendMessage(new FindPathNotFoundMessage(),
                                   engine.searchParent, MSG_DELAY, 0);
            engine.visited = false;
        } else {
            engine.catom.setColor(WHITE); //todo probleeeeem

            cerr << "growth failed" << endl;
            awaitKeyPressed();
            assert(false);
        } 
    }
}

void FindPathIgnoreMessage::handle(Catoms3DMotionEngine& engine) {
    handleFindPathResponse(engine, destinationInterface);
}

void FindPathNotFoundMessage::handle(Catoms3DMotionEngine& engine) {
    handleFindPathResponse(engine, destinationInterface);
}
