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
    for (int k = 0; k < NUM_MHE_HORIZON+1; k++) {
        mmh.mutable_mhe()->add_x();
        mmh.mutable_mhe()->add_yx();
    }
    for (int k = 0; k < NUM_MHE_HORIZON; k++) {
        mmh.mutable_mhe()->add_u();
        mmh.mutable_mhe()->add_yu();
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

    assert(Y.size() == (NUM_MEASUREMENTS_X + NUM_MEASUREMENTS_U) * NUM_MHE_HORIZON);
    assert(YN.size() == NUM_MEASUREMENTS_X);

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

    // set yx and yu
    for (int k = 0; k < NUM_MHE_HORIZON; k++) {
        fromMeasurementsX(mhe->mutable_yx(k),
                          (MeasurementsX*) &(Y[k*(NUM_MEASUREMENTS_X+NUM_MEASUREMENTS_U)]));
        fromMeasurementsU(mhe->mutable_yu(k),
                          (MeasurementsU*) &(Y[k*(NUM_MEASUREMENTS_X+NUM_MEASUREMENTS_U)+
                                               NUM_MEASUREMENTS_U]));
    }
    fromMeasurementsX(mhe->mutable_yx(NUM_MHE_HORIZON), (MeasurementsX*) &(YN[0]));

    // set stats
    mhe->set_kkt(kkt);
    mhe->set_objective(obj);
    mhe->set_preptime(prepTime);
    mhe->set_fbtime(fbTime);
}

void ProtobufBridge::setMheExpectedMeas(const vector< double > &YX_OF_X,
                                        const vector< double > &YU_OF_U) {
    assert(YX_OF_X.size() == NUM_MEASUREMENTS_X * (NUM_MHE_HORIZON + 1));
    assert(YU_OF_U.size() == NUM_MEASUREMENTS_U * NUM_MHE_HORIZON);

    Carousel::Mhe * mhe = mmh.mutable_mhe();
    if (0 == mhe->yx_of_x_size())
        for (int k = 0; k < NUM_MHE_HORIZON+1; k++)
            mhe->add_yx_of_x();
    if (0 == mhe->yu_of_u_size())
        for (int k = 0; k < NUM_MHE_HORIZON; k++)
            mhe->add_yu_of_u();

    // set yx_of_x
    for (int k = 0; k < NUM_MHE_HORIZON+1; k++)
        fromMeasurementsX(mhe->mutable_yx_of_x(k), (MeasurementsX*) &(YX_OF_X[k*NUM_MEASUREMENTS_X]));

    // set yy_of_u
    for (int k = 0; k < NUM_MHE_HORIZON; k++)
        fromMeasurementsU(mhe->mutable_yu_of_u(k), (MeasurementsU*) &(YU_OF_U[k*NUM_MEASUREMENTS_U]));

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
                                 const vector< double > &Z,
                                 const vector< double > &U,
                                 const vector< double > &YX,
                                 const vector< double > &YU,
                                 const vector< double > &outs) {
    assert(X.size() ==  NUM_DIFFSTATES);
    assert(Z.size() ==  NUM_ALGVARS);
    assert(U.size() ==  NUM_CONTROLS);
    assert(YX.size() ==  NUM_MEASUREMENTS_X);
    assert(YU.size() ==  NUM_MEASUREMENTS_U);
    assert(outs.size() ==  NUM_OUTPUTS);
    Carousel::Sim * sim = mmh.mutable_sim();
    fromDifferentialStates(sim->mutable_x(), (DifferentialStates*) &(X[0]));
    fromAlgebraicVars(sim->mutable_z(), (AlgebraicVars*) &(Z[0]));
    fromControls(sim->mutable_u(), (Controls*) &(U[0]));
    fromMeasurementsX(sim->mutable_yx(), (MeasurementsX*) &(YX[0]));
    fromMeasurementsU(sim->mutable_yu(), (MeasurementsU*) &(YU[0]));
    fromOutputs(sim->mutable_outs(), (Outputs*) &(outs[0]));
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
