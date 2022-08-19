cd depthai-core

git switch main

git submodule update --init --recursive

cmake -S. -Bbuild -D'DEPTHAI_BUILD_EXAMPLES=ON'
cmake --build build -- -j12

FOR DEBUG
cmake -S . -B build -D 'DEPTHAI_BUILD_EXAMPLES=ON' -D 'CMAKE_BUILD_TYPE=Debug' 
cmake --build build -j$(nproc)


or

cmake -S . -B build -D'DEPTHAI_BUILD_EXAMPLES=ON' -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j$(nproc)


run

./build/examples/rgb_depth_aligned 
./build/examples/rgb_depth_alignedPx 

./build/examples/rgb_depth_confidence_aligned    ????
./build/examples/stereo_depth_video      multiple streams


the recorded data will be at :

out






TRY this streams   from  https://docs.luxonis.com/en/latest/pages/tutorials/first_steps/

color

Shows preview from color camera

nnInput

Shows preview from right mono camera

Disabled if no AI model is running

left

Shows preview from left mono camera

OAK-D needed

right

Shows preview from right mono camera

OAK-D needed

depth

Shows disparity map calculated from depthRaw preview and JET colored. Best for visualizing depth

OAK-D needed

depthRaw

Shows raw depth map. Best for depth-based calculations

OAK-D needed

disparity

Shows disparity map produced on device

OAK-D needed

disparityColor

Shows disparity map produced on device and JET colored. Should be the same as depth preview but produced on the device.

OAK-D needed

rectifiedLeft

Rectified left camera frames

OAK-D needed

rectifiedRight






## Px Summary

C++ project
git remote add origin git@github.com:luisfico/depthai-core.git	     
(oak c++ running as example of library) OK get cloud from aligned 4Kimages with depth 400p. Min distance of deteccion=65cm   (Depth aligment with color camera performed by ISP oakd)

C++ project
git remote add origin git@github.com:luisfico/depthai-core-example.git  
(oak c++ isolated project ) ko get correct cloud  

Python project
git remote add origin git@github.com:luisfico/depthai-experiments.git   
(oak python get color ) OK get cloud from aligned stereo images 400p. Min distance of deteccion=35cm (no depth aligment with color camera. TODO)




*******RUN OTHER EXAMPLES

Use monoOAK
    ./build/examples/apriltag_rgb 

Use monoOAKD
    ./build/examples/apriltag       
    ./build/examples/edge_detector  
    ./build/examples/feature_tracker
    ./build/examples/image_manip

Stero problem: depth map low quality (discontunities)
    ./build/examples/depth_crop_control   (crop depth img control roi pose with keyboard)  
    ./build/examples/depth_post_processing  (disparidad la cambia a disparadad colorida)
    ./build/examples/depth_preview    (ERROR no es la depth, sino muestra la DISPARIDAD)         
    ./build/examples/rgb_depth_aligned (tested)
    ./build/examples/rgb_depth_confidence_aligned   ???
    ./build/examples/stereo_depth_video  (show all outputs of steroNode: left,right,leftRect,rightRect,Disp,Depth)



****SOME COMMENTS about nodes

aprilTag        (use output 4 corner marker     potencial use: for the apply solvepnp)
EdgeDetector    (use output edged)              potencial use: modelBasedPose)
FeatureTracker  (mode feature and optical flow) potencial use: vo,sfm,slam?
IMU             (ivo,ivslam,etc)
ImageManip      (roi, rotate, warp, deformation iamge, etc)
ObjectTracker   (reconoce objetos,personas  y sigue su region2d)







Stereodepth   for estimate disparity and depth(try)?
TRY get nodeStereo.DEPTHT   directamente  y  realinearlo a camera color
