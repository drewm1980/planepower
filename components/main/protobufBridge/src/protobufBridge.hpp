#ifndef __PROTOBUFBRIDGE__
#define __PROTOBUFBRIDGE__

#include <rtt/TaskContext.hpp>
#include <rtt/Logger.hpp>
#include <rtt/Property.hpp>
#include <rtt/Attribute.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/OperationCaller.hpp>
#include <rtt/Operation.hpp>
#include <rtt/Port.hpp>

#include <ocl/OCL.hpp>

#include <fstream>
using std::ifstream;

using namespace std;
using namespace RTT;
using namespace BFL;
using namespace Orocos;
using namespace KDL;

#include <zmq.hpp>

// These files are in the "rawesome" git repo maintained by Greg Horn
#include "zhelpers.hpp"
#include "kite.pb.h"

#include "carousel_types.h"

// Make those two guys as properties
#define NSTATES         22
#define NCONTROLS       3
#define NHORIZON        10

namespace OCL
{
  class ProtobufBridge
    : public TaskContext
  {
  protected:
    InputPort< vector< double > > _stateInputPort;
    vector< double >              X;
                        
    InputPort< vector< double > > portMheFullStateVector;
    InputPort< vector< double > > portMheFullControlVector;
    vector< double > mheFullStateVector;
    vector< double > mheFullControlVector;
                        
    InputPort< vector< double > > portMpcFullStateVector;
    InputPort< vector< double > > portMpcFullControlVector;
    vector< double > mpcFullStateVector;
    vector< double > mpcFullControlVector;
                        
  private:
    kite::MultiCarousel mc;

    zmq::context_t *context;
    zmq::socket_t *socket;

    string X_serialized;

    void toCarouselState(const StateVector *state, const ControlVector *control,
			 double transparency, kite::CarouselState *cs);

  public:
    ProtobufBridge(std::string name);
    ~ProtobufBridge();
    bool configureHook();
    bool startHook();
    void updateHook();
    void stopHook();
    void cleanUpHook();
  };
}
#endif // __PROTOBUFBRIDGE__
