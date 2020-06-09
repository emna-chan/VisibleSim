/**
 * @file   nodes2DWorld.h
 * @author pthalamy <pthalamy@p3520-pthalamy-linux>
 * @date   Wed Jun 19 13:59:04 2019
 *
 * @brief
 *
 *
 */

#ifndef NODES2DWORLD_H_
#define NODES2DWORLD_H_

#include <vector>

#include "base/buildingBlock.h"
#include "gui/openglViewer.h"
#include "base/world.h"
#include "math/vector3D.h"
#include "grid/cell3DPosition.h"
#include "robots/nodes2D/nodes2DBlock.h"
#include "gui/objLoader.h"
#include "utils/trace.h"

//!< \namespace Nodes2D
namespace Nodes2D {

static const Vector3D defaultBlockSize{10.0, 10.0, 10.0};

class Nodes2DMotionEngine;
class Nodes2DMotion;

/**
 * \class Nodes2DWorld nodes2DWorld.h
 */
class Nodes2DWorld : public BaseSimulator::World {
protected:
    ObjLoader::ObjLoader *objConnector = NULL;           //!< Object loader for a block
        Nodes2DMotionEngine *nodes2DMotionEngine;
    virtual ~Nodes2DWorld();
public:
    Nodes2DWorld(const Cell3DPosition &gridSize, const Vector3D &gridScale,
                  int argc, char *argv[]);

    static void deleteWorld();
    static Nodes2DWorld* getWorld() {
        assert(world != NULL);
        return((Nodes2DWorld*)world);
    }

    void printInfo() {
        OUTPUT << "I'm a Nodes2DWorld" << endl;
    }

    virtual Nodes2DBlock* getBlockById(int bId) override {
        return((Nodes2DBlock*)World::getBlockById(bId));
    }

    virtual void addBlock(bID blockId, BlockCodeBuilder bcb, const Cell3DPosition &pos, const Color &col,
                          short orientation, bool master) override;

    GLuint idTextureWall, idTextureDigits;

    /**
     * \brief Connects block on grid cell pos to its neighbor
     * \param pos : Position of the block to connect
     */
    void linkBlock(const Cell3DPosition &pos) override;

    void glDraw() override;
    void glDrawShadows() override;
    void glDrawId() override;
    void glDrawIdByMaterial() override;
    void glDrawBackground() override;
    void updateGlData(BuildingBlock *bb) override;
    void updateGlData(Nodes2DBlock*blc,const Color &color);
    void updateGlData(Nodes2DBlock*blc, bool visible);
    void updateGlData(Nodes2DBlock*blc, const Cell3DPosition &position);
    void updateGlData(Nodes2DBlock*blc, const Vector3D &position);
    void updateGlData(Nodes2DBlock*blc, const Matrix &mat);
    void setSelectedFace(int n) override;
    void exportConfiguration() override;

    virtual void disconnectBlock(BuildingBlock *block);

    void createPopupMenu(int ix, int iy) override;
    void menuChoice(int n) override;

    vector<Nodes2DMotion*>getAllMotionsForModule(Nodes2DBlock*nb);
/**
 * \brief load the background textures (internal)
 */
    void loadTextures(const string &str) override;
};

inline void deleteWorld() {
    Nodes2DWorld::deleteWorld();
}

inline Nodes2DWorld* getWorld() { return(Nodes2DWorld::getWorld()); }


} // Nodes2D namespace

#endif /* NODES2DWORLD_H_ */
