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
        N = mheRT.ocp.N
        nyx = mheRT.ocp.yx.numel()
        nyu = mheRT.ocp.yu.numel()
        yx_of_x = numpy.zeros( (N+1, nyx) )
        yu_of_u = numpy.zeros( (N, nyu) )
        for k in range(N+1):
            yx_of_x[k,:] = mheRT.computeYX(mheRT.x[k,:])
        for k in range(N):
            yu_of_u[k,:] = mheRT.computeYU(mheRT.u[k,:])
        self._pbb.setMheExpectedMeas(DVector(yx_of_x.flatten()),
                                     DVector(yu_of_u.flatten()))

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
