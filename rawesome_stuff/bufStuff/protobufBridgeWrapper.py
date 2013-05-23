import protobufBridge
from protobufBridge import DVector

class ProtobufBridge(object):
    def __init__(self):
        self._pbb = protobufBridge.ProtobufBridge()

    def setMhe(self,mheRT):
        self._pbb.setMhe(DVector(mheRT.x.flatten()),
                         DVector(mheRT.u.flatten()),
                         DVector(mheRT.y.flatten()),
                         DVector(mheRT.yN.flatten()),
                         mheRT.getKKT(),
                         mheRT.getObjective(),
                         mheRT.preparationTime,
                         mheRT.feedbackTime)

    def setMpc(self,mpcRT):
        self._pbb.setMpc(DVector(mpcRT.x.flatten()),
                         DVector(mpcRT.u.flatten()),
                         DVector(mpcRT.x0.flatten()),
                         mpcRT.getKKT(),
                         mpcRT.getObjective(),
                         mpcRT.preparationTime,
                         mpcRT.feedbackTime)

    def setSimState(self, x, u):
        self._pbb.setSimState(DVector(x.flatten()), DVector(u.flatten()))

    def sendMessage(self):
        self._pbb.sendMessage()
