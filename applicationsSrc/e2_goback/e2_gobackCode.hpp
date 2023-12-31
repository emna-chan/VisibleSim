/**
 * @file e2_GoBackCode.hpp
 * Generated by VisibleSim BlockCode Generator
 * https://services-stgi.pu-pm.univ-fcomte.fr/visiblesim/generator.php#
 * @author yourName
 * @date 2022-09-20
 **/

#ifndef E2_GoBackCode_H_
#define E2_GoBackCode_H_
#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"
#include "robots/blinkyBlocks/blinkyBlocksWorld.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"

static const int GO_MSG_ID = 1001;
static const int BACK_MSG_ID = 1002;

using namespace BlinkyBlocks;

class E2_GoBackCode : public BlinkyBlocksBlockCode {
private:
	BlinkyBlocksBlock *module = nullptr;
	bool isLeader=false;
	int myDistance;
	int myNbWaitedAnswer;
    P2PNetworkInterface *myParent;
public :
	E2_GoBackCode(BlinkyBlocksBlock *host);
	~E2_GoBackCode() {};

/**
  * This function is called on startup of the blockCode, it can be used to perform initial
  *  configuration of the host or this instance of the program.
  * @note this can be thought of as the main function of the module
  */
    void startup() override;

/**
  * @brief Message handler for the message 'Go'
  * @param _msg Pointer to the message received by the module, requires casting
  * @param sender Connector of the module that has received the message and that is connected to the sender
  */
   void myGoFunc(std::shared_ptr<Message>_msg,P2PNetworkInterface *sender);

/**
  * @brief Message handler for the message 'Back'
  * @param _msg Pointer to the message received by the module, requires casting
  * @param sender Connector of the module that has received the message and that is connected to the sender
  */
   void myBackFunc(std::shared_ptr<Message>_msg,P2PNetworkInterface *sender);

/**
  * @brief Provides the user with a pointer to the configuration file parser, which can be used to read additional user information from each block config. Has to be overridden in the child class.
  * @param config : pointer to the TiXmlElement representing the block configuration file, all information related to concerned block have already been parsed
  *
  */
    void parseUserBlockElements(TiXmlElement *config) override;

/*****************************************************************************/
/** needed to associate code to module                                      **/
	static BlockCode *buildNewBlockCode(BuildingBlock *host) {
	    return(new E2_GoBackCode((BlinkyBlocksBlock*)host));
	}
/*****************************************************************************/
};

#endif /* E2_GoBackCode_H_ */