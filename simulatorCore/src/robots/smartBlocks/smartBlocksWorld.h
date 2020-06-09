/*
 * smartBlocksWorld.h
 *
 *  Created on: 12 avril 2013
 *      Author: ben
 */

#ifndef SMARTBLOCKSWORLD_H_
#define SMARTBLOCKSWORLD_H_

#include "../../gui/openglViewer.h"
#include "../../base/world.h"
#include "../../math/vector3D.h"
#include "smartBlocksBlock.h"
#include "../../events/scheduler.h"

namespace SmartBlocks {

static const Vector3D defaultBlockSize{25.0, 25.0, 11.0};

class SmartBlocksWorld : public BaseSimulator::World {
protected:
    virtual ~SmartBlocksWorld();
public:
    inline static const int numPickingTextures = 5; /* The number of picking textures defined
                                                       for this type of catom,
                                                       used to deduce selected Block / face */

    SmartBlocksWorld(const Cell3DPosition &gridSize, const Vector3D &gridScale,
                     int argc, char *argv[]);

    GLuint idTextureFloor,idTextureDigits;

    static void deleteWorld();
    static SmartBlocksWorld* getWorld() {
        assert(world != NULL);
        return((SmartBlocksWorld*)world);
    }

    void printInfo() {
        cout << "I'm a SmartBlocksWorld" << endl;
    }

    virtual void addBlock(bID blockId, BlockCode *(*blockCodeBuildingFunction)(BuildingBlock*),
                          const Cell3DPosition &pos, const Color &col,
                          short orientation = 0, bool master = false) override;

    void linkBlock(const Cell3DPosition &pos) override;
    void loadTextures(const string &str) override;
    void glDraw() override;
    void glDrawId() override;
    void glDrawIdByMaterial() override;
    void glDrawBackground() override;
    void setSelectedFace(int n) override;
    void exportConfiguration() override;
};

inline void deleteWorld() {
    SmartBlocksWorld::deleteWorld();
}

inline SmartBlocksWorld* getWorld() { return(SmartBlocksWorld::getWorld()); }

} // SmartBlocks namespace

#endif /* SMARTBLOCKSWORLD_H_ */
