#include "tetrisCode.hpp"

void TetrisCode::sendTmn5(bool reinit, bool blocked)
{
    TmnData data = TmnData(stage, update, rotation, position, color, nbReinit, nbFree);
    ReinitData rData = ReinitData(stage, nbReinit, tmn, movement);
    if (rotation == NORTH)
    {
        itf[northId] = topItf;
        itf[eastId] = rightItf;
        itf[southId] = bottomItf;
        itf[westId] = leftItf;
        northBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
    }
    else if (rotation == EAST)
    {
        itf[northId] = rightItf;
        itf[eastId] = bottomItf;
        itf[southId] = leftItf;
        itf[westId] = topItf;
        northBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
    }
    else if (rotation == SOUTH)
    {
        itf[northId] = bottomItf;
        itf[eastId] = leftItf;
        itf[southId] = topItf;
        itf[westId] = rightItf;
        northBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
    }
    else if (rotation == WEST)
    {
        itf[northId] = leftItf;
        itf[eastId] = topItf;
        itf[southId] = rightItf;
        itf[westId] = bottomItf;
        northBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
    }
    if (northBool)
    {
        if (position == 1)
        {
            data.position = 3;
        }
        if (position == 1)
        {
            P2PNetworkInterface *i = itf[northId];
            if (reinit && i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
                nbReinitBackMsg += 1;
            }
            else if (blocked && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
            }
            else
            {
                if (i != parent && i != nullptr and i->isConnected())
                {
                    sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                    nbTmnBackMsg += 1;
                }
            }
        }
    }
    else
    {
        P2PNetworkInterface *i = itf[northId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
    data.position = position;
    if (southBool)
    {
        if (position == 3)
        {
            data.position = 1;
        }
        if (position == 3)
        {
            P2PNetworkInterface *i = itf[southId];
            if (reinit && i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
                nbReinitBackMsg += 1;
            }
            else if (blocked && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
            }
            else
            {
                if (i != parent && i != nullptr and i->isConnected())
                {
                    sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                    nbTmnBackMsg += 1;
                }
            }
        }
    }
    else
    {
        P2PNetworkInterface *i = itf[southId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
    data.position = position;
    if (eastBool)
    {
        if (position == 1)
        {
            data.position = 4;
        }
        else if (position == 2)
        {
            data.position = 1;
        }
        if (position == 1 || position == 2)
        {
            P2PNetworkInterface *i = itf[eastId];
            if (reinit && i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
                nbReinitBackMsg += 1;
            }
            else if (blocked && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
            }
            else
            {
                if (i != parent && i != nullptr and i->isConnected())
                {
                    sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                    nbTmnBackMsg += 1;
                }
            }
        }
    }
    else
    {
        P2PNetworkInterface *i = itf[eastId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
    data.position = position;
    if (westBool)
    {
        if (position == 1)
        {
            data.position = 2;
        }
        else if (position == 4)
        {
            data.position = 1;
        }
        if (position == 1 || position == 4)
        {
            P2PNetworkInterface *i = itf[westId];
            if (reinit && i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
                nbReinitBackMsg += 1;
            }
            else if (blocked && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
            }
            else
            {
                if (i != parent && i != nullptr and i->isConnected())
                {
                    sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                    nbTmnBackMsg += 1;
                }
            }
        }
    }
    else
    {
        P2PNetworkInterface *i = itf[westId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 5 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new MessageOf<int>(BLOCKED_MSG_ID,stage), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 5 Message", new MessageOf<TmnData>(TMN5_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
}

void TetrisCode::myTmn5Func(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<TmnData> *msg = static_cast<MessageOf<TmnData> *>(_msg.get());
    TmnData msgData = *msg->getData();
    if (msgData.stage >= stage && update < msgData.nbupdate && (tmn != 5 || rotation != msgData.rotation || position != msgData.position || color != msgData.color))
    {
        stage = msgData.stage;
        tmn = 5;
        update = msgData.nbupdate;
        rotation = msgData.rotation;
        position = msgData.position;
        color = msgData.color;
        nbReinit = msgData.nbReinit;
        nbFree = msgData.nbFree;
        parent = sender;
        nbTmnBackMsg = 0;
        console << "recieved tmn " << tmn << " rot = " << rotation << " pos = " << position << "\n";
        module->setColor(Colors[color]);
        sendTmn5(false, false);
        if (nbTmnBackMsg == 0 && parent != nullptr && parent->isConnected())
        {
            TmnBackData data = TmnBackData(stage, update);
            sendMessage("Tmn Back Message Parent", new MessageOf<TmnBackData>(TMNBACK_MSG_ID, data), parent, 0, 0);
        }
    }
    else if (msgData.stage >= stage && update == msgData.nbupdate)
    {
        stage = msgData.stage;
        TmnBackData data = TmnBackData(stage, update);
        sendMessage("Tmn Back Message", new MessageOf<TmnBackData>(TMNBACK_MSG_ID, data), sender, 0, 0);
    }
}

void TetrisCode::myRestartTmn5Func(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<TmnData> *msg = static_cast<MessageOf<TmnData> *>(_msg.get());
    TmnData msgData = *msg->getData();
    if (msgData.stage >= stage && (roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE))
    {
        stage = msgData.stage;
        parent = nullptr;
        leaderBlockCode = this;
        tmn = 5;
        update = msgData.nbupdate;
        nbReinit = msgData.nbReinit;
        nbFree = msgData.nbFree;
        rotation = msgData.rotation;
        position = msgData.position;
        color = msgData.color;
        goingRight = msgData.goingR;
        goingLeft = msgData.goingL;
        turnCK = msgData.rotCW;
        turnCounterCK = msgData.rotCCW;
        module->setColor(Colors[color]);
        sendTmn5(false, false);
    }
    else if (msgData.stage >= stage)
    {
        stage = msgData.stage;
        P2PNetworkInterface *i = nullptr;
        if (sender == topItf)
        {
            i = bottomItf;
        }
        else if (sender == bottomItf)
        {
            i = topItf;
        }
        else if (sender == rightItf)
        {
            i = leftItf;
        }
        else if (sender == leftItf)
        {
            i = rightItf;
        }
        if (i != nullptr && i->isConnected())
        {
            sendMessage("Restart Tmn 5 Message", new MessageOf<TmnData>(START_TMN5_MSG_ID, msgData), i, 0, 0);
        }
    }
}

void TetrisCode::verifTmn5()
{
    if (movement == GO_LEFT || movement == GO_RIGHT || movement == DOWN)
    {
        verifications.clear();
        int rot1 = 0;
        int rot2 = 0;
        int rot3 = 0;
        int rot4 = 0;
        int dir = 0;
        P2PNetworkInterface *i = nullptr; //for direct verifications
        if (movement == DOWN)
        {
            rot1 = NORTH;
            rot2 = SOUTH;
            rot3 = EAST;
            rot4 = WEST;
            dir = SOUTH;
            i = bottomItf;
        }
        else if (movement == GO_RIGHT)
        {
            rot1 = WEST;
            rot2 = EAST;
            rot3 = NORTH;
            rot4 = SOUTH;
            dir = EAST;
            i = rightItf;
        }
        else if (movement == GO_LEFT)
        {
            rot1 = EAST;
            rot2 = WEST;
            rot3 = SOUTH;
            rot4 = NORTH;
            dir = WEST;
            i = leftItf;
        }
        if (rotation == rot1)
        {
            verifications.push_back(freeAnswer(stage, 2, dir));
            verifications.push_back(freeAnswer(stage, 4, dir));
            if (movement == GO_LEFT)
            {
                verifications.push_back(freeAnswer(stage, 1, WEST));
                nbFree += 1;
                isFreeData data = isFreeData(stage, nbFree, 1, WEST);
                sendVerifTmn5(false, data);
            }
            else
            {
                if (i != nullptr && i->isConnected())
                {
                    sendMessage("Direct Verification Message", new MessageOf<int>(FREEMSG_ID, stage), i, 0, 0);
                }
                else
                {
                    //the tetramino is blocked
                    blocked = true;
                    sendTmn5(false, true);
                    BlockedData data = BlockedData(stage, nbLinesReinit, 1);
                    if (leftItf != nullptr && leftItf->isConnected())
                    {
                        sendMessage("Counting blocked neighbors", new MessageOf<BlockedData>(COUNT_BCK_MSG_ID, data), leftItf, 0, 0);
                    }
                    if (rightItf != nullptr && rightItf->isConnected())
                    {
                        sendMessage("Counting blocked neighbors", new MessageOf<BlockedData>(COUNT_BCK_MSG_ID, data), rightItf, 0, 0);
                    }
                    nbTmn += 1;
                    NewTmnData data2 = NewTmnData(stage, nbTmn);
                    sendMessageToAllNeighbors("New Tetramino Message", new MessageOf<NewTmnData>(NEWTMNMSG_ID, data2), 0, 0, 0);
                }
            }
        }
        else if (rotation == rot2)
        {
            nbFree += 1;
            verifications.push_back(freeAnswer(stage, 2, dir));
            verifications.push_back(freeAnswer(stage, 3, dir));
            verifications.push_back(freeAnswer(stage, 4, dir));
            isFreeData data = isFreeData(stage, nbFree, 4, dir);
            sendVerifTmn5(false, data);
        }
        else if (rotation == rot3)
        {
            nbFree += 1;
            verifications.push_back(freeAnswer(stage, 4, dir));
            verifications.push_back(freeAnswer(stage, 3, dir));
            isFreeData data = isFreeData(stage, nbFree, 3, dir);
            sendVerifTmn5(false, data);
        }
        else if (rotation == rot4)
        {
            nbFree += 1;
            verifications.push_back(freeAnswer(stage, 2, dir));
            verifications.push_back(freeAnswer(stage, 3, dir));
            isFreeData data = isFreeData(stage, nbFree, 3, dir);
            sendVerifTmn5(false, data);
        }
    }
    else if (movement == ROT_CK || movement == ROT_COUNTER_CK)
    {
        farVerifications.clear();
        std::vector<int> dirs;
        farVerif v;
        if (movement == ROT_CK)
        {
            dirs.push_back(SOUTH);
            v = farVerif(stage, 0, dirs, rotation);
            farVerifications.push_back(v);
        }
        else if (movement == ROT_COUNTER_CK)
        {
            dirs.push_back(SOUTH);
            v = farVerif(stage, 0, dirs, rotation);
            farVerifications.push_back(v);
        }
        sendFarVerif(v, nullptr);
    }
}

void TetrisCode::sendVerifTmn5(bool answer, isFreeData data)
{
    P2PNetworkInterface *i = itf[westId];
    if (((position != 2 && position != 3) || !westBool) && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
    i = itf[eastId];
    if (((position != 4 && position != 3) || !eastBool) && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
    i = itf[northId];
    if ((position == 1 || !northBool) && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
    i = itf[southId];
    if ((position == 3 || !southBool) && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
}