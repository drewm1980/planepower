#include "protobufBridge.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ProtobufBridge)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;

namespace OCL
{
  ProtobufBridge::ProtobufBridge(std::string name)
    : TaskContext(name)
  {
    ports()->addEventPort( "stateInputPort",_stateInputPort ).doc("x,y,z"
								  ",dx,dy,dz"
								  ",e11,e12,e13,e21,e22,e23,e31,e32,e33"
								  ",w1,w2,w3"
								  ",delta,ddelta,ur,up"); // We ONLY use the first 18 states from this;
    // mismatch in u has no effect on us.
                                                                                         
    ports()->addPort("portMHEFullStateVector", portMheFullStateVector).doc( "MHE: all states over the horizon." );
    ports()->addPort("portMHEFullControlVector", portMheFullControlVector).doc( "MHE: all controls over the horizon." );
                
    ports()->addPort("portMPCFullStateVector", portMpcFullStateVector).doc( "MPC: all states over the horizon." );
    ports()->addPort("portMPCFullControlVector", portMpcFullControlVector).doc( "MPC: all controls over the horizon." );
  }

  ProtobufBridge::~ProtobufBridge()
  {
  }

  bool  ProtobufBridge::configureHook()
  {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    X.resize(NSTATES,0.0);
                
    mheFullStateVector.resize(   (NHORIZON + 1) * NSTATES,   0.0);
    mheFullControlVector.resize(  NHORIZON      * NCONTROLS, 0.0);
                
    mpcFullStateVector.resize(   (NHORIZON + 1) * NSTATES,   0.0);
    mpcFullControlVector.resize(  NHORIZON      * NCONTROLS, 0.0);

    mc.clear_css();
    for (int k=0; k<NHORIZON+1; k++) mc.add_css();

    context = new zmq::context_t(1);

    return true;
  }

  bool  ProtobufBridge::startHook()
  {
    socket = new zmq::socket_t(*context,ZMQ_PUB);
    socket->bind("tcp://*:5563");

    return true;
  }

  void ProtobufBridge::toCarouselState(const StateVector *state, const ControlVector *control,
				       double transparency, kite::CarouselState *cs)
  {
    kite::Xyz *xyz = cs->mutable_kitexyz();
    kite::Dcm *dcm = cs->mutable_kitedcm();
    xyz->set_x(state->x);
    xyz->set_y(state->y);
    xyz->set_z(state->z);
    dcm->set_r11(state->e11); // Note, there is an implicit transpose happening in here.
    dcm->set_r21(state->e12);
    dcm->set_r31(state->e13);
    dcm->set_r12(state->e21);
    dcm->set_r22(state->e22);
    dcm->set_r32(state->e23);
    dcm->set_r13(state->e31);
    dcm->set_r23(state->e32);
    dcm->set_r33(state->e33);
    cs->set_delta(state->delta);
    cs->set_kitetransparency(transparency);
    cs->set_linetransparency(transparency);

    cs->set_rarm(1.085);
    cs->set_zt(-0.05);
    cs->set_visspan(0.96);
  }

  void  ProtobufBridge::updateHook()
  {
    _stateInputPort.read( X );
                
    portMheFullStateVector.read( mheFullStateVector );
    portMheFullControlVector.read( mheFullControlVector );
                
    portMpcFullStateVector.read( mpcFullStateVector );
    portMpcFullControlVector.read( mpcFullControlVector );

    for (int k=0; k<NHORIZON+1; k++){
      StateVector * mheState = (StateVector*) &(mheFullStateVector[k*NSTATES]);
      //StateVector * mpcState = (StateVector*) &(mpcFullStateVector[k*NSTATES]);
      ControlVector * mheControl = (ControlVector*) &(mheFullControlVector[k*NCONTROLS]);
      //ControlVector * mpcControl = (ControlVector*) &(mpcFullControlVector[k*NCONTROLS]);
      double transparency = 0.2;
      if (k==NHORIZON){
        // set camera rotation
        mc.set_camerarotrads(mheState->delta);

        // set transparency
        transparency = 1.0;

        // write the "current state" field
        kite::CarouselState *cs = mc.mutable_currentstate();
        toCarouselState(mheState, mheControl, transparency, cs);
      }
      kite::CarouselState *cs = mc.mutable_css(k);
      toCarouselState(mheState, mheControl, transparency, cs);
    }

    if (!mc.SerializeToString(&X_serialized)) {
      cerr << "Failed to serialize mc." << endl;
      return;
    }
    s_sendmore(*socket, "multi-carousel");
    s_send(*socket, X_serialized);
  }

  void  ProtobufBridge::stopHook()
  {
    delete socket;
  }

  void  ProtobufBridge::cleanUpHook()
  {
    delete context;
    google::protobuf::ShutdownProtobufLibrary(); // optional
  }

}//namespace

