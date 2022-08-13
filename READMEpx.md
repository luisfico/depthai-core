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

./build/examples/rgb_depth_confidence_aligned 

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