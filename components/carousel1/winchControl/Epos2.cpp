// Copyright (C) 2009-2010 Institut de Robòtica i Informàtica Industrial, CSIC-UPC.
// Author Martí Morta Garriga  (mmorta@iri.upc.edu)
// All rights reserved.
//
// Copyright (C) 2013 Jochen Sprickerhof <jochen@sprickerhof.de>
//
// Copyright (C) 2014 Milan Vukov <milan.vukov@esat.kuleuven.be>
//
// This file is part of IRI EPOS2 Driver
// IRI EPOS2 Driver is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Epos2.h"

#include <iostream>
#include <exception>

using namespace std;

#include <boost/exception/all.hpp>
#include <boost/asio.hpp> // include boost

using namespace::boost::asio;  // save tons of typing

#define DEBUG 0

// Debugging macro
#define LOG() \
	if (DEBUG == 0); else cout << "\n[EPOS2]\t"

CEpos2::CEpos2(const std::string& _port, unsigned _baud, int8_t _nodeId)
  : port( _port ), baud( _baud ), nodeId( _nodeId ),
    io(), serialPort( io )
{}

CEpos2::~CEpos2()
{}

void CEpos2::init()
{
    this->openDevice();
    uint32_t status = this->readStatusWord();
    LOG() << "Status word: " << hex << status;
}

void CEpos2::close()
{
    this->disableVoltage();
}

void CEpos2::openDevice()
{
  if(serialPort.is_open() == true)
    serialPort.close();

	try
	{
		serialPort.open( port.c_str() );
	}
	catch (boost::system::system_error& e)
	{
		LOG() << e.what() << endl;
		throw( e );
	}

	LOG() << "Device is created";

	serialPort.cancel();
	LOG() << "Cancel all asynchronous operations associated with the serial port.";

	// Set option
	serialPort.set_option( serial_port_base::baud_rate( baud ));
	serialPort.set_option( serial_port_base::character_size( 8 ));
	serialPort.set_option(
		serial_port_base::stop_bits( serial_port_base::stop_bits::one ));
	serialPort.set_option(
		serial_port_base::parity( serial_port_base::parity::none ));
	serialPort.set_option(
		serial_port_base::flow_control( serial_port_base::flow_control::none ));

	LOG() << "Device is configured";
}

int32_t CEpos2::readObject(int16_t index, int8_t subindex)
{
  uint32_t ec, result;
    uint16_t req_frame[ 4 ];
    uint16_t ans_frame[ 6 ];

    req_frame[ 0 ] = 0x1001;     // header (OPCODE, LEN - 1), RS232 specific
    req_frame[ 1 ] = index;      // index
    req_frame[ 2 ] = ((0x0000 | this->nodeId) << 8) | subindex; // node_id subindex
    req_frame[ 3 ] = 0x0000;     // CRC

    // TODO Make this non-blocking one day
    this->sendFrame( req_frame );
    this->receiveFrame( ans_frame );
    // TODO Checksum checking

    ec = (uint32_t)ans_frame[ 1 ] + ((uint32_t)ans_frame[ 2 ] << 16);
    LOG() << "Error code: " << hex << ec;
    result = (uint32_t)ans_frame[ 3 ] + ((uint32_t)ans_frame[ 4 ] << 16);

    return result;
}

int32_t CEpos2::writeObject(int16_t index, int8_t subindex, int32_t data)
{
  uint32_t ec;
    uint16_t req_frame[ 6 ];
    uint16_t ans_frame[ 4 ];
    
    req_frame[0] = 0x1103;     // header (OPCODE, LEN - 1)
    req_frame[1] = index;      // data
    req_frame[2] = ((0x0000 | this->nodeId) << 8) | subindex;
    req_frame[3] = data & 0x0000FFFF;
    req_frame[4] = data >> 16;
    req_frame[5] = 0x0000;     // checksum

    // TODO Non-blocking
    this->sendFrame( req_frame );
    this->receiveFrame( ans_frame );
    // TODO Checksum

    ec = (uint32_t)ans_frame[ 1 ] + ((uint32_t)ans_frame[ 2 ] << 16);
    LOG() << "Error code: " << hex << ec;

    return ec;
}

//     SEND FRAME
// ----------------------------------------------------------------------------

void CEpos2::sendFrame(uint16_t *frame)
{
    uint8_t trans_frame[ 32 ];
    uint16_t length = (frame[0] & 0x00FF) + 2 + 1;

    // Add checksum to the frame
    frame[length - 1] = this->computeChecksum(frame, length);

    // OPCODE
    trans_frame[ 0 ] = frame[ 0 ] >> 8;
    // LEN - 1
    trans_frame[ 1 ] = frame[ 0 ] & 0x00FF;

    unsigned tf_i = 2;
    for (unsigned i = 1; i < length; ++i)
    {
      trans_frame[ tf_i++ ] = frame[ i ] & 0x00FF;
      trans_frame[ tf_i++ ] = frame[ i ]  >> 8;
    }

    LOG() << "sendFrame start";

#if DEBUG == 1
    LOG() << "Sending frame (hex): ";
    for (unsigned el = 0; el < tf_i; ++el)
      cout << hex << int(trans_frame[ el ]) << ", ";
#endif // DEBUG == 1

    char ack;
    unsigned written, ack_len;
    
    written = write(serialPort, buffer(trans_frame, 1));
    ack_len = read(serialPort, buffer(&ack, 1));
    if (ack != 'O')
      throw runtime_error( "Wrong ACK received" );

    written = write(serialPort, buffer(trans_frame + 1, tf_i - 1));
    ack_len = read(serialPort, buffer(&ack, 1));
    if (ack != 'O')
      throw runtime_error( "Wrong ACK received" );
      
    LOG() << "sendFrame end";
}

