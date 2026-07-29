#pragma once
#include <UniquePointerDeleter.h>




using unique_device = std::unique_ptr<_k4a_device_t, K4aDeviceDeleter>;
using unique_tracker = std::unique_ptr<_k4abt_tracker_t, K4aTrackerDeleter>;
using unique_capture = std::unique_ptr<_k4a_capture_t, K4aSensorCaptureDeleter>;
using unique_frame = std::unique_ptr<_k4abt_frame_t, K4aBodyFrameDeleter>;


class Kinect
{
private:
    unique_device m_device;
    unique_tracker m_tracker;

    //future concurrency 
    std::condition_variable cv;
    std::mutex mtx;

public:
    Kinect();
    ~Kinect()= default;
};

