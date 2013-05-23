#include "protobufBridge.hpp"

#include "protoConverters.h"
#include "Carousel_dimensions.h"

ProtobufBridge::ProtobufBridge()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    debugVec.resize(10, 0.0);

    mheFullStateVector.resize(   (NUM_MHE_HORIZON + 1) * NUM_DIFFSTATES, 0.0);
    mheFullControlVector.resize(  NUM_MHE_HORIZON      * NUM_CONTROLS, 0.0);

    mpcFullStateVector.resize(   (NUM_MPC_HORIZON + 1) * NUM_DIFFSTATES, 0.0);
    mpcFullControlVector.resize(  NUM_MPC_HORIZON      * NUM_CONTROLS, 0.0);

    measurementsPast.resize( NUM_MHE_HORIZON * NUM_MEASUREMENTS, 0.0);
    measurementsCurrent.resize(                NUM_MEASUREMENTS_END, 0.0);

    referenceTrajectory.resize( (NUM_MPC_HORIZON + 1) * (NUM_DIFFSTATES + NUM_CONTROLS), 0.0);

    controlsApplied.resize( NUM_CONTROLS );

    mmh.clear_mhehorizon();
    mmh.clear_mpchorizon();
    mmh.clear_measurementshorizon();
    mmh.clear_referencetrajectory();
    for (int k=0; k<NUM_MHE_HORIZON+1; k++)
        mmh.add_mhehorizon();
    for (int k=0; k<NUM_MHE_HORIZON; k++)
        mmh.add_measurementshorizon();
    for (int k=0; k<NUM_MPC_HORIZON+1; k++){
        mmh.add_mpchorizon();
        mmh.add_referencetrajectory();
    }

    context = new zmq::context_t(1);
    socket = new zmq::socket_t(*context,ZMQ_PUB);
    socket->bind("tcp://*:5563");
}

ProtobufBridge::~ProtobufBridge()
{
    delete socket;
    delete context;
    google::protobuf::ShutdownProtobufLibrary(); // optional
}