void CEpos2::receiveFrame(uint16_t* ans_frame)
{
    uint8_t read_buffer[ 32 ];
    uint8_t opcode, lenm1;

    LOG() << "readFrame start";
    unsigned received = read(serialPort, buffer(&opcode, 1));
    if (opcode == 0x00)
    {
      //LOG() << "Received 0x00";

      char ack = 'O';
      
      write(serialPort, buffer(&ack, 1));
      //LOG() << "Written ack";
      
      received = read(serialPort, buffer(&lenm1, 1));
      //LOG() << "Received length " << lenm1;
      received = read(serialPort, buffer(read_buffer, (lenm1 + 1) * 2 + 2));

      write(serialPort, buffer(&ack, 1));
      //LOG() << "Written ack";
    }
    else
      runtime_error( "Wrong OPCODE received" );

    // First word in the frame
    ans_frame[ 0 ] = ((uint16_t)opcode << 8) + lenm1;

    // The rest of the frame
    unsigned length = ((lenm1 + 1) * 2 + 2 * 2) / 2;
    for(unsigned i = 1, tf_i = 0; i < length; ++i)
    {
      	ans_frame[ i ] = read_buffer[ tf_i++ ];
	ans_frame[ i ] += (uint16_t)read_buffer[ tf_i++ ] << 8;
    }

#if DEBUG == 1
    LOG() << "Received frame data (hex): ";
    for (unsigned el = 0; el < length; ++ el)
      cout << hex << ans_frame[ el ] << ", ";
#endif // DEBUG == 1

    LOG() << "readFrame end";
}


uint16_t CEpos2::computeChecksum(uint16_t *pDataArray, uint16_t numberOfWords)
{
    uint16_t shifter, c;
    uint16_t carry;
    uint16_t CRC = 0;

    while (numberOfWords--)
    {
      c = *pDataArray++;
      shifter = 0x8000;                 //Initialize BitX to Bit15 
      
      do
        {
            carry = CRC & 0x8000;    //Check if Bit15 of CRC is set
            CRC <<= 1;               //CRC = CRC * 2
            if(c & shifter) CRC++;   //CRC = CRC + 1, if BitX is set in c
            if(carry) CRC ^= 0x1021; //CRC = CRC XOR G(x), if carry is true
            shifter >>= 1;           //Set BitX to next lower Bit, shifter = shifter/2
        }
        while(shifter);
    }
    
    return CRC;
}


//----------------------------------------------------------------------------
//   MANAGEMENT
// ----------------------------------------------------------------------------
//     Get State
// ----------------------------------------------------------------------------

long CEpos2::getState()
{
    long ans = this->readObject(0x6041, 0x00);

    LOG() << "ESTAT: " << hex << ans;

    // OBTENIR EL NUMERO D'ESTAT
    bool bits[16];
    bits[0]=  (ans & 0x0001);
    bits[1]=  (ans & 0x0002);
    bits[2]=  (ans & 0x0004);
    bits[3]=  (ans & 0x0008);

    bits[4]=  (ans & 0x0010);
    bits[5]=  (ans & 0x0020);
    bits[6]=  (ans & 0x0040);
    bits[7]=  (ans & 0x0080);

    bits[8]=  ans & 0x0100;
    bits[9]=  ans & 0x0200;
    bits[10]= ans & 0x0400;
    bits[11]= ans & 0x0800;

    bits[12]= ans & 0x1000;
    bits[13]= ans & 0x2000;
    bits[14]= ans & 0x4000;
    bits[15]= ans & 0x8000;

    LOG()   << bits[15]
            << bits[14]
            << bits[13]
            << bits[12]
            << bits[11]
            << bits[10]
            << bits[9]
            << bits[8]
            << bits[7]
            << bits[6]
            << bits[5]
            << bits[4]
            << bits[3]
            << bits[2]
            << bits[1]
            << bits[0];

    if(bits[14])
    {
        if(bits[4])
        {
            LOG() << "State: Measure Init" << endl;
            return(MEASURE_INIT);
        }
        else
        {
            LOG() << "State: Refresh" << endl;
            return(REFRESH);
        }
    }
    else
    {
        if(!bits[8])
        {
            LOG() << "State: Start" << endl;
            return(START);
        }
        else
        {
            if(bits[6])
            {
               	LOG() << "State: Switch on disabled" << endl;
                return(SWITCH_ON_DISABLED);
            }
            else
            {
                if(bits[5])
                {
                    if(bits[4])
                    {
                        LOG() << "State: Operation Enable" << endl;
                        return(OPERATION_ENABLE);
                    }
                    else
                    {
                        if(bits[1])
                        {
                            LOG() << "State: Switched On" << endl;
                            return(SWITCH_ON);
                        }
                        else
                        {
                            LOG() << "State: Ready to Switch On" << endl;
                            return(READY_TO_SWITCH_ON);
                        }
                    }
                }
                else
                {
                    if(!bits[3])
                    {
                        if(bits[2])
                        {
                            LOG() << "State: Quick Stop Active" << endl;
                            return(QUICK_STOP);
                        }
                        else
                        {
                            LOG() << "State: Not Ready to Switch On" << endl;
                            return(NOT_READY_TO_SWITCH_ON);
                        }
                    }
                    else
                    {
                        if(bits[4])
                        {
                            LOG() << "State: Fault Reaction Active (Enabled)" << endl;
                            return(QUICK_STOP_ACTIVE_ENABLE);
                        }
                        else
                        {
                            if(bits[2])
                            {
                                LOG() << "State: Fault Reaction Active (Disabled)" << endl;
                                return(QUICK_STOP_ACTIVE_DISABLE);
                            }
                            else
                            {
                                LOG() << "State: Fault" << endl;
                                return(FAULT);
                            }
                        }
                    }
                }
            }
        }
    }
    
    LOG() << this->searchErrorDescription( this->readError() ) << endl;
}

