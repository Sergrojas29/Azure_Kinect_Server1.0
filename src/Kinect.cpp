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



std::optional<k4abt_body_t> Kinect::getBodyPosition() {
    // Possible Returns 
    // K4A_WAIT_RESULT_SUCCEEDED (0)
    // K4A_WAIT_RESULT_FAILED (1)
    // K4A_WAIT_RESULT_TIMEOUT (2)

    const int timeout_ms = 0; // 0 timeout means return immediately if no frame is available

    k4a_capture_t raw_capture = nullptr;
    k4a_wait_result_t capture_result = k4a_device_get_capture(m_device.get(), &raw_capture, timeout_ms);
    if (capture_result == K4A_WAIT_RESULT_FAILED) {
        throw std::runtime_error("Failed to get Raw capture pointer");
    }else if(capture_result == K4A_WAIT_RESULT_TIMEOUT){
        return std::nullopt;
    }

    // Create transient smart pointer
    unique_capture capture(raw_capture); 
    k4a_wait_result_t enqueue_result = k4abt_tracker_enqueue_capture(m_tracker.get(), capture.get(), timeout_ms);
    if (enqueue_result == K4A_WAIT_RESULT_FAILED) {
        throw std::runtime_error("Failed Tracker enqueue");
    }else if(enqueue_result == K4A_WAIT_RESULT_TIMEOUT) {
        return std::nullopt; 
    }



    k4abt_frame_t raw_frame = nullptr;
    k4a_wait_result_t pop_result = k4abt_tracker_pop_result(m_tracker.get(), &raw_frame, timeout_ms);
    if (pop_result == K4A_WAIT_RESULT_FAILED) {
        throw std::runtime_error("Failed to pop tracker");
    }else if (pop_result == K4A_WAIT_RESULT_TIMEOUT) {
        return std::nullopt; 
    }
    // Create Unq ptr for the Frame
    unique_frame frame(raw_frame);

    // Check for Body -> Get First and return
    if (k4abt_frame_get_num_bodies(frame.get()) > 0) {
        k4abt_body_t body;
        k4abt_frame_get_body_skeleton(frame.get(), 0, &body.skeleton);
        
        //SUCCESSFUL CAPTURE OF BODY or BODIES
        return body;
    }
    
    //FAIL
    return std::nullopt;
}