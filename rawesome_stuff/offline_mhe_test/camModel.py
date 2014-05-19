# -*- coding: utf-8 -*-
"""
Created on Mon Jun 11 11:57:28 2012

@author: kurt
"""

import numpy as np
import casadi as C

def singleCamModel(p,R_c2b,RP,P,pos_marker_body):
    '''
    p = [x,y,z] position of the airplane
    R _c2b= rotation matrix of the plane
    RP = calibration matrix, giving the orientation and position of the camera
    P = calibration matrix, the projection matrix, which is intrinsic to the camera
    '''
    trans = C.SXMatrix(4,4)
    R_NED = np.eye(4)
    R_NED[1,1] = -1
    R_NED[2,2] = -1
    trans[0:3,3] = p
    trans[0,0] = 1.0;trans[1,1] = 1.0;trans[2,2] = 1.0;trans[3,3] = 1.0
    #Get the pixel values: multiplication from right to left do:
    # Rotate over the orientation of the BODY
    # Translate over the position of the BODY: You now have the position of the marker in the REFERENCE frame
    # Transform (rotate & translate) over the pose of the REFERENCE frame w.r.t. the CAMERA frame: You now have the position of the marker in the camera frame
    # Multiply with the camera projection matrix to get the pixel values
    R_b2c_4x4 = C.SXMatrix(4,4)
    R_b2c_4x4[0:3,0:3] = R_c2b.T
    R_b2c_4x4[3,3] = 1
    uvs = C.mul([P,RP,R_NED,trans,R_b2c_4x4,C.vertcat([pos_marker_body,1.0])])
    # Devide by s, which is the homogeneous scaling factor
    uv = uvs[:2]/uvs[2]
    return uv
    #Jh_cam.generateCode('camModelJacobian.c')

def fullCamModel(dae,conf):
    print "Using the updated full camera model"
	
	# Rotation matrix to convert from NWU to NED frame type
    R_nwu2ned = np.eye( 3 )
    R_nwu2ned[1, 1] = R_nwu2ned[2, 2] = -1.0
	
    PdatC1 = conf['PdatC1']
    PdatC2 = conf['PdatC2']
    RPC1 = conf['RPC1']
    RPC2 = conf['RPC2']
    pos_marker_body1 = C.mul(R_nwu2ned, conf['pos_marker_body1'])
    pos_marker_body2 = C.mul(R_nwu2ned, conf['pos_marker_body2'])
    pos_marker_body3 = C.mul(R_nwu2ned, conf['pos_marker_body3'])
    
    RpC1 = C.DMatrix.eye(4)
    RpC1[0:3,0:3] = RPC1[0:3,0:3].T
    RpC1[0:3,3] = C.mul(-RPC1[0:3,0:3].T,RPC1[0:3,3])
    RpC2 = C.DMatrix.eye(4);
    RpC2[0:3,0:3] = RPC2[0:3,0:3].T
    RpC2[0:3,3] = C.mul(-RPC2[0:3,0:3].T,RPC2[0:3,3])
    
    PC1 = C.SXMatrix(3,3)
    PC1[0,0] = PdatC1[0]
    PC1[1,1] = PdatC1[1]
    PC1[0,2] = PdatC1[2]
    PC1[1,2] = PdatC1[3]
    PC1[2,2] = 1.0
    PC2 = C.SXMatrix(3,3)
    PC2[0,0] = PdatC2[0]
    PC2[1,1] = PdatC2[1]
    PC2[0,2] = PdatC2[2]
    PC2[1,2] = PdatC2[3]
    PC2[2,2] = 1.0
    p = C.vertcat([dae['x'],dae['y'],dae['z']])
    R = C.veccat( [dae[n] for n in ['e11', 'e12', 'e13',
                                    'e21', 'e22', 'e23',
                                    'e31', 'e32', 'e33']]
                      ).reshape((3,3))
                      
    uv_all = C.vertcat([C.vec(singleCamModel(p,R,RpC1[0:3,:],PC1,pos_marker_body1)) ,\
                        C.vec(singleCamModel(p,R,RpC1[0:3,:],PC1,pos_marker_body2)) ,\
                        C.vec(singleCamModel(p,R,RpC1[0:3,:],PC1,pos_marker_body3)) ,\
                        C.vec(singleCamModel(p,R,RpC2[0:3,:],PC2,pos_marker_body1)) ,\
                        C.vec(singleCamModel(p,R,RpC2[0:3,:],PC2,pos_marker_body2)) ,\
                        C.vec(singleCamModel(p,R,RpC2[0:3,:],PC2,pos_marker_body3))])
    return uv_all