//     SHUTDOWN (transition)
// ----------------------------------------------------------------------------

void CEpos2::shutdown()
{
    this->writeObject(0x6040, 0x00, 0x06);
}

//     SWITCH ON (transition)
// ----------------------------------------------------------------------------

void CEpos2::switchOn()
{
    this->writeObject(0x6040, 0x00, 0x07);
}

//     DISABLE VOLTAGE (transition)
// ----------------------------------------------------------------------------

void CEpos2::disableVoltage()
{
    this->writeObject(0x6040, 0x00, 0x00);
}

//     QUICK STOP (transition)
// ----------------------------------------------------------------------------

void CEpos2::quickStop()
{
    this->writeObject(0x6040, 0x00, 0x02);
}

//     DISABLE OPERATION (transition)
// ----------------------------------------------------------------------------

void CEpos2::disableOperation()
{
    this->writeObject(0x6040, 0x00, 0x07);
}

//     ENABLE OPERATION (transition)
// ----------------------------------------------------------------------------

void CEpos2::enableOperation()
{
    this->writeObject(0x6040, 0x00, 0x0F);
}

//     FAULT RESET (transition)
// ----------------------------------------------------------------------------

void CEpos2::faultReset()
{
    this->writeObject(0x6040, 0x00, 0x80);
}

//----------------------------------------------------------------------------
//   OPERATION MODES
// ----------------------------------------------------------------------------
//     GET OPERATION MODE
// ----------------------------------------------------------------------------

long CEpos2::getOperationMode()
{
   return static_cast<int8_t>(this->readObject(0x6061, 0x00));
}

//     GET OPERATION MODE DESCRIPTION
// ----------------------------------------------------------------------------

std::string CEpos2::getOpModeDescription(long opmode)
{
    std::string name;

    switch(opmode)
    {
    case VELOCITY:
        name="Velocity";
        break;
    case POSITION:
        name="Position";
        break;
    case PROFILE_POSITION:
        name="Profile Position";
        break;
    case PROFILE_VELOCITY:
        name="Profile Velocity";
        break;
    case INTERPOLATED_PROFILE_POSITION:
        name="Interpolated Profile Position";
        break;
    case HOMING:
        name="Homing";
        break;
    }

    return(name);
}

//     SET OPERATION MODE
// ----------------------------------------------------------------------------

void CEpos2::setOperationMode(long opmode)
{
    this->writeObject(0x6060, 0x00,opmode);
}

//     ENABLE CONTROLLER
// ----------------------------------------------------------------------------

void CEpos2::enableController()
{
    int estat=0, timeout=0;
    bool controller_connected = false;

    estat = this->getState();

    while( !controller_connected && timeout<10 )
    {
        switch(estat)
        {
        case 0:
            // FAULT
            this->faultReset();
            timeout++;
            break;
        case 1:
            // START
            break;
        case 2:
            // NOT_READY_TO_SWITCH_ON
            break;
        case 3:
            // SWITCH_ON_DISABLED
            timeout++;
            this->shutdown();
            break;
        case 4:
            // READY_TO_SWITCH_ON
            this->switchOn();
            break;
        case 5:
            // SWITCH_ON
            controller_connected = true;
            break;
        case 6:
            // REFRESH
	  controller_connected = true;
            break;
        case 7:
            // MEASURE_INIT
            break;
        case 8:
            // OPERATION_ENABLE
            this->disableOperation();
            break;
        case 9:
            // QUICK_STOP
            this->disableVoltage();
            break;
        case 10:
            // QUICK_STOP_ACTIVE_DISABLE
            break;
        case 11:
            // QUICK_STOP_ACTIVE_ENABLE
            break;
        }
        estat = this->getState();
    }
}

//     ENABLE MOTOR
// ----------------------------------------------------------------------------

void CEpos2::enableMotor(long opmode)
{
    int estat;

    estat = this->getState();

    if( estat == SWITCH_ON )
    {
        this->enableOperation();
    }

    if( opmode != NO_OPERATION )
    {
        if( this->getOperationMode() != opmode )
        {
            this->setOperationMode(opmode);
        }
    }
}


//     IS TARGET REACHED ? (Shared between modes)
// ----------------------------------------------------------------------------

bool CEpos2::isTargetReached()
{

    long ans = this->readObject(0x6041, 0x00);

    LOG() << "Estat: hex = " << hex << ans << " / dec = " << dec << ans;
    
	// OBTENIR EL NUMERO D'ESTAT

    // bit10 = 0 , not reached
    // bit10 = 1 , reached

    return((bool)(ans & 0x0400));
}

//----------------------------------------------------------------------------
//   MODE VELOCITY
// ----------------------------------------------------------------------------
//     GET TARGET VELOCITY
// ----------------------------------------------------------------------------