void  ProtobufBridge::sendMessage()
{
//        portMheFullStateVector.read( mheFullStateVector );
//        portMheFullControlVector.read( mheFullControlVector );
//
//        portMpcFullStateVector.read( mpcFullStateVector );
//        portMpcFullControlVector.read( mpcFullControlVector );
//
//        portMeasurementsPast.read( measurementsPast );
//        portMeasurementsCurrent.read( measurementsCurrent );
//
//        portReferenceTrajectory.read( referenceTrajectory );
//
//        portControlsApplied.read( controlsApplied );
//
//        portDebugVec.read( debugVec );

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

    // set the controls applied
    //mmh.mutable_controlsapplied()->set_urright(controlsApplied[0]);
    //mmh.mutable_controlsapplied()->set_urleft(controlsApplied[1]);
    //mmh.mutable_controlsapplied()->set_up(controlsApplied[2]);

    Carousel::Dae *dae;

    // write the "current state" field
    DifferentialStates * x = (DifferentialStates*) &(mheFullStateVector[NUM_MHE_HORIZON*NUM_DIFFSTATES]);
    Controls * u = (Controls*)   &(mpcFullControlVector[0]);
    toDae(mmh.mutable_currentstate(), x, u);

    // set reference trajectory
    toDae(mmh.mutable_referencetrajectory(0), x, u); // off by one so that it lines up with mpc
//        for (int k=0; k<NUM_MPC_HORIZON; k++){
//            x = (DifferentialStates*) &(referenceTrajectory[k*(NUM_DIFFSTATES+NUM_CONTROLS)]);
//            u = (Controls*)   &(referenceTrajectory[k*(NUM_DIFFSTATES+NUM_CONTROLS)+NUM_DIFFSTATES]);
//            dae = mmh.mutable_referencetrajectory(k+1); // off by one so that it lines up with mpc
//            toDae(dae, x, u);
//        }

    // set mhe horizon
    for (int k=0; k<NUM_MHE_HORIZON+1; k++){
        x = (DifferentialStates*) &(mheFullStateVector[k*NUM_DIFFSTATES]);
        u = (Controls*)   &(mheFullControlVector[k*NUM_CONTROLS]);
        dae = mmh.mutable_mhehorizon(k);
        if (k<NUM_MHE_HORIZON)
            toDae(dae, x, u);
        else
            toDae(dae, x, NULL);
    }

    // set mpc horizon
    for (int k=0; k<NUM_MPC_HORIZON+1; k++){
        x = (DifferentialStates*) &(mpcFullStateVector[k*NUM_DIFFSTATES]);
        u = (Controls*)   &(mpcFullControlVector[k*NUM_CONTROLS]);
        dae = mmh.mutable_mpchorizon(k);
        if (k<NUM_MPC_HORIZON )
            toDae(dae, x, u);
        else
            toDae(dae, x, NULL);
    }

    // set measurement horizon
    // 0 through (NUM_MHE_HORIZON-1) are measurementsPast, X and U
    for (int k=0; k<NUM_MHE_HORIZON; k++){
        fromMeasurements(mmh.mutable_measurementshorizon(k),
                         (Measurements*)&(measurementsPast[k*NUM_MEASUREMENTS]));
    }
    //// NUM_MHE_HORIZON is measurementsCurrent, X only
    //fromMeasurements((mmh.mutable_measurementshorizon(NUM_MHE_HORIZON))->mutable_measurementsend(),
    //                     (MeasurementsEnd*)&(measurementsCurrent[0]));

    // set current measurement
    fromMeasurementsEnd(mmh.mutable_measurementsend(),
                        (MeasurementsEnd*)&(measurementsCurrent[0]));
    // measurementsLastLatest uses measurementsPast[0], X and U
    //fromMeasurements(mmh.mutable_measurementslastlatest()->mutable_measurementsx(),
    //                     (Measurements*)&(measurementsPast[0*NUM_MEASUREMENTS*(NUM_MHE_HORIZON-1)]));
    //fromMeasurementsUVec(mmh.mutable_measurementslastlatest()->mutable_measurementsu(),
    //                     (MeasurementsUVec*)&(measurementsPast[0*NUM_MEASUREMENTS*(NUM_MHE_HORIZON-1)+NUM_MEASUREMENTS_END]));


    if (!mmh.SerializeToString(&X_serialized)) {
        cerr << "Failed to serialize mmh." << endl;
        return;
    }
    s_sendmore(*socket, "mhe-mpc-horizons");
    s_send(*socket, X_serialized);
}

void ProtobufBridge::toDae(Carousel::Dae * dae, const DifferentialStates * x, const Controls * u){
    fromDifferentialStates(dae->mutable_differentialstates(), x);
    if (u != NULL)
        fromControls(dae->mutable_controls(), u);
}

void ProtobufBridge::setNumbers(const vector< double > &mheFullStateVector_, const vector< double > & mheFullControlVector_,const vector< double >& mpcFullStateVector_ ,   const vector< double >& mpcFullControlVector_,    const vector< double >& measurementsPast_,    const vector< double >& measurementsCurrent_,    const vector< double >& referenceTrajectory_,     const vector< double >& controlsApplied_,    const vector< double >& debugVec_) {
  copy(mheFullStateVector_.begin(),mheFullStateVector_.end(),mheFullStateVector.begin());
  copy(mheFullControlVector_.begin(),mheFullControlVector_.end(),mheFullControlVector.begin());
  copy(mpcFullStateVector_.begin(),mpcFullStateVector_.end(),mpcFullStateVector.begin());
  copy(mpcFullControlVector_.begin(),mpcFullControlVector_.end(),mpcFullControlVector.begin());
  copy(measurementsPast_.begin(),measurementsPast_.end(),measurementsPast.begin());
  copy(measurementsCurrent_.begin(),measurementsCurrent_.end(),measurementsCurrent.begin());
  copy(referenceTrajectory_.begin(),referenceTrajectory_.end(),referenceTrajectory.begin());
  copy(controlsApplied_.begin(),controlsApplied_.end(),controlsApplied.begin());
  copy(debugVec_.begin(),debugVec_.end(),debugVec.begin());
}
