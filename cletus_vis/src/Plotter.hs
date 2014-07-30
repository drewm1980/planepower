{-# OPTIONS_GHC -Wall -fno-warn-orphans #-}
{-# Language DeriveGeneric #-}
{-# Language FlexibleInstances #-}
{-# Language StandaloneDeriving #-}

module Main ( main ) where

import Control.Monad ( forever )
import GHC.Generics ( Generic )
import qualified Text.ProtocolBuffers as PB
--import qualified System.Remote.Monitoring as EKG

import PlotHo -- ( Lookup(..), SignalTree(..), runPlotter, addChannel, makeSignalTree )
--import Channels
import qualified ZmqHelpers as ZMQ

import qualified Protos.ControllerGainsProto.ControllerGainsMsg as Msg
import qualified Protos.ControllerGainsProto as Msg
import qualified Protos.ImuAccelProto.ImuAccelMsg as Msg
import qualified Protos.ImuAccelProto as Msg
import qualified Protos.ImuGyroProto.ImuGyroMsg as Msg
import qualified Protos.ImuGyroProto as Msg
import qualified Protos.ImuMagProto.ImuMagMsg as Msg
import qualified Protos.ImuMagProto as Msg
import qualified Protos.LineAnglesProto.LineAnglesMsg as Msg
import qualified Protos.LineAnglesProto as Msg
import qualified Protos.PIDControllerDebugProto.PIDControllerDebugMsg as Msg
import qualified Protos.PIDControllerDebugProto as Msg
import qualified Protos.PIDControllerGainsProto.PIDControllerGainsMsg as Msg
import qualified Protos.PIDControllerGainsProto as Msg
import qualified Protos.ReferenceProto.ReferenceMsg as Msg
import qualified Protos.ReferenceProto as Msg
import qualified Protos.ResampledMeasurementsProto.ResampledMeasurementsMsg as Msg
import qualified Protos.ResampledMeasurementsProto as Msg
import qualified Protos.SiemensDriveCommandProto.SiemensDriveCommandMsg as Msg
import qualified Protos.SiemensDriveCommandProto as Msg
import qualified Protos.SiemensDriveStateProto.SiemensDriveStateMsg as Msg
import qualified Protos.SiemensDriveStateProto as Msg

deriving instance Generic Msg.ControllerGainsMsg
--deriving instance Generic Msg.ControllerGainsProto
deriving instance Generic Msg.ImuAccelMsg
--deriving instance Generic Msg.ImuAccelProto
deriving instance Generic Msg.ImuGyroMsg
--deriving instance Generic Msg.ImuGyroProto
deriving instance Generic Msg.ImuMagMsg
--deriving instance Generic Msg.ImuMagProto
deriving instance Generic Msg.LineAnglesMsg
--deriving instance Generic Msg.LineAnglesProto
deriving instance Generic Msg.PIDControllerDebugMsg
--deriving instance Generic Msg.PIDControllerDebugProto
deriving instance Generic Msg.PIDControllerGainsMsg
--deriving instance Generic Msg.PIDControllerGainsProto
deriving instance Generic Msg.ReferenceMsg
--deriving instance Generic Msg.ReferenceProto
deriving instance Generic Msg.ResampledMeasurementsMsg
--deriving instance Generic Msg.ResampledMeasurementsProto
deriving instance Generic Msg.SiemensDriveCommandMsg
--deriving instance Generic Msg.SiemensDriveCommandProto
deriving instance Generic Msg.SiemensDriveStateMsg
--deriving instance Generic Msg.SiemensDriveStateProto

instance Lookup Msg.ControllerGainsMsg
--instance Lookup Msg.ControllerGainsProto
instance Lookup Msg.ImuAccelMsg
--instance Lookup Msg.ImuAccelProto
instance Lookup Msg.ImuGyroMsg
--instance Lookup Msg.ImuGyroProto
instance Lookup Msg.ImuMagMsg
--instance Lookup Msg.ImuMagProto
instance Lookup Msg.LineAnglesMsg
--instance Lookup Msg.LineAnglesProto
instance Lookup Msg.PIDControllerDebugMsg
--instance Lookup Msg.PIDControllerDebugProto
instance Lookup Msg.PIDControllerGainsMsg
--instance Lookup Msg.PIDControllerGainsProto
instance Lookup Msg.ReferenceMsg
--instance Lookup Msg.ReferenceProto
instance Lookup Msg.ResampledMeasurementsMsg
--instance Lookup Msg.ResampledMeasurementsProto
instance Lookup Msg.SiemensDriveCommandMsg
--instance Lookup Msg.SiemensDriveCommandProto
instance Lookup Msg.SiemensDriveStateMsg
--instance Lookup Msg.SiemensDriveStateProto


st :: SignalTree Msg.LineAnglesMsg
st = makeSignalTree PB.defaultValue

--st :: SignalTree Msg.EncoderMsg
--st = makeSignalTree PB.defaultValue
--
--st :: SignalTree Msg.LineAngleSensorMsg
--st = makeSignalTree PB.defaultValue
--
--st :: SignalTree Msg.LineAnglesMsg
--st = makeSignalTree PB.defaultValue
--
--st :: SignalTree Msg.McuHandlerMsg
--st = makeSignalTree PB.defaultValue
--
--st :: SignalTree Msg.ResampledMeasurementsMsg
--st = makeSignalTree PB.defaultValue
--
--st :: SignalTree Msg.SiemensDriveCommandMsg
--st = makeSignalTree PB.defaultValue
--
--st :: SignalTree Msg.SiemensDriveStateMsg
--st = makeSignalTree PB.defaultValue
--
--st :: SignalTree Msg.WinchControlMsg
--st = makeSignalTree PB.defaultValue


--deriving instance Generic Msg.ControllerGainsMsg
--instance Lookup Msg.ControllerGainsMsg
--
instance Lookup (PB.Seq PB.Utf8) where
  toAccessorTree _ _ = Data ("Utf8","Utf8") []

lol :: (PB.ReflectDescriptor a, PB.Wire a)
       => ZMQ.Context -> String -> String
       -> (a -> IO ())
       -> (SignalTree a -> IO ())
       -> IO ()
lol context channel messageName newMessage _ =
  ZMQ.withSubscriber context channel messageName $ \receive ->
    forever $ do
      msg <- receive
      newMessage $ case ZMQ.decodeProto msg of
        Left err -> error err
        Right woo -> woo

main :: IO ()
main = ZMQ.withContext 1 $ \ctx -> do
  let url k = "tcp://10.42.0.21:" ++ show (5562 + k :: Int)
      channel k name val = do
        addChannel name (makeSignalTree val) (lol ctx (url k) "")
      
  runPlotter $ do
    channel 0 "Siemens Drive Sensors" (PB.defaultValue :: Msg.SiemensDriveStateMsg)
    channel 1 "Line Angle Measurements" (PB.defaultValue :: Msg.LineAnglesMsg)
    channel 2 "Armbone Gyro data" (PB.defaultValue :: Msg.ImuGyroMsg)
    channel 3 "Armbone Accelerometer data" (PB.defaultValue :: Msg.ImuAccelMsg)
    channel 4 "Armbone Magnetometer data" (PB.defaultValue :: Msg.ImuMagMsg)
    channel 5 "Resampled Measurements" (PB.defaultValue :: Msg.ResampledMeasurementsMsg)
    channel 6 "Commands to the Siemens Drives" (PB.defaultValue :: Msg.SiemensDriveCommandMsg)
    channel 7 "Reference Signal (An angle for closed loop experiments)" (PB.defaultValue :: Msg.ReferenceMsg)
    channel 8 "PID controller debug info" (PB.defaultValue :: Msg.PIDControllerDebugMsg)

--Starting siemensSensorsTelemetry on port tcp://*:5562..
--Starting lineAngleSensor2Telemetry on port tcp://*:5563..
--Starting armboneGyroTelemetry on port tcp://*:5564..
--Starting armboneAccelTelemetry on port tcp://*:5565..
--Starting armboneMagTelemetry on port tcp://*:5566..
--Starting resampledMeasurementsTelemetry on port tcp://*:5567..
--Starting controllerTelemetry on port tcp://*:5568..
--Starting referenceTelemetry on port tcp://*:5569..
--Starting pidDebugTelemetry on port tcp://*:5570..