long CEpos2::getTargetVelocity()
{
    return this->readObject(0x206B, 0x00);
}

//     SET TARGET VELOCITY
// ----------------------------------------------------------------------------

void CEpos2::setTargetVelocity(long velocity)
{
    this->writeObject(0x206B, 0x00,velocity);
}

//     START VELOCITY
// ----------------------------------------------------------------------------

void CEpos2::startVelocity()
{
    this->writeObject(0x6040, 0x00, 0x010f);
}

//     STOP VELOCITY
// ----------------------------------------------------------------------------

void CEpos2::stopVelocity()
{
    // just velocity command = 0
    this->writeObject(0x206B, 0x00,0x0000);
}

//----------------------------------------------------------------------------
//   MODE PROFILE VELOCITY
// ----------------------------------------------------------------------------
//     GET TARGET PROFILE VELOCITY
// ----------------------------------------------------------------------------

long CEpos2::getTargetProfileVelocity()
{

    return this->readObject(0x60FF, 0x00);
}

//     SET TARGET PROFILE VELOCITY
// ----------------------------------------------------------------------------

void CEpos2::setTargetProfileVelocity(long velocity)
{
    this->writeObject(0x60FF, 0x00, velocity);
}

//     START PROFILE VELOCITY
// ----------------------------------------------------------------------------

void CEpos2::startProfileVelocity()
{
    int intmode = 0x000F;

    this->writeObject(0x6040, 0x00,intmode);
}

//     STOP PROFILE VELOCITY
// ----------------------------------------------------------------------------

void CEpos2::stopProfileVelocity()
{
    int intmode = 0x010F;
    this->writeObject(0x6040, 0x00,intmode);
}

//----------------------------------------------------------------------------
//   MODE PROFILE POSITION
// ----------------------------------------------------------------------------
//     GET TARGET PROFILE POSITION
// ----------------------------------------------------------------------------

long CEpos2::getTargetProfilePosition()
{
    return this->readObject(0x607A, 0x00);
}

//     SET TARGET PROFILE POSITION
// ----------------------------------------------------------------------------

void CEpos2::setTargetProfilePosition(long position)
{
    this->writeObject(0x607A, 0x00,position);
}

// 0 halt, 1 abs, 2 rel
void CEpos2::startProfilePosition(epos_posmodes mode, bool blocking, bool wait, bool new_point)
{

    int halt        = mode==HALT ?      0x0100 : 0x0000;
    int rel         = mode==RELATIVE ?  0x0040 : 0x0000;
    int nowait      = !wait ?           0x0020 : 0x0000;
    int newsetpoint = new_point ?       0x0010 : 0x0000;

    int intmode = 0x000F | halt | rel | nowait | newsetpoint;

    this->writeObject(0x6040, 0x00,intmode);

    if( blocking )
    {

        while( !this->isTargetReached() )
        {
            this->getMovementInfo();
            usleep(1000);
        }
    }

}



// Current

long CEpos2::getTargetCurrent()
{
    return 1;
}

void CEpos2::setTargetCurrent(long current) {}

void CEpos2::startCurrent() {}

void CEpos2::stopCurrent() {}



//----------------------------------------------------------------------------
//   CONTROL PARAMETERS
// ----------------------------------------------------------------------------

long CEpos2::getCurrentPGain()
{
    return this->readObject(0x60F6, 0x01);
}

void CEpos2::setCurrentPGain(long gain)
{
    this->writeObject(0x60F6, 0x01,gain);
}

long CEpos2::getCurrentIGain()
{
    return this->readObject(0x60F6, 0x02);
}

void CEpos2::setCurrentIGain(long gain)
{
    this->writeObject(0x60F6, 0x02,gain);
}

// Velocity

long CEpos2::getVelocityPGain()
{
    return this->readObject(0x60F9, 0x01);
}

void CEpos2::setVelocityPGain(long gain)
{
    this->writeObject(0x60F9, 0x01,gain);
}

long CEpos2::getVelocityIGain()
{
    return this->readObject(0x60F9, 0x02);
}

void CEpos2::setVelocityIGain(long gain)
{
    this->writeObject(0x60F9, 0x03,gain);
}

long CEpos2::getVelocitySetPointFactorPGain()
{
    return this->readObject(0x60F9, 0x03);
}

void CEpos2::setVelocitySetPointFactorPGain(long gain)
{
    this->writeObject(0x60F9, 0x03,gain);
}

// Position

long CEpos2::getPositionPGain()
{
    return this->readObject(0x60FB, 0x01);
}

void CEpos2::setPositionPGain(long gain)
{
    this->writeObject(0x60FB, 0x01,gain);
}

long CEpos2::getPositionIGain()
{
    return this->readObject(0x60FB, 0x02);
}

void CEpos2::setPositionIGain(long gain)
{
    this->writeObject(0x60FB, 0x02,gain);
}

long CEpos2::getPositionDGain()
{

    return this->readObject(0x60FB, 0x03);
}

void CEpos2::setPositionDGain(long gain)
{
    this->writeObject(0x60FB, 0x03,gain);
}

long CEpos2::getPositionVFFGain()
{
    return this->readObject(0x60FB, 0x04);
}

void CEpos2::setPositionVFFGain(long gain)
{
    this->writeObject(0x60FB, 0x04,gain);
}

long CEpos2::getPositionAFFGain()
{
    return this->readObject(0x60FB, 0x05);
}

