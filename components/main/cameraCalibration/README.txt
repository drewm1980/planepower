These are some experimental scripts for camera calibration based
on Casadi.

Update: This has turned into a growing collection of numpy/casadi
scripts for camera related geometry, including generating some code
we use on the carousel.

The "markers" and "cameras" links should point to directories containing
calibration data that is loaded by a number of the modules.

The scripts:

markers_from_pose.py
pose_from_markers.py

are of special importance, as they generate code used by the carousel
for simulation and control, respectively.

Author: Andrew Wagner
