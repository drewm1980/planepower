#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>

#include "simpleTrajectoryGenerator.hpp"

//
// Class methods
//
SimpleTrajectoryGenerator::SimpleTrajectoryGenerator(const std::string& name)
	:
		TaskContext(name, PreOperational)
//		logger(dynamic_cast<OCL::logging::Category*>(&log4cpp::Category::getInstance("dynamicMHE")))
{
	//
	// Input ports
	//
	this->addEventPort("portTrigger", portTrigger)
			.doc("Trigger");
	
	//
	// Output ports
	//
	this->addPort("portReferences", portReferences)
			.doc("References");
	
	this->addPort("portWeightingMatrixP", portWeightsP)
			.doc("Weighting matrices P");
	
	//
	// Properties
	//
	this->addProperty("N", N)
			.doc("Horizon length");
	this->addProperty("NX", NX)
			.doc("# of states");
	this->addProperty("NU", NU)
			.doc("# of controls");
	
}

bool SimpleTrajectoryGenerator::configureHook()
{
	
	bool status;
	unsigned i, j;
	
	// References
	status = readDataFromFile(referencesFileName.c_str(), references, 0, NX + NU);
	if (status == false)
	{
			log( Error ) << "Error in reading a file with references" << endlog();

		return false;
	}
	if (status == false)
	{
		log( Error ) << "Error in reading a file with references" << endlog();
	
		return false;
	}
	
	numOfRefs = references.size();
	if (numOfRefs < N)
	{
		log( Error ) << "Minimum number of reference point is equal to NMPC horizon length (" << numOfRefs << ")" << endlog();
		
		return false;
	}
	
	// Weights P
	// TODO Think how to send those matrices
	status = readDataFromFile(weightsPFileName.c_str(), weightsP, 0, NX * NX);
	if (status == false)
	{
		log( Error ) << "Error in reading a file with weights P" << endlog();
		
		return false;
	}
	if (status == false)
	{
		log( Error ) << "Error in reading a file with weights P" << endlog();
		
		return false;
	}
	
	numOfWeightsP = weightsP.size();
	
	if (numOfWeightsP < 1)
	{
		log( Error ) << "Minimum number of weighting matrices is equal to 1" << endlog();
		
		return false;
	}
	
	// TODO For starters, assume ther is only one P, output that one.
	
	//
	// Initialize and output the relevant output ports
	// Initilization with the references from the file
	//
	
	vector< double >::iterator itExecRefs;
	
	refCounter = 0;
	
	execReferences.resize(N * NX + N * NU, 0.0);
	
	for (i = 0, itExecRefs = execReferences.begin(); i < N; ++i)
		copy(references[refCounter + i].begin(), references[refCounter + i].end(), itExecRefs + i * (NX + NU));
	
	portReferences.setDataSample( execReferences );
	portReferences.write( execReferences );
	
	execWeightsP.resize(NX * NX, 0.0);
	
	copy(weightsP[ refCounter ].begin(), weightsP[refCounter].end(), execWeightsP.begin());
	
	portWeightsP.setDataSample( execWeightsP );
	portWeightsP.write( execWeightsP );
	
	refCounter++;
	
	return true;
}

bool SimpleTrajectoryGenerator::startHook()
{
	refCounter = 0;

	return true;
}

void SimpleTrajectoryGenerator::updateHook()
{
	unsigned i, j;
	vector< double >::iterator itExecRefs;
	
	portTrigger.read( trigger );
	if (trigger == false)
		return;
	
	if ((refCounter + N - 1) < numOfRefs)
	{
		for (i = 0, itExecRefs = execReferences.begin(); i < N; ++i)
			copy(references[refCounter + i].begin(), references[refCounter + i].end(), itExecRefs + i * (NX + NU));
		
		portReferences.write( execReferences );
		
		refCounter++;
	}
}

void SimpleTrajectoryGenerator::stopHook( )
{
}

void SimpleTrajectoryGenerator::cleanupHook( )
{
}

void SimpleTrajectoryGenerator::errorHook( )
{
}

bool SimpleTrajectoryGenerator::readDataFromFile(const char* fileName, vector< vector< double > >& data, unsigned numRows, unsigned numCols)
{
	ifstream file( fileName );
	string line;

	if ( file.is_open() )
	{
		data.clear();

		while( getline(file, line) )
		{
			istringstream linestream( line );
			vector< double > linedata;
			double number;

			while( linestream >> number )
			{
				linedata.push_back( number );
			}

			if (linedata.size() != numCols && numCols > 0)
			{
				file.close();

				return false;
			}

			data.push_back( linedata );
		}

		file.close();

		if (data.size() != numRows && numRows > 0)
			return false;
	}
	else
		return false;

	return true;
}

void SimpleTrajectoryGenerator::printMatrix(string name, vector< vector< double > > data)
{
	cout << name << ": " << endl;
	for(unsigned i = 0; i < data.size(); ++i)
	{
		for(unsigned j = 0; j < data[ i ].size(); ++j)
			cout << data[ i ][ j ] << " ";

		cout << endl;
	}
}

ORO_CREATE_COMPONENT( SimpleTrajectoryGenerator )