void CEpos2::setPositionAFFGain(long gain)
{
    this->writeObject(0x60FB, 0x05,gain);
}

void CEpos2::getControlParameters(long &cp,long &ci,long &vp,long &vi,
                                  long &vspf, long &pp,long &pi,long &pd,
                                  long &pv,long &pa)
{
    cp = this->getCurrentPGain();
    ci = this->getCurrentIGain();
    vp = this->getVelocityPGain();
    vi = this->getVelocityIGain();
    vspf = this->getVelocitySetPointFactorPGain();
    pp = this->getPositionPGain();
    pi = this->getPositionIGain();
    pd = this->getPositionDGain();
    pv = this->getPositionVFFGain();
    pa = this->getPositionAFFGain();
}

void CEpos2::setControlParameters(long cp,long ci,long vp,long vi,long vspf,
                                  long pp,long pi,long pd,long pv,long pa)
{
    this->setCurrentPGain(cp);
    this->setCurrentIGain(ci);
    this->setVelocityPGain(vp);
    this->setVelocityIGain(vi);
    this->setVelocitySetPointFactorPGain(vspf);
    this->setPositionPGain(pp);
    this->setPositionIGain(pi);
    this->setPositionDGain(pd);
    this->setPositionVFFGain(pv);
    this->setPositionAFFGain(pa);

    this->getControlParameters(cp,ci,vp,vi,vspf,pp,pi,pd,pv,pa);
}

void CEpos2::printControlParameters()
{
	long cp, ci, vp, vi, vspf, pp, pi, pd, pv, pa;

	this->getControlParameters(cp, ci, vp, vi, vspf, pp, pi, pd, pv, pa);

	LOG()	<< dec << "Control Parameters:" << endl
			<< "Current:  P = " << cp << "  I = " << ci << endl
			<< "Velocity: P = " << vp << "  I = " << vi << "	SetPointFactorP = " << vspf << endl
			<< "Position: P = " << pp << "  I = " << pi << "	D = "<< pd << endl
			<< "        Vff = " << pv << "  Aff = " << pa << endl;
}

//----------------------------------------------------------------------------
//   PROFILE PARAMETERS
// ----------------------------------------------------------------------------

long CEpos2::getProfileVelocity(void)
{
    return this->readObject(0x6081, 0x00);
}

void CEpos2::setProfileVelocity(long velocity)
{
    this->writeObject(0x6081, 0x00,velocity);
}

long CEpos2::getProfileMaxVelocity(void)
{
    return this->readObject(0x607F, 0x00);
}

void CEpos2::setProfileMaxVelocity(long velocity)
{
    this->writeObject(0x607F, 0x00,velocity);
}

long CEpos2::getProfileAcceleration(void)
{
    return this->readObject(0x6083, 0x00);
}

void CEpos2::setProfileAcceleration(long acceleration)
{
    this->writeObject(0x6083, 0x00,acceleration);
}

long CEpos2::getProfileDeceleration(void)
{
    return this->readObject(0x6084, 0x00);
}

void CEpos2::setProfileDeceleration(long deceleration)
{
    this->writeObject(0x6084, 0x00,deceleration);
}

long CEpos2::getProfileQuickStopDecel(void)
{
    return this->readObject(0x6085, 0x00);
}

void CEpos2::setProfileQuickStopDecel(long deceleration)
{
    this->writeObject(0x6085, 0x00,deceleration);
}

long CEpos2::getProfileType(void)
{
    return this->readObject(0x6086, 0x00);
}

void CEpos2::setProfileType(long type)
{
    this->writeObject(0x6086, 0x00,type);
}

long CEpos2::getMaxAcceleration(void)
{
    return this->readObject(0x60C5, 0x00);
}

void CEpos2::setMaxAcceleration(long max_acceleration)
{
    this->writeObject(0x60C5, 0x00,max_acceleration);
}

void CEpos2::getProfileData(long &vel,long &maxvel,long &acc,long &dec,
                            long &qsdec, long &maxacc, long &type)
{
    vel    = this->getProfileVelocity();
    maxvel = this->getProfileMaxVelocity();
    acc    = this->getProfileAcceleration();
    dec    = this->getProfileDeceleration();
    qsdec  = this->getProfileQuickStopDecel();
    maxacc = this->getMaxAcceleration();
    type   = this->getProfileType();
}

void CEpos2::setProfileData(long vel,long maxvel,long acc,long dec,
                            long qsdec,long maxacc,long type)
{
    this->setProfileVelocity(vel);
    this->setProfileMaxVelocity(maxvel);
    this->setProfileAcceleration(acc);
    this->setProfileDeceleration(dec);
    this->setProfileQuickStopDecel(qsdec);
    this->setMaxAcceleration(maxacc);
    this->setProfileType(type);

    long v, m, a, d, q, ma, t;
    this->getProfileData(v, m, a, d, q, ma, t);
}

//----------------------------------------------------------------------------
//   READ INFORMATION
// ----------------------------------------------------------------------------

long CEpos2::readVelocity()
{
    return this->readObject(0x2028, 0x00);
}

long CEpos2::readVelocitySensorActual()
{
    return this->readObject(0x6069, 0x00);
}

long CEpos2::readVelocityDemand()
{
    return this->readObject(0x606B, 0x00);
}

long CEpos2::readVelocityActual	()
{
    return this->readObject(0x606C, 0x00);
}

long CEpos2::readCurrent()
{
    long ans = this->readObject(0x6078, 0x00);
    return this->getNegativeLong(ans);
}

