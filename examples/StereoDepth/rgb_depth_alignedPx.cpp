/*
BUILD:cmake -S . -B build -D 'DEPTHAI_BUILD_EXAMPLES=ON' -D 'CMAKE_BUILD_TYPE=Debug'
cmake --build build -j$(nproc)
RUN: ./build/examples/rgb_depth_alignedPx
TRY to get  "depth" or "depthRaw"

TODO: to estimate  factorFix, and test more subpixel
*/

#include <cstdio>
#include <iostream>

#include "utility.hpp"

// Includes common necessary includes for development using depthai library
#include <fstream>

#include "depthai/depthai.hpp"

// Optional. If set (true), the ColorCamera is downscaled from 1080p to 720p.
// Otherwise (false), the aligned depth is automatically upscaled to 1080p
static std::atomic<bool> downscaleColor{false};  // true
static constexpr int fps = 15;//30;
// The disparity is computed at this resolution, then upscaled to RGB resolution
static constexpr auto monoRes = dai::MonoCameraProperties::SensorResolution::THE_400_P;  // THE_720_P

static float rgbWeight = 0.4f;
static float depthWeight = 0.6f;

static void updateBlendWeights(int percentRgb, void* ctx) {
    rgbWeight = float(percentRgb) / 100.f;
    depthWeight = 1.f - rgbWeight;
}



