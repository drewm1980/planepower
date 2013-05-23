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

import qualified Carousel.Trajectory as CT
import qualified Carousel.Dae as CD
import qualified Carousel.MheMpcHorizons as MMH

import ParseArgs ( getip )
import Plotter ( runPlotter, newChannel, makeAccessors )

main :: IO ()
main = do
--  ekgTid <- fmap EKG.serverThreadId $ EKG.forkServer "localhost" 8000
  ip <- getip "plot-ho-matic" "tcp://localhost:5563"
  putStrLn $ "using ip \""++ip++"\""

  let zmqChan0 = "carousel trajectory"
      zmqChan1 = "carousel state"
      zmqChan2 = "mhe mpc"
  (c0, write0) <- newChannel zmqChan0 $(makeAccessors ''CT.Trajectory)
  (c1, write1) <- newChannel zmqChan1 $(makeAccessors ''CD.Dae)
  (c2, write2) <- newChannel zmqChan2 $(makeAccessors ''MMH.MheMpcHorizons)
  listenerTid0 <- CC.forkIO (sub ip write0 zmqChan0)
  listenerTid1 <- CC.forkIO (sub ip write1 zmqChan1)
  listenerTid2 <- CC.forkIO (sub ip write2 zmqChan2)
  
  runPlotter [c0,c1,c2] [listenerTid0,listenerTid1,listenerTid2]

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
