#ifndef SIMPLE_TRAJECTORY_GENERATOR_HPP
#define SIMPLE_TRAJECTORY_GENERATOR_HPP

#include <rtt/TaskContext.hpp>
#include <rtt/RTT.hpp>
#include <rtt/Property.hpp>
#include <rtt/Port.hpp>
#include <rtt/Component.hpp>

#include <rtt/base/DataObjectInterface.hpp>

#include <rtt/os/TimeService.hpp>

// #define REFS_FILENAME "/home/kurt/planepower/matlab_acado_codegen_simulation/refs.dat"
// #define WEIGHTS_FILENAME "/home/kurt/planepower/matlab_acado_codegen_simulation/weights.dat"

//
// Used namespaces
//
using namespace RTT;
using namespace RTT::base;
using namespace RTT::os;
using namespace std;

namespace OCL
{

class SimpleTrajectoryGenerator
	: public TaskContext
{
public:
	SimpleTrajectoryGenerator(const std::string& name);

	virtual ~SimpleTrajectoryGenerator( )
	{}

	virtual bool configureHook( );

	virtual bool startHook( );

	virtual void updateHook( );

	virtual void stopHook( );

	virtual void cleanupHook( );

	virtual void errorHook( );

private:

	bool readDataFromFile(const char* fileName, vector< vector< double > >& data, unsigned numRows = 0, unsigned numCols = 0);

	void printMatrix(string name, vector< vector< double > > data);
	
	string referencesFileName;
	string weightsPFileName;
	
	unsigned N, NX, NU;
	
	vector< vector< double > > references;
	vector< vector< double > > weightsP;
	
	unsigned numOfRefs;
	unsigned numOfWeightsP;
	
	unsigned refCounter;
	
	vector< double > execReferences;
	vector< double > execWeightsP;
	
	OutputPort< vector< double > > portReferences;
	OutputPort< vector< double > > portWeightsP;
	OutputPort< bool > portReady;

	OutputPort< vector< double > > portCurrentReference;
	vector< double > currentReference;
	
	InputPort< bool > portTrigger;
	bool trigger;

	//
	// TODO Logging
	//

	/// Our logging category
//	OCL::logging::Category* logger;

};
} // namespace
#endif // SIMPLE_TRAJECTORY_GENERATOR_HPP
