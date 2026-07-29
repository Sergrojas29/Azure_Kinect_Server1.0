#include "Kinect.h"


Kinect::Kinect()
{
    try
    {
        // Open the physical device
    k4a_device_t raw_device = nullptr;
    if (k4a_device_open(K4A_DEVICE_DEFAULT, &raw_device) != K4A_RESULT_SUCCEEDED) {
        throw std::runtime_error("Error: Failed to open Azure Kinect device.");
    }
    
    m_device.reset(raw_device);
    std::println("Camera opened successfully.");

    // Configure and start the camera
    k4a_device_configuration_t device_config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;

    //! should change to wide? 
    //* Currently Narrow Unbinned
    device_config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    //*Color off
    device_config.color_resolution = K4A_COLOR_RESOLUTION_OFF;
    if (k4a_device_start_cameras(m_device.get(), &device_config) != K4A_RESULT_SUCCEEDED) {
        throw std::runtime_error("Error: Failed to start cameras.");
    }


    // Get camera's calibration data
    k4a_calibration_t calibration;
    if (k4a_device_get_calibration(m_device.get(), device_config.depth_mode, device_config.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
        throw std::runtime_error("Error: Failed to get device calibration.");
    }

    // Init the Tracker
    k4abt_tracker_t raw_tracker = nullptr;
    k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
    tracker_config.processing_mode = K4ABT_TRACKER_PROCESSING_MODE_GPU_CUDA; 
    
    if (k4abt_tracker_create(&calibration, tracker_config, &raw_tracker) != K4A_RESULT_SUCCEEDED) {
        throw std::runtime_error("Error: Failed to initialize AI tracker.");
    }
    m_tracker.reset(raw_tracker);  

    }
    catch(const std::exception& e)
    {
        std::println("There was an exception caught : {}" , e);
    }       
}

