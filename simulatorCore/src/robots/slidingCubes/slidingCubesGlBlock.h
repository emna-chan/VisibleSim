/*
 * slidingCubesGlBlock.h
 *
 *  Created on: 12 janvier 2014
 *      Author: Benoît
 */

#ifndef SLIDINGCUBESGLBLOCK_H_
#define SLIDINGCUBESGLBLOCK_H_
#include <string>
#include <sstream>
#include "gui/objLoader.h"
#include "base/glBlock.h"

namespace SlidingCubes {
class SlidingCubesGlBlock:public GlBlock {
protected :
    int nextId = 0;
    int prevId = 0;
public :
    string popupString;

    SlidingCubesGlBlock(bID id) : GlBlock(id) {};
    virtual ~SlidingCubesGlBlock() {};

    virtual void setPrevNext(int p,int n);
    virtual string getPopupInfo() override;

    void glDraw(ObjLoader::ObjLoader *ptrObj) override;
};
}
#endif /* SLIDINGCUBESGLBLOCK_H_ */
