-- Copyright 2012-2013 Greg Horn
--
-- This file is part of rawesome.
--
-- rawesome is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- rawesome is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Lesser General Public License for more details.
--
-- You should have received a copy of the GNU Lesser General Public License
-- along with rawesome.  If not, see <http://www.gnu.org/licenses/>.

{-# OPTIONS_GHC -Wall #-}

module Main ( main ) where

import Data.Foldable ( toList )
import Data.Packed ( fromLists )
import Text.ProtocolBuffers.Basic ( uToString )

import SpatialMath

import MultiCarousel ( State(..), NiceKite(..), runMultiCarousel )
import qualified Carousel.MheMpcHorizons as MMH
import qualified Carousel.Mhe as Mhe
import qualified Carousel.Mpc as Mpc
import qualified Carousel.Sim as Sim
import qualified Carousel.DifferentialStates as CX

toNice :: Maybe Double -> CX.DifferentialStates -> NiceKite
toNice delta0 daeX = NiceKite { nk_xyz = xyz
                              , nk_q'n'b = q'n'b
                              , nk_r'n0'a0 = r'n0'a0
                              , nk_r'n0't0 = r'n0't0
                              , nk_lineAlpha = 1 -- realToFrac $ fromMaybe 1 (CS.lineTransparency cs)
                              , nk_kiteAlpha = 1 -- realToFrac $ fromMaybe 1 (CS.kiteTransparency cs)
                              , nk_visSpan = 0.5 -- fromMaybe 1 (CS.visSpan cs)
                              }
  where
--    daeX = PD.differentialStates dae
--    daeZ = PD.algebraicVars dae
--    daeU = PD.controls dae
--    daeP = PD.parameters dae

    x = CX.x daeX
    y = CX.y daeX
    z = CX.z daeX
    
    e11 = CX.e11 daeX
    e12 = CX.e12 daeX
    e13 = CX.e13 daeX
    
    e21 = CX.e21 daeX
    e22 = CX.e22 daeX
    e23 = CX.e23 daeX
    
    e31 = CX.e31 daeX
    e32 = CX.e32 daeX
    e33 = CX.e33 daeX
    
    delta' = atan2 (CX.sin_delta daeX) (CX.cos_delta daeX)
    delta = case delta0 of Nothing -> delta'
                           Just delta0' -> delta' - delta0'

    q'nwu'ned = Quat 0 1 0 0

    q'n'a = Quat (cos(0.5*delta)) 0 0 (sin(-0.5*delta))

    q'aNWU'bNWU = quatOfDcm $ fromLists [ [e11, e12, e13]
                                        , [e21, e22, e23]
                                        , [e31, e32, e33]
                                        ]
    q'a'b = q'nwu'ned * q'aNWU'bNWU * q'nwu'ned
    q'n'b = q'n'a * q'a'b
    q'n'aNWU = q'n'a * q'nwu'ned

    rArm = Xyz 1.2 0 0 -- CS.rArm
    xyzArm = rArm + Xyz x y z
    xyz = rotVecByQuatB2A q'n'aNWU xyzArm

    zt = 0 --CS.zt cs
    r'n0'a0 = rotVecByQuatB2A q'n'a rArm
    r'n0't0 = xyz + (rotVecByQuatB2A q'n'b $ Xyz 0 0 (-zt))

toState :: MMH.MheMpcHorizons -> State
toState mmh = State (mpckites ++ mhekites ++ simKite) messages
  where
    shiftZ :: Double -> NiceKite -> NiceKite
    shiftZ dz nk = nk {nk_xyz = Xyz 0 0 dz + (nk_xyz nk)}
    messages = map uToString $ toList $ MMH.messages mmh
    mpckites = map (toNice delta0) $ toList $ Mpc.x $ MMH.mpc mmh
    mhekites = map (shiftZ 1 . (toNice delta0)) $ toList $ Mhe.x $ MMH.mhe mmh

    (delta0,simKite) = case MMH.sim mmh of
      Nothing -> (Nothing, [])
      Just sim -> (Just $ atan2 (CX.sin_delta simX) (CX.cos_delta simX), [simKite'])
        where
          simX = Sim.x sim
          simKite' = shiftZ (-1) $ toNice delta0 $ simX

main :: IO ()
main = runMultiCarousel "mhe-mpc" toState