long CEpos2::readCurrentAveraged()
{
    long ans = this->readObject(0x2027, 0x00);
    return this->getNegativeLong(ans);
}

long CEpos2::readCurrentDemanded()
{
    return this->readObject(0x2031, 0x00);
}

int32_t CEpos2::readPosition()
{
    return this->readObject(0x6064, 0x00);
}

long CEpos2::readStatusWord()
{
    return this->readObject(0x6041, 0x00);
}

long CEpos2::readEncoderCounter()
{
    return this->readObject(0x2020, 0x00);
}

long CEpos2::readEncoderCounterAtIndexPulse()
{
    return this->readObject(0x2021, 0x00);
}

long CEpos2::readHallsensorPattern()
{
    return this->readObject(0x2022, 0x00);
}

long CEpos2::readFollowingError()
{
    return this->readObject(0x20F4, 0x00);
}

void CEpos2::getMovementInfo()
{
    long vel_actual, vel_avg, vel_demand;
    int cur_actual, cur_avg, cur_demand;
    long pos;

    vel_actual = this->readVelocityActual();
    vel_avg    = this->readVelocity();
    vel_demand = this->readVelocityDemand();

    cur_actual = this->readCurrent();
    cur_avg    = this->readCurrentAveraged();
    cur_demand = this->readCurrentDemanded();

    pos        = this->readPosition();

	LOG()	<< "Movement info: " << endl
			<< "\tvel_actual " << vel_actual << endl
			<< "\tvel_avg " << vel_avg << endl
			<< "\tvel_demand " << vel_demand << endl

			<< "\tcur_actual " << cur_actual << endl
			<< "\tcur_avg " << cur_avg << endl
			<< "\tcur_demand " << cur_demand << endl

			<< "\tpor " << pos << endl;
}

//----------------------------------------------------------------------------
//   ERRORS
// ----------------------------------------------------------------------------

char CEpos2::readError()
{
    char error_num = 0;
    long ans = this->readObject(0x1001, 0x00);

    bool bits[8];
    bits[0]=  (ans & 0x0001);
    bits[1]=  (ans & 0x0002);
    bits[2]=  (ans & 0x0004);
    bits[3]=  (ans & 0x0008);

    bits[4]=  (ans & 0x0010);
    bits[5]=  (ans & 0x0020);
    bits[7]=  (ans & 0x0080);

    if (bits[7]) error_num=6; // Motion Error
    if (bits[5]) error_num=5; // Device profile specific
    if (bits[4]) error_num=4; // Communication Error

    if (bits[3]) error_num=3; // Temperature Error
    if (bits[2]) error_num=2; // Voltage Error
    if (bits[1]) error_num=1; // Current Error
    if (bits[0]) error_num=0; // Generic Error

    LOG() 	<< "Error: "<< error_num << " " << this->error_names[(unsigned char)error_num]
			<< " Value: 0x" << hex << ans << " , " << dec<< ans;

    return( error_num );
}

void CEpos2::readErrorHistory(long *error[5])
{
    std::string error_des;

    long number_errors = this->readObject(0x1003, 0x00);
    LOG() << "Number of Errors: " << number_errors << std::endl;

    // Read Errors
    for(int i = 1; i <= number_errors; i++)
    {
        long ans = this->readObject(0x1003, i);
        error[i] = &ans;
        error_des = this->searchErrorDescription(ans);

        LOG() << "Id: " << i << " : " << hex << "0x"<< ans << " = " << error_des << endl;
    }
}

std::string CEpos2::searchErrorDescription(long error_code)
{
    int j=0;
    bool found = false;

    // error_codes length = 34

    while( !found && j < 34 )
    {
        if( error_code == this->error_codes[j] )
        {
            found = true;

            LOG() << "Error Description "<< this->error_descriptions[j] << std::endl;

            return this->error_descriptions[j];

        }
        else
        {
            j++;
        }
    }
    if(!found) return "No Description for this error";
    else       return "Error Description";
}

//----------------------------------------------------------------------------
//   VERSIONS
// ----------------------------------------------------------------------------

long CEpos2::readVersionSoftware()
{
    return this->readObject(0x2003, 0x01);
}

long CEpos2::readVersionHardware()
{
    return this->readObject(0x2003, 0x02);
}

// SENSOR CONFIGURATION

long CEpos2::getEncoderPulseNumber()
{
    return this->readObject(0x2210, 0x01);
}

void CEpos2::setEncoderPulseNumber(long pulses)
{
    this->writeObject(0x2210, 0x01, pulses);
}

long CEpos2::getEncoderType()
{
    return 1;
}

void CEpos2::setEncoderType(long type)
{}

long CEpos2::getEncoderPolarity()
{
    return 1;
}

void CEpos2::setEncoderPolarity(long polarity)
{}

void CEpos2::getEncoderParameters(long &pulses, long &type, long &polarity)
{}

void CEpos2::setEncoderParameters(long pulses, long type, long polarity)
{}

// Motor

long CEpos2::getMotorType()
{
    return 1;
}

void CEpos2::setMotorType(long type) {}

long CEpos2::getMotorContinuousCurrentLimit()
{
    return 1;
}

void CEpos2::setMotorContinuousCurrentLimit(long current_mA) {}

long CEpos2::getMotorOutputCurrentLimit()
{
    return 1;
}

void CEpos2::setMotorOutputCurrentLimit(long current_mA) {}

