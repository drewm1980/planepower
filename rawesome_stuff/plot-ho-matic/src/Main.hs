{-# OPTIONS_GHC -Wall #-}
{-# Language CPP #-}
{-# Language DoAndIfThenElse #-}
{-# Language TemplateHaskell #-}
--{-# OPTIONS_GHC -ddump-splices #-}
--{-# Language OverloadedStrings #-}

module Main ( main ) where

#if OSX
import qualified System.ZMQ3 as ZMQ
#else
import qualified System.ZMQ as ZMQ
#endif
import qualified Control.Concurrent as CC
import Control.Monad ( forever )
import qualified Data.ByteString.Lazy as BL
import qualified Text.ProtocolBuffers as PB

--import qualified System.Remote.Monitoring as EKG

import qualified Sensors as SS
{-import qualified Sensors.Servos as SS-}

import ParseArgs ( getip )
import Plotter ( runPlotter, newChannel, makeAccessors )

main :: IO ()
main = do
--  ekgTid <- fmap EKG.serverThreadId $ EKG.forkServer "localhost" 8000
  ip <- getip "plot-ho-matic" "tcp://localhost:5563"
  putStrLn $ "using ip \""++ip++"\""

  let zmqChan0 = "Sensors"
  (c0, write0) <- newChannel zmqChan0 $(makeAccessors ''SS)
  listenerTid0 <- CC.forkIO (sub ip write0 zmqChan0)
  
  runPlotter [c0] [listenerTid0]

withContext :: (ZMQ.Context -> IO a) -> IO a
#if OSX
withContext = ZMQ.withContext
#else
withContext = ZMQ.withContext 1
#endif

sub :: (PB.Wire a, PB.ReflectDescriptor a) => String -> (a -> IO ()) -> String -> IO ()
sub ip writeChan name = withContext $ \context -> do
#if OSX
  let receive = ZMQ.receive
#else
  let receive = flip ZMQ.receive []
#endif
  ZMQ.withSocket context ZMQ.Sub $ \subscriber -> do
    ZMQ.connect subscriber ip
    ZMQ.subscribe subscriber name
    forever $ do
      _ <- receive subscriber
      mre <- ZMQ.moreToReceive subscriber
      if mre
      then do
        msg <- receive subscriber
        let cs = case PB.messageGet (BL.fromChunks [msg]) of
              Left err -> error err
              Right (cs',_) -> cs'
        writeChan cs
      else return ()
