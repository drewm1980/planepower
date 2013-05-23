#ifndef __PROTOBUFBRIDGE__
#define __PROTOBUFBRIDGE__

#include <zmq.hpp>

#include "zhelpers.hpp"
#include "Carousel.pb.h"
#include "Carousel_structs.h"

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
    ProtobufBridge();
    ~ProtobufBridge();
    void sendMessage();

    void setNumbers(const vector< double > &mheFullStateVector, const vector< double > & mheFullControlVector,const vector< double >& mpcFullStateVector ,   const vector< double >& mpcFullControlVector,    const vector< double >& measurementsPast,    const vector< double >& measurementsCurrent,    const vector< double >& referenceTrajectory,     const vector< double >& controlsApplied,    const vector< double >& debugVec);

    
};

#endif // __PROTOBUFBRIDGE__
