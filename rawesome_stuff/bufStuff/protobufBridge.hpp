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
    vector< double > debugVec;

private:
    Carousel::MheMpcHorizons mmh;

    zmq::context_t *context;
    zmq::socket_t *socket;

public:
    ProtobufBridge();
    ~ProtobufBridge();
    void setMhe(const vector< double > &mheX,
                const vector< double > &mheU,
                const vector< double > &mheY,
                const vector< double > &mheYN,
                const double kkt, const double obj,
                const double prepTime, const double fbTime);
    void setMpc(const vector< double > &mpcX,
                const vector< double > &mpcU,
                const vector< double > &mpcX0,
                const double kkt, const double obj,
                const double prepTime, const double fbTime);
    void setSimState(const vector< double > &X,
                     const vector< double > &U);
    void sendMessage();

    string packedMsg;
};

#endif // __PROTOBUFBRIDGE__
