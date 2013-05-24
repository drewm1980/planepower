import numpy

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
        # compute expected measurements as fcn of x/u
        (N, ny) = mheRT.y.shape
        y_of_x = numpy.zeros( (N, ny) )
        for k in range(N):
            y_of_x[k,:] = mheRT.computeY(mheRT.x[k,:], mheRT.u[k,:])
        yN_of_xN = mheRT.computeYN(mheRT.x[-1,:])
        self._pbb.setMheExpectedMeas(DVector(y_of_x.flatten()),
                                     DVector(yN_of_xN))

    def setMpc(self,mpcRT):
        self._pbb.setMpc(DVector(mpcRT.x.flatten()),
                         DVector(mpcRT.u.flatten()),
                         DVector(mpcRT.x0.flatten()),
                         mpcRT.getKKT(),
                         mpcRT.getObjective(),
                         mpcRT.preparationTime,
                         mpcRT.feedbackTime)

    def setSimState(self, x, z, u, y, yn, outs):
        self._pbb.setSimState(DVector(x.flatten()),
                              DVector(z.flatten()),
                              DVector(u.flatten()),
                              DVector(y.flatten()),
                              DVector(yn.flatten()),
                              DVector(outs.flatten()))

    def sendMessage(self):
        self._pbb.sendMessage()
        return self._pbb.packedMsg
