#include "protobufBridge.hpp"

#include "protoConverters.h"
#include "Carousel_dimensions.h"

ProtobufBridge::ProtobufBridge() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    debugVec.resize(10, 0.0);

//    referenceTrajectory.resize( (NUM_MPC_HORIZON + 1) * (NUM_DIFFSTATES + NUM_CONTROLS), 0.0);

    mmh.clear_mhe();
    mmh.clear_mpc();
    mmh.clear_messages();
//    mmh.clear_referencetrajectory();
    // setup MHE
    for (int k = 0; k < NUM_MHE_HORIZON+1; k++)
        mmh.mutable_mhe()->add_x();
    for (int k = 0; k < NUM_MHE_HORIZON; k++) {
        mmh.mutable_mhe()->add_u();
        mmh.mutable_mhe()->add_y();
    }
    // setup MPC
    for (int k = 0; k < NUM_MPC_HORIZON+1; k++)
        mmh.mutable_mpc()->add_x();
    for (int k = 0; k < NUM_MPC_HORIZON; k++)
        mmh.mutable_mpc()->add_u();
//    for (int k=0; k<NUM_MPC_HORIZON+1; k++){
//        mmh.add_referencetrajectory();
//    }

    context = new zmq::context_t(1);
    socket = new zmq::socket_t(*context, ZMQ_PUB);
    socket->bind("tcp://*:5563");
}

ProtobufBridge::~ProtobufBridge() {
    delete socket;
    delete context;
    google::protobuf::ShutdownProtobufLibrary();  // optional
}

void ProtobufBridge::setMhe(const vector< double > &X,
                            const vector< double > &U,
                            const vector< double > &Y,
                            const vector< double > &YN,
                            const double kkt, const double obj,
                            const double prepTime, const double fbTime) {
    assert(X.size() == (NUM_MHE_HORIZON + 1) * NUM_DIFFSTATES);
    assert(U.size() ==  NUM_MHE_HORIZON * NUM_CONTROLS);

    assert(Y.size() == NUM_MHE_HORIZON * NUM_MEASUREMENTS);
    assert(YN.size() == NUM_MEASUREMENTS_END);

    Carousel::Mhe * mhe = mmh.mutable_mhe();

    // set x
    for (int k = 0; k < NUM_MHE_HORIZON+1; k++) {
        DifferentialStates * x = (DifferentialStates*) &(X[k*NUM_DIFFSTATES]);
        fromDifferentialStates(mhe->mutable_x(k), x);
        if (k == NUM_MHE_HORIZON)
            fromDifferentialStates(mmh.mutable_mhexn(), x);
    }

    // set u
    for (int k = 0; k < NUM_MHE_HORIZON; k++)
        fromControls(mhe->mutable_u(k), (Controls*) &(U[k*NUM_CONTROLS]));

    // set y
    for (int k = 0; k < NUM_MHE_HORIZON; k++)
        fromMeasurements(mhe->mutable_y(k), (Measurements*) &(Y[k*NUM_MEASUREMENTS]));

    // set yN
    fromMeasurementsEnd(mhe->mutable_yn(), (MeasurementsEnd*) &(YN[0]));

    // set stats
    mhe->set_kkt(kkt);
    mhe->set_objective(obj);
    mhe->set_preptime(prepTime);
    mhe->set_fbtime(fbTime);
}

void ProtobufBridge::setMpc(const vector< double > &X,
                            const vector< double > &U,
                            const vector< double > &X0,
                            const double kkt, const double obj,
                            const double prepTime, const double fbTime) {
    assert(X.size() == (NUM_MPC_HORIZON + 1) * NUM_DIFFSTATES);
    assert(U.size() ==  NUM_MPC_HORIZON * NUM_CONTROLS);
    assert(X0.size() ==  NUM_DIFFSTATES);

    Carousel::Mpc * mpc = mmh.mutable_mpc();

    // set x0
    fromDifferentialStates(mpc->mutable_x0(), (DifferentialStates*) &(X0[0]));

    // set x
    for (int k = 0; k < NUM_MPC_HORIZON+1; k++) {
        DifferentialStates * x = (DifferentialStates*) &(X[k*NUM_DIFFSTATES]);
        fromDifferentialStates(mpc->mutable_x(k), x);
    }

    // set u
    for (int k = 0; k < NUM_MPC_HORIZON; k++) {
        Controls * u = (Controls*) &(U[k*NUM_CONTROLS]);
        fromControls(mpc->mutable_u(k), u);
        // set u0
        if (k == 0)
            fromControls(mmh.mutable_mpcu0(), u);
    }

    // set stats
    mpc->set_kkt(kkt);
    mpc->set_objective(obj);
    mpc->set_preptime(prepTime);
    mpc->set_fbtime(fbTime);

    // set reference trajectory
//    toDae(mmh.mutable_referencetrajectory(0), x, u); // off by one so that it lines up with mpc
//        for (int k=0; k<NUM_MPC_HORIZON; k++){
//            x = (DifferentialStates*) &(referenceTrajectory[k*(NUM_DIFFSTATES+NUM_CONTROLS)]);
//            u = (Controls*)   &(referenceTrajectory[k*(NUM_DIFFSTATES+NUM_CONTROLS)+NUM_DIFFSTATES]);
//            dae = mmh.mutable_referencetrajectory(k+1); // off by one so that it lines up with mpc
//            toDae(dae, x, u);
//        }
}

void ProtobufBridge::setSimState(const vector< double > &X,
                                 const vector< double > &U) {
    assert(X.size() ==  NUM_DIFFSTATES);
    assert(U.size() ==  NUM_CONTROLS);
    fromDifferentialStates(mmh.mutable_simx(), (DifferentialStates*) &(X[0]));
    fromControls(mmh.mutable_simu(), (Controls*) &(U[0]));
}


void  ProtobufBridge::sendMessage() {
    // set the debug vector
    mmh.mutable_debug()->set_d0(debugVec[0]);
    mmh.mutable_debug()->set_d1(debugVec[1]);
    mmh.mutable_debug()->set_d2(debugVec[2]);
    mmh.mutable_debug()->set_d3(debugVec[3]);
    mmh.mutable_debug()->set_d4(debugVec[4]);
    mmh.mutable_debug()->set_d5(debugVec[5]);
    mmh.mutable_debug()->set_d6(debugVec[6]);
    mmh.mutable_debug()->set_d7(debugVec[7]);
    mmh.mutable_debug()->set_d8(debugVec[8]);
    mmh.mutable_debug()->set_d9(debugVec[9]);

    // pack the string
    assert ( true == mmh.SerializeToString(&packedMsg) );

    // send
    s_sendmore(*socket, "mhe-mpc");
    s_send(*socket, packedMsg);
}
