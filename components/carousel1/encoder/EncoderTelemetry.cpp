/// Base class
#include "TelemetryBase.hpp"
/// Custom data type used by OROCOS
#include "types/EncoderDataType.hpp"
/// Protobuf header, autogenerated
#include "EncoderTelemetry.pb.h"

class EncoderTelemetry
  : public TelemetryBase<EncoderDataType, EncoderProto::EncoderMsg>
{
  typedef TelemetryBase<EncoderDataType, EncoderProto::EncoderMsg> Base;

public:
  EncoderTelemetry(std::string name)
    : Base( name )
  {}

  ~EncoderTelemetry()
  {}

protected:

  virtual void fill();

};

void EncoderTelemetry::fill()
{
  msg.set_theta( data.theta );
  msg.set_sin_theta( data.sin_theta );
  msg.set_cos_theta( data.cos_theta );

  msg.set_omega_filt_rpm( data.omega_filt_rpm );
  msg.set_dbg_speed_voltage( data.dbg_speed_voltage );

  msg.set_ts_trigger( (double)((uint64_t)data.ts_trigger - startTime) * 1e-9);
  msg.set_ts_elapsed( data.ts_elapsed );
}

ORO_CREATE_COMPONENT_LIBRARY()
ORO_LIST_COMPONENT_TYPE( EncoderTelemetry )
