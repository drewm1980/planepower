#!/usr/bin/env bash
names="DifferentialStates AlgebraicVars Controls Parameters Outputs MeasurementsX MeasurementsU Dae Trajectory Debug MheMpcHorizons Mhe Mpc Sim"
#ln -sf ../Carousel/Carousel.hs CarouselRealFiles/Carousel.hs
ln -sf CarouselRealFiles/Carousel.hs Carousel.hs
mkdir -p Carousel
for x in $names;
do
	#echo $x
	#mkdir -p Carousel/$x
	(
	cd Carousel
	ln -sf ../CarouselRealFiles/$x.hs $x.hs
	)
done
