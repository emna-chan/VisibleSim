#include "syncResponse.h"

void SyncResponse::response(SyncModel syncModel, DIRECTION direction, bool canSyncLine, bool toLeft) {
    Sync_response_message *msg = new Sync_response_message(syncModel, canSyncLine, toLeft);
    Cell3DPosition pos = catom->position;
    if (direction == DIRECTION_UP)
        pos = pos.addY(1);
    if (direction == DIRECTION_DOWN)
        pos = pos.addY(-1);
    if (direction == DIRECTION_RIGHT)
        pos = pos.addX(1);
    if (direction == DIRECTION_LEFT)
        pos = pos.addX(-1);
    getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + 1000, msg, catom->getInterface(pos)));
}

void SyncResponse::forwardResponse(shared_ptr<Sync_response_message> msg) {
    response(msg->syncModel, syncResponseModel->routes[msg->syncModel.requestCatomID].direction, msg->canSyncLine, msg->toLeft);
}
