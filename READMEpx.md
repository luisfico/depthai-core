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

the recorded data will be at :

out