long CEpos2::getMotorPolePairNumber()
{
    return 1;
}

void CEpos2::setMotorPolePairNumber(char pole_pairs) {}

long CEpos2::getThermalTimeCtWinding()
{
    return 1;
}

void CEpos2::setThermalTimeCtWinding(long time_ds) {}

//----------------------------------------------------------------------------
//   UTILITIES
// ----------------------------------------------------------------------------

long CEpos2::getMaxFollowingError()
{
    return this->readObject(0x6065, 0x00);
}

void CEpos2::setMaxFollowingError(long error)
{
    this->writeObject(0x6065, 0x00,error);
}

long CEpos2::getMinPositionLimit	()
{
    return this->readObject(0x607D, 0x01);
}

void CEpos2::setMinPositionLimit(long limit)
{
    this->writeObject(0x607D, 0x01,limit);
}


long CEpos2::getMaxPositionLimit	()
{
    return this->readObject(0x607D, 0x02);
}

void CEpos2::setMaxPositionLimit(long limit)
{
    this->writeObject(0x607D, 0x02,limit);
}

void CEpos2::disablePositionLimits(void)
{
    // min
    // -2147483647-1
    this->writeObject(0x607D, 0x01, -2147483647-1);
    // max
    this->writeObject(0x607D, 0x02, 2147483647);
}

long CEpos2::getPositionWindow()
{
    return 1;
}

void CEpos2::setPositionWindow(long window_qc) {}

long CEpos2::getPositionWindowTime()
{
    return 1;
}

void CEpos2::setPositionWindowTime(long time_ms) {}

long CEpos2::getVelocityWindow()
{
    return 1;
}

void CEpos2::setVelocityWindow(long window_rm) {}

long CEpos2::getVelocityWindowTime()
{
    return 1;
}

void CEpos2::setVelocityWindowTime(long time_ms) {}

long CEpos2::getPositionNotationIndex()
{
    return 1;
}

void CEpos2::setPositionNotationIndex(long notation) {}

long CEpos2::getVelocityNotationIndex()
{
    return 1;
}

void CEpos2::setVelocityNotationIndex(long notation) {}

long CEpos2::getAccelerationNotationIndex()
{
    return 1;
}

void CEpos2::setAccelerationNotationIndex(long notation) {}

long CEpos2::getPositionDimensionIndex()
{
    return 1;
}

void CEpos2::setPositionDimensionIndex(long Dimension) {}

long CEpos2::getVelocityDimensionIndex()
{
    return 1;
}

void CEpos2::setVelocityDimensionIndex(long Dimension) {}

long CEpos2::getAccelerationDimensionIndex()
{
    return 1;
}

void CEpos2::setAccelerationDimensionIndex(long Dimension) {}


void CEpos2::saveParameters()
{
    this->writeObject(0x1010, 0x01, 0x65766173);

}

void CEpos2::restoreDefaultParameters()
{
    this->writeObject(0x1011, 0x01, 0x64616F6C);

}

long CEpos2::getRS232Baudrate()
{
    return this->readObject(0x2002, 0x00);
}

void CEpos2::setRS232Baudrate(long baudrate)
{
    this->writeObject(0x2002, 0x00, baudrate);
}

long CEpos2::getRS232FrameTimeout()
{
    return this->readObject(0x2005, 0x00);
}

void CEpos2::setRS232FrameTimeout(long timeout)
{
    this->writeObject(0x2005, 0x00, timeout);
}

long CEpos2::getNegativeLong(long positiu)
{
    if(positiu>32767)
    {
        return(positiu-65536);
    }
    else
    {
        return(positiu);
    }
}


// #############################   I/O   ######################################

long CEpos2::getAnalogOutput1()
{
    return 1;
}

void CEpos2::setAnalogOutput1(long voltage_mV) {}

// #############################   MARKER POSITION   ##########################

long CEpos2::getPositionMarker(int buffer)
{
    int obj;
    switch(buffer)
    {
    case 0:
        obj = 1;
        break;
    case 1:
        obj = 5;
        break;
    case 2:
        obj = 6;
        break;
    }
    return this->readObject(0x2074, obj);
}

void CEpos2::setPositionMarker(char mode, char polarity, char edge_type, char digitalIN)
{
    // set the digital input as position marker & options
    this->writeObject(0x2070, digitalIN, 4);
    // mask (which functionalities are active) (bit 3 0x0008)
    this->writeObject(0x2071, 0x02, 0x0008);
    // execution (if set the function executes) (bit 3 0x0008)
    this->writeObject(0x2071, 0x04, 0x0008);

    // options
    this->writeObject(0x2071, 0x03, polarity);
    this->writeObject(0x2074, 0x02, edge_type);
    this->writeObject(0x2074, 0x03, mode);

}

void CEpos2::waitPositionMarker()
{
    long markpos = this->getPositionMarker();

    while(markpos == this->getPositionMarker())
    {
        // minimum freq = 0.05s = 20Hz
        usleep(1000000*0.05);
    }
}


// #############################   HOMING   ###################################



void CEpos2::setHoming(int home_method, int speed_pos, int speed_zero,
                       int acc, int digitalIN)
{
    // set digital input as home switch
    this->writeObject(0x2070, digitalIN, 3);
    // mask
    this->writeObject(0x2071, 0x02, 0x0004);
    this->writeObject(0x2071, 0x04, 0x000C);
    // options
    this->writeObject(0x6098, 0x00, home_method);
    this->writeObject(0x6099, 0x01, speed_pos);
    this->writeObject(0x6099, 0x02, speed_zero);
    this->writeObject(0x609A, 0x00, acc);
}