int main() {
    std::ofstream file;
    //file.open("cloudDepth.csv");
    //file.open("cloudDepth.csv", std::fstream::in | std::fstream::out | std::fstream::app);
    //file << "//X;Y;Z;B;G;R\n";

    using namespace std;

    // Create pipeline
    dai::Pipeline pipeline;
    dai::Device device;
    std::vector<std::string> queueNames;

    // Define sources and outputs
    auto camRgb = pipeline.create<dai::node::ColorCamera>();
    auto left = pipeline.create<dai::node::MonoCamera>();
    auto right = pipeline.create<dai::node::MonoCamera>();
    auto stereo = pipeline.create<dai::node::StereoDepth>();

    auto rgbOut = pipeline.create<dai::node::XLinkOut>();
    auto depthOut = pipeline.create<dai::node::XLinkOut>();
    
    rgbOut->setStreamName("rgb");
    queueNames.push_back("rgb");
    depthOut->setStreamName("depth");
    queueNames.push_back("depth");

    // Properties
    camRgb->setBoardSocket(dai::CameraBoardSocket::RGB);
    //camRgb->setResolution(dai::ColorCameraProperties::SensorResolution::THE_1080_P);  // color THE_1080_P
    camRgb->setResolution(dai::ColorCameraProperties::SensorResolution::THE_4_K);  // color THE_1080_P
    camRgb->setFps(fps);
    if(downscaleColor) camRgb->setIspScale(2, 3);
    // For now, RGB needs fixed focus to properly align with depth.
    // This value was used during calibration
    try {
        auto calibData = device.readCalibration2();
        auto lensPosition = calibData.getLensPosition(dai::CameraBoardSocket::RGB);
        if(lensPosition) {
            camRgb->initialControl.setManualFocus(lensPosition);
        }
    } catch(const std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 1;
    }

    left->setResolution(monoRes);
    left->setBoardSocket(dai::CameraBoardSocket::LEFT);
    left->setFps(fps);
    right->setResolution(monoRes);
    right->setBoardSocket(dai::CameraBoardSocket::RIGHT);
    right->setFps(fps);


// Create a node that will produce the depth map (using disparity output as it's easier to visualize depth this way)
    stereo->setDefaultProfilePreset(dai::node::StereoDepth::PresetMode::HIGH_ACCURACY);  // HIGH_ACCURACY,HIGH_DENSITY
    // Options: MEDIAN_OFF, KERNEL_3x3, KERNEL_5x5, KERNEL_7x7 (default)
    // stereo->initialConfig.setMedianFilter(dai::MedianFilter::KERNEL_7x7);    
    stereo->setLeftRightCheck(true); // LR-check is required for depth alignment  // Better handling for occlusions:
    stereo->setExtendedDisparity(false); // Closer-in minimum depth, disparity range is doubled (from 95 to 190):
    stereo->setSubpixel(true);// Better accuracy for longer distance, fractional disparity 32-levels:
    stereo->setDepthAlign(dai::CameraBoardSocket::RGB);

    // Linking
    camRgb->isp.link(rgbOut->input);
    left->out.link(stereo->left);
    right->out.link(stereo->right);
    stereo->disparity.link(depthOut->input);

    // Connect to device and start pipeline
    device.startPipeline(pipeline);

    // Sets queues size and behavior
    for(const auto& name : queueNames) {
        device.getOutputQueue(name, 4, false);
    }

    std::unordered_map<std::string, cv::Mat> frame;

    auto rgbWindowName = "rgb";
    auto depthWindowName = "depth";
    //auto blendedWindowName = "rgb-depth";
    cv::namedWindow(rgbWindowName);
    cv::namedWindow(depthWindowName);
    //cv::namedWindow(blendedWindowName);
    int defaultValue = (int)(rgbWeight * 100);
    //cv::createTrackbar("RGB Weight %", blendedWindowName, &defaultValue, 100, updateBlendWeights);

    int ite = 0;
    while(true) {
        std::unordered_map<std::string, std::shared_ptr<dai::ImgFrame>> latestPacket;

        auto queueEvents = device.getQueueEvents(queueNames);
        for(const auto& name : queueEvents) {
            auto packets = device.getOutputQueue(name)->tryGetAll<dai::ImgFrame>();
            auto count = packets.size();
            if(count > 0) {
                latestPacket[name] = packets[count - 1];
            }
        }

        for(const auto& name : queueNames) {
            if(latestPacket.find(name) != latestPacket.end()) {
                //-------- Disparity frame aligned to color frame
                if(name == depthWindowName) {
                    frame[name] = latestPacket[name]->getFrame();

                    auto maxDisparity = stereo->initialConfig.getMaxDisparity();
                    // Optional, extend range 0..95 -> 0..255, for a better visualisation
                    if(1) frame[name].convertTo(frame[name], CV_8UC1, 255. / maxDisparity);  // rescale distarity values to <0,255>

                    // Optional, apply false colorization
                    // if(1) cv::applyColorMap(frame[name], frame[name], cv::COLORMAP_HOT);
                    cv::imwrite("/home/lc/env/oakd/codeCpp/out/" + std::to_string(ite) + "disparityAligned.png", frame[name]);

                } else {
                    //-------- olor frame
                    frame[name] = latestPacket[name]->getCvFrame();
                    cv::imwrite("/home/lc/env/oakd/codeCpp/out/" + std::to_string(ite) + "color.png", frame[name]);
                }

                cv::imshow(name, frame[name]);
                // cv::imwrite("/home/lc/env/oakd/codeCpp/out/"+std::to_string(ite)+name+".png",frame[name]);
            }
        }

        // Blend when both received
        if(frame.find(rgbWindowName) != frame.end() && frame.find(depthWindowName) != frame.end()) {
            
            /*
            // Need to have both frames in BGR format before blending
            if(frame[depthWindowName].channels() < 3) {
                cv::cvtColor(frame[depthWindowName], frame[depthWindowName], cv::COLOR_GRAY2BGR);
            }
            cv::Mat blended;
            cv::addWeighted(frame[rgbWindowName], rgbWeight, frame[depthWindowName], depthWeight, 0, blended);
            cv::imshow(blendedWindowName, blended);
            */

            //-----Generation pointcloud with respect to left stereo frame-----------ini
            file.open("/home/lc/env/oakd/codeCpp/out/" + std::to_string(ite) + "cloud.csv");
            file << "//X;Y;Z;B;G;R\n";

            cv::Mat img_rgb = frame["rgb"].clone();
            cv::Mat img_disparity = frame["depth"].clone();
            //cv::cvtColor(img_disparity, img_disparity, cv::COLOR_BGR2GRAY);
            //auto maxDisparity = stereo->initialConfig.getMaxDisparity();
            //remember depth=f*b/disp  :   depthMin=f*b/dispMax ,  dispMax=95, depth400P=45cm =  570*7.5cm /95     -> en practica + de 60cm

            double fx = 3090.419189, fy = fx, cx = 1953.194824, cy = 1068.689209; //  COLOR 4K color = 3840x2160 is (1280×720   x3times)  
          //double fx = 2366.810547, fy = fx, cx = 1980.788452, cy = 1073.155884;  // RIGHT 4K Intrinsics from getCameraIntrinsics function 4K 3840x2160 (RIGHT):   4K = 3840x2160 is (1280×720   x3times)
            double factorFix = 7.24519034;         //0.4   1         // 720/400; //720/400; // 1080/720      0.4; //1000; //0.4;  // upscale   THE_400_P to THE_720_P
            //double baselineStereo = 0.075;  // Stereo baseline distance: 7.5 cm
            for(int v = 0; v < img_disparity.rows; v++) {
                for(int u = 0; u < img_disparity.cols; u++) {
                  //double disparityD = img_disparity.ptr<uchar>(v)[u];  // ok!!   disparityD value is scaled by 16bits? so   real disparityD= disparityD/16bits ?
                  //double disparityD = img_disparity.ptr<CV_8UC1>(v)[u]*maxDisparity/255.0;  // ok!!   disparityD value is scaled by 16bits? so   real disparityD= disparityD/16bits ?
                    double disparityD = static_cast<double>(img_disparity.ptr<int8_t>(v)[u]);  // ok!!   disparityD value is scaled by 16bits? so   real disparityD= disparityD/16bits ?
                    
                    // double disparityD = disparity.ptr<uint8_t>(v)[u]; //ok!!   disparityD value is scaled by 16bits? so   real disparityD=
                    // disparityD/16bits ? double disparityD = disparity.ptr<uint16_t>(v)[u]; double disparityD = disparity.ptr<uint32_t>(v)[u]; uchar
                    // dispValue = disparity.ptr<uchar>(v)[u];  double disparityD = static_cast<double>(dispValue);
                    if(disparityD <= 0.0) continue;

                    double xNorm = (u - cx) / fx;                                 // x normalizado
                    double yNorm = (v - cy) / fy;                                 // y normalizado
                    double depth = factorFix*1 / (disparityD);  // ok depth=z real = scala w
                    
                    double xP = xNorm * depth;  // x normalizado se escala y se recupera x real
                    double yP = yNorm * depth;
                    double zP = depth;

                    int colorB = img_rgb.data[v * img_rgb.step + u * img_rgb.channels()];     // blue
                    int colorG = img_rgb.data[v * img_rgb.step + u * img_rgb.channels() + 1]; // green
                    int colorR = img_rgb.data[v * img_rgb.step + u * img_rgb.channels() + 2];
                    
                    file << xP << ";" << yP << ";" << zP << ";" << colorB << ";" << colorG << ";" << colorR<< "\n";
                }
            }
            file.close();
            //-----Generation pointcloud-----------end

            frame.clear();
        }

        int key = cv::waitKey(1);
        if(key == 'q' || key == 'Q') {
            return 0;
        }
        ite++;
    }
    return 0;
}
