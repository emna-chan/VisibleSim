/*
 *bbCycleBlockCode.h
 *
 *  Created on: 26 mars 2013
 *      Author: dom
 */

#ifndef MUSICPLAYERBLOCKCODE_H_
#define MUSICPLAYERBLOCKCODE_H_

#define SYNC_MSG_ID	9002
#define SCORE_MSG_ID	9001

#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"
#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"

#include "utils/color.h"
#include <vector>

class SynchroMessage;
class ScoreMessage;
class Note;
typedef std::shared_ptr<SynchroMessage> SynchroMessage_ptr;
typedef std::shared_ptr<ScoreMessage> ScoreMessage_ptr;

class MusicPlayerBlockCode : public BlinkyBlocks::BlinkyBlocksBlockCode {
    P2PNetworkInterface *block2Answer;
    bool received[1000];
    bool b[6];
    bool assigned;
    bool back;
    int rootInterface;
    int64_t delay;
    int idMessage;
    std::vector<Note> toPlay;

public:
    MusicPlayerBlockCode(BlinkyBlocks::BlinkyBlocksBlock *host);
    ~MusicPlayerBlockCode();

    void startup() override;
    void init() override;
    void processLocalEvent(EventPtr pev) override;
    Color getColor(Time time);
    std::vector<Note> Score();
    void sendClockToNeighbors (P2PNetworkInterface *except, int hop, Time clock, int id);
    void sendSongToNeighbors (P2PNetworkInterface *except, std::vector<Note> score);
    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new MusicPlayerBlockCode((BlinkyBlocks::BlinkyBlocksBlock*)host));
    }
};

class SynchroMessage : public Message {
public:
    int idSync;
    Time time;
    int nbhop;
    SynchroMessage(Time t, int hop, int id);
    unsigned int size() { return(17); }
    ~SynchroMessage();
};

class ScoreMessage : public Message {
public:
    std::vector<Note> score;
    ScoreMessage(std::vector<Note> song);
    ~ScoreMessage();
};

class Note {
public:
    float frequency;
    float startTime;
    int timeDiv;
    Note(float freq, float time, int div);
    ~Note();
};
#endif /* BBCYCLEBLOCKCODE_H_ */
