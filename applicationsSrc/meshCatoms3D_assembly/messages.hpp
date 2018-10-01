/**
 * @file   messages.hpp
 * @author pthalamy <pthalamy@p3520-pthalamy-linux>
 * @date   Tue Jul 10 13:47:20 2018
 * 
 * @brief  
 * 
 * 
 */


#ifndef MC3D_MESSAGES_H_
#define MC3D_MESSAGES_H_

#include "network.h"

#include "meshSpanningTree.hpp"

static const uint MSG_DELAY = 1;

using namespace MeshSpanningTree;

class DisassemblyTriggerMessage : public AbstractMeshSpanningTreeMessage {
public:
    DisassemblyTriggerMessage(const MeshSpanningTreeRuleMatcher& _ruleMatcher,
                              const bool isAck)
        : AbstractMeshSpanningTreeMessage(_ruleMatcher, isAck) {};
    virtual ~DisassemblyTriggerMessage() {};

    virtual void handle(BaseSimulator::BlockCode*);
    virtual Message* clone() { return new DisassemblyTriggerMessage(*this); }
    virtual string getName() { return "DisassemblyTrigger"; }
    virtual AbstractMeshSpanningTreeMessage*
    buildNewMeshSpanningTreeMessage(BaseSimulator::BlockCode& bc,
                                    const bool isAck) override;
};

class SubTreeScaffoldConstructionDoneMessage : public AbstractMeshSpanningTreeMessage {
public:
    SubTreeScaffoldConstructionDoneMessage(const MeshSpanningTreeRuleMatcher& _ruleMatcher,
                                           const bool isAck)
        : AbstractMeshSpanningTreeMessage(_ruleMatcher, true) {};
    virtual ~SubTreeScaffoldConstructionDoneMessage() {};

    virtual void handle(BaseSimulator::BlockCode*);
    virtual Message* clone() { return new SubTreeScaffoldConstructionDoneMessage(*this); }
    virtual string getName() { return "SubTreeScaffoldConstructionDone"; }
    virtual AbstractMeshSpanningTreeMessage*
    buildNewMeshSpanningTreeMessage(BaseSimulator::BlockCode& bc,
                                    const bool isAck) override;
};

#endif /* MC3D_MESSAGES_H_ */
