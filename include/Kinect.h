#pragma once
#include <optional>
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

public:
    Kinect();
    ~Kinect() = default;

    Kinect(const Kinect &) = delete;
    Kinect &operator=(const Kinect &) = delete;

    Kinect(Kinect &&) = default;
    Kinect &operator=(Kinect &&) = default;

    std::optional<k4abt_body_t> getBodyPosition();
};