void CEpos2::doHoming(bool blocking)
{
    this->writeObject(0x6040, 0x00, 0x001F);
    if(blocking)
    {
        while(!this->isTargetReached())
            usleep(1000000*0.05);
    }
}

void CEpos2::stopHoming()
{
    this->writeObject(0x6040, 0x00, 0x010F);
}

// #############################   DIG IN   ###################################

int CEpos2::getDigInState(int digitalIN)
{
    return this->readObject(0x2070, digitalIN);
}

int CEpos2::getDigInStateMask()
{
    return this->readObject(0x2071, 0x01);
}

int CEpos2::getDigInFuncMask()
{
    return this->readObject(0x2071, 0x02);
}

int CEpos2::getDigInPolarity()
{
    return this->readObject(0x2071, 0x03);
}

int CEpos2::getDigInExecutionMask()
{
    return this->readObject(0x2071, 0x04);
}


// deprecated


void CEpos2::setHomePosition(long home_position_qc)
{
    this->writeObject(0x2081, 0x00,home_position_qc);
}
long CEpos2::getHomePosition()
{
    return this->readObject(0x2081, 0x00);
}

void CEpos2::setHome()
{
    char c;
    long home_pos=0;

    long mode_anterior = this->getOperationMode();
    this->disableOperation();
    LOG() << "Move Load to 0 position and press a key " << endl;
    LOG() << "Wait until process finishes" << endl;
    this->enableOperation();
    home_pos = this->readPosition();
    this->setOperationMode(HOMING);
    this->getOperationMode();
    this->setHomePosition(home_pos);
    this->getHomePosition();
    this->setOperationMode(mode_anterior);
    this->getOperationMode();
    LOG() << "Restart EPOS2 (unplug from current) after that the new home will be set" << std::endl;
}

const std::string CEpos2::error_names[] =
{
    "Generic Error",
    "Current Error",
    "Voltage Error",
    "Temperature Error",
    "Communication Error",
    "Device profile specific",
    "Motion Error"
};

const int CEpos2::error_codes[]=
{
    0x0000,
    0x1000,
    0x2310,
    0x3210,
    0x3220,
    0x4210,
    0x5113,
    0x5114,
    0x6100,
    0x6320,
    0x7320,
    0x8110,
    0x8111,
    0x8120,
    0x8130,
    0x8150,
    0x81FD,
    0x81FE,
    0x81FF,
    0x8210,
    0x8611,
    0xFF01,
    0xFF02,
    0xFF03,
    0xFF04,
    0xFF05,
    0xFF06,
    0xFF07,
    0xFF08,
    0xFF09,
    0xFF0A,
    0xFF0B,
    0xFF0C,
    0xFF0D
};

const std::string CEpos2::error_descriptions[]=
{
    "No Error",
    "Generic Error",
    "Over Current Error",
    "Over Voltage Error",
    "Under Voltage",
    "Over Temperature",
    "Supply Voltage (+5V) Too Low",
    "Supply Voltage Output Stage Too Low",
    "Internal Software Error",
    "Software Parameter Error",
    "Sensor Position Error",
    "CAN Overrun Error (Objects lost)",
    "CAN Overrun Error",
    "CAN Passive Mode Error",
    "CAN Life Guard Error",
    "CAN Transmit COB-ID collision",
    "CAN Bus Off",
    "CAN Rx Queue Overrun",
    "CAN Tx Queue Overrun",
    "CAN PDO length Error",
    "Following Error",
    "Hall Sensor Error",
    "Index Processing Error",
    "Encoder Resolution Error",
    "Hallsensor not found Error",
    "Negative Limit Error",
    "Positive Limit Error",
    "Hall Angle detection Error",
    "Software Position Limit Error",
    "Position Sensor Breach",
    "System Overloaded",
    "Interpolated Position Mode Error",
    "Autotuning Identification Error"
};

void CEpos2::getDigOutFunc(uint16_t& state, uint16_t& mask, uint16_t& polarity)
{
  state = this->readObject(0x2078, 0x01);
  mask = this->readObject(0x2078, 0x02);
  polarity = this->readObject(0x2078, 0x03);
}

uint16_t CEpos2::getDigOutConf(uint8_t index)
{
  return this->readObject(0x2079, index);
}

void CEpos2::setDigOut(uint8_t index, bool state, bool mask, bool polarity)
{
  static uint8_t defaults[ 5 ] = {0xF, 0xE, 0xD, 0xC, 0xB};

  // First write default configuration
  this->writeObject(0x2079, index, defaults[index - 1]);
  
  // Read the functions
  uint16_t _state, _polarity, _mask;
  this->getDigOutFunc(_state, _mask, _polarity);
  
  // Configure the parameters
  if ( state )
    _state |= 0x0001 << (16 - index);
  else
    _state &= !(0x0001 << (16 - index));

  if ( polarity )
    _polarity |= 0x0001 << (16 - index);
  else
    _polarity &= !(0x0001 << (16 - index));

  if ( mask )
    _mask |= 0x0001 << (16 - index);
  else
    _mask &= !(0x0001 << (16 - index));

  // Send parameters to the drive
  this->writeObject(0x2078, 0x03, _polarity);
  this->writeObject(0x2078, 0x02, _mask);
  this->writeObject(0x2078, 0x01, _state);
}
