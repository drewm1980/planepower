#ifndef __PROTOBUFBRIDGE__
#define __PROTOBUFBRIDGE__

#include <zmq.hpp>

#include "zhelpers.hpp"
#include "../autogen/Carousel.pb.h"
#include "../autogen/Carousel_structs.h"

using namespace std;

class ProtobufBridge
{
protected:
    vector< double > mheFullStateVector;
    vector< double > mheFullControlVector;

    vector< double > mpcFullStateVector;
    vector< double > mpcFullControlVector;

    vector< double > measurementsPast;

    vector< double > measurementsCurrent;

    vector< double > referenceTrajectory;

    vector< double > controlsApplied;

    vector< double > debugVec;

private:
    Carousel::MheMpcHorizons mmh;

    zmq::context_t *context;
    zmq::socket_t *socket;

    string X_serialized;

    void toDae(Carousel::Dae * dae, const DifferentialStates * x, const Controls * u);

public:
    ProtobufBridge(string name);
    ~ProtobufBridge();
    bool configureHook();
    bool startHook();
    void updateHook();
    void stopHook();
    void cleanUpHook();
};

#endif // __PROTOBUFBRIDGE__
