#include "imuBuffer.hpp"
#include <ocl/Component.hpp>

ORO_CREATE_COMPONENT( OCL::ImuBuffer)

using namespace std;
using namespace RTT;
using namespace Orocos;
using namespace BFL;


namespace OCL
{
	 ImuBuffer::ImuBuffer(std::string name)
		 : TaskContext(name)
	 {

		// Add properties

		properties()->addProperty( "prop",_prop ).doc("The description of the property");

		// Add ports

		addEventPort( "imuData",_imuData, boost::bind(&ImuBuffer::addMeasurement,this) ).doc("The IMU data: omegax, omegay, omegaz, ax, ay, az");
		addPort( "imuMeanCov",_imuMeanCov ).doc("The mean and covariance of last and second to last imu data: mean_last, 1/cov_last, mean_prev, 1/cov_prev");
		addPort("imuCameraRatio" , _imuCameraRatio).doc("The ratio of imu freq. on camera freq.");
		imuMeanCov.resize(24,0.0);
		imuMean_last.resize(6,0.0);
		imuMean_prev.resize(6,0.0);
		imucov_last.resize(6,0.0);
		imucov_prev.resize(6,0.0);
		halfBufferIndex = 0;
		fullBufferIndex = 0;

		sigma_omega = 1.0;
		sigma_acc = 0.1;
		addProperty("sigma_omega",sigma_omega).doc("The standard deviation of the angular velocity measurements. Default = 1.0");
		addProperty("sigma_acc",sigma_acc).doc("The standard deviation of the acceleration measurements. Default = 0.1");

		_imuMeanCov.setDataSample( imuMeanCov );
		_imuMeanCov.write( imuMeanCov );
	}
	

	ImuBuffer::~ImuBuffer()
	{
	}

	bool  ImuBuffer::configureHook()
	{
		if( !_imuCameraRatio.connected() )
		{
			cout << "The port with ratio of imu on camera is not connected" << endl;
			return false;
		}
		int imuCameraRatio = 50;
		_imuCameraRatio.read(imuCameraRatio);
		halfBuffer.resize(imuCameraRatio/2);
		fullBuffer.resize(imuCameraRatio);
		vector<double> zeros;
		zeros.resize(6);
		memset(&zeros[0],0,zeros.size()*sizeof(double));
		for( unsigned int i = 0; i < halfBuffer.size() ; i++ )
		{
			halfBuffer[i] = zeros;
		}
		for( unsigned int i = 0; i < fullBuffer.size() ; i++ )
		{
			fullBuffer[i] = zeros;
		}
		return true;
	 }

	bool  ImuBuffer::startHook()
	{
		return true;
	}

	void  ImuBuffer::updateHook()
	{
	}


	void ImuBuffer::addMeasurement()
	{

		_imuData.read(imuData);

		for(unsigned int i = 0 ; i < 6 ; i++)
		{
			if(isnan(imuData[i])){
				return;
			}
		}

		fullBuffer[fullBufferIndex] = halfBuffer[halfBufferIndex]; // The overflowing element of halfBuffer goes into fullBuffer
		halfBuffer[halfBufferIndex] = imuData;
		halfBufferIndex++;halfBufferIndex = halfBufferIndex%halfBuffer.size();
		fullBufferIndex++;fullBufferIndex = fullBufferIndex%fullBuffer.size();

		memset(&imuMean_last[0],0,imuMean_last.size()*sizeof(double));
		memset(&imucov_last[0],0,imucov_last.size()*sizeof(double));
		memset(&imuMean_prev[0],0,imuMean_prev.size()*sizeof(double));
		memset(&imucov_prev[0],0,imucov_prev.size()*sizeof(double));

		// Compute the mean
		for(unsigned int i = 0 ; i < 6 ; i++ )
		{
			for(unsigned int j = 0 ; j < halfBuffer.size() ; j++ )
			{
				imuMean_last[i] += halfBuffer[j][i];
			}
			for(unsigned int j = 0 ; j < fullBuffer.size() ; j++ )
			{
				imuMean_prev[i] += fullBuffer[j][i];
			}
			imuMean_last[i] = imuMean_last[i]/halfBuffer.size();
			imuMean_prev[i] = imuMean_prev[i]/fullBuffer.size();
		}

		// Compute the covariance
		for(unsigned int i = 0 ; i < 6 ; i++ )
		{
			for(unsigned int j = 0 ; j < halfBuffer.size() ; j++ )
			{
				imucov_last[i] += (halfBuffer[j][i]-imuMean_last[i])*(halfBuffer[j][i]-imuMean_last[i]);

			}
			for(unsigned int j = 0 ; j < fullBuffer.size() ; j++ )
			{
				imucov_prev[i] += (fullBuffer[j][i]-imuMean_prev[i])*(fullBuffer[j][i]-imuMean_prev[i]);
			}
			imucov_last[i] = 1.0/(  ( halfBuffer.size()-1.0 )*(halfBuffer.size())  )*imucov_last[i];
			imucov_prev[i] = 1.0/(  ( fullBuffer.size()-1.0 )*(fullBuffer.size())  )*imucov_prev[i];
		}
		// Use fixed values for now
		for(unsigned int i = 0 ; i < 3 ; i++ )
		{
			imucov_last[i] = 1.0/sigma_omega/sigma_omega;
			imucov_last[i+3] = 1.0/sigma_acc/sigma_acc;
			imucov_prev[i] = 1.0/sigma_omega/sigma_omega;
			imucov_prev[i+3] = 1.0/sigma_acc/sigma_acc;
		}

		// Write it on the port
		for(unsigned int i = 0 ; i < 6 ; i++ )
		{
			imuMeanCov[i] = imuMean_last[i];
			imuMeanCov[i+6] = imucov_last[i];
			//imuMeanCov[i+6] = 1.0/imucov_last[i];
			imuMeanCov[i+12] = imuMean_prev[i];
			//imuMeanCov[i+18] = 1.0/imucov_prev[i];
			imuMeanCov[i+18] = imucov_prev[i];
		}
		_imuMeanCov.write(imuMeanCov);
	}

	void  ImuBuffer::stopHook()
	{
	}

	void  ImuBuffer::cleanUpHook()
	{
	}


}//namespace

