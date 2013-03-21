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
	
	this->addPort("portReady", portReady)
			.doc("Indicates whether the reference has been written. "
					"User component should always read this first before reading the reference");
	
	this->addPort("portCurrentReference", portCurrentReference)
			.doc("The current reference we are trying to follow. For reporting purposes");
	
	//
	// Properties
	//
	this->addProperty("N", N)
			.doc("Horizon length");
	this->addProperty("NX", NX)
			.doc("# of states");
	this->addProperty("NU", NU)
			.doc("# of controls");

	this->addProperty("referencesFileName", referencesFileName)
			.doc("Name of a file with references");
	this->addProperty("weightsPFileName", weightsPFileName)
			.doc("Name of a file with weihting matrices P");
}

bool SimpleTrajectoryGenerator::configureHook()
{
	
	bool status;
	unsigned i;
	
	// References
	status = readDataFromFile(referencesFileName.c_str(), references, 0, NX + NU);
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
	status = readDataFromFile(weightsPFileName.c_str(), weightsP, 0, NX * NX);
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
	
	//
	// Initialize and output the relevant output ports
	// Initilization with the references from the file
	//
	
	vector< double >::iterator itExecRefs;
	
	refCounter = 0;
	
	// Initialize execution references
	execReferences.resize(N * NX + N * NU, 0.0);
	
	for (i = 0, itExecRefs = execReferences.begin(); i < N; ++i)
		copy(references[refCounter + i].begin(), references[refCounter + i].end(), itExecRefs + i * (NX + NU));
	
	portReferences.setDataSample( execReferences );
	portReferences.write( execReferences );
	
	// Initialize execution P matrices
	execWeightsP.resize(NX * NX, 0.0);
	
	copy(weightsP[ refCounter ].begin(), weightsP[ refCounter ].end(), execWeightsP.begin());
	
	portWeightsP.setDataSample( execWeightsP );
	portWeightsP.write( execWeightsP );

	portReady.setDataSample( false );
	portReady.write( false );
	
	// Initialize references for logging purposes
	currentReference.resize(NX, 0.0);
	portCurrentReference.setDataSample( currentReference );
	portCurrentReference.write( currentReference );
	
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
	unsigned i;
	vector< double >::iterator itExecRefs;
	
	portTrigger.read( trigger );
	if (trigger == false)
		return;
	
	for (i = 0, itExecRefs = execReferences.begin(); i < N; ++i)
		copy(references[(refCounter + i) % numOfRefs].begin(), references[(refCounter + i) % numOfRefs].end(), itExecRefs + i * (NX + NU));

	portReferences.write( execReferences );

	copy(references[refCounter % numOfRefs].begin(), references[refCounter % numOfRefs].begin()+NX,currentReference.begin());
	portCurrentReference.write( currentReference );

	copy(weightsP[(refCounter + N) % numOfRefs].begin(), weightsP[(refCounter + N) % numOfRefs].end(), execWeightsP.begin());
	portWeightsP.write( execWeightsP );
	refCounter++;
	refCounter = refCounter % numOfRefs;
	
	portReady.write(true);
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
				log( Error ) << "numCols is wrong: " << linedata.size() << " vs " << numCols << endl;
				
				file.close();

				return false;
			}

			data.push_back( linedata );
		}

		file.close();

		if (data.size() != numRows && numRows > 0)
		{
			log( Error ) << "numRows is wrong: " << data.size() << " vs " << numRows << endl;
			return false;
		}
	}
	else
	{
		log( Error ) << "Could not open file " << fileName << endl;
		return false;
	}

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
