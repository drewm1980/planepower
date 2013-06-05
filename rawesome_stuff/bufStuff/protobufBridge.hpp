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
    void setMhe(const vector< double > &X,
                const vector< double > &U,
                const vector< double > &Y,
                const vector< double > &YN,
                const double kkt, const double obj,
                const double prepTime, const double fbTime);
    void setMpc(const vector< double > &X,
                const vector< double > &U,
                const vector< double > &X0,
                const vector< double > &Y,
                const vector< double > &YN,
                const double kkt, const double obj,
                const double prepTime, const double fbTime);
    void setSimState(const vector< double > &X,
                     const vector< double > &Z,
                     const vector< double > &U,
                     const vector< double > &YX,
                     const vector< double > &YU,
                     const vector< double > &outs);
    void setMheExpectedMeas(const vector< double > &YX_OF_X,
                            const vector< double > &YU_OF_U);
    void sendMessage();

    string packedMsg;
};

#endif // __PROTOBUFBRIDGE__
