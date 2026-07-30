#include "utils.h"

// Shared Data
std::atomic<int> correct_pose_count{0};
bool start = false;
bool Kinect_shutdown = false;
PoseState POSE_STATE;

// Lock and Conditional Variable
std::condition_variable cv;
std::mutex k4a_mtx;

std::atomic<bool> keep_running{true};

void keyboard_listner()
{
    char input;
    while (keep_running)
    {
        std::cin >> input;
        if (input == '0')
        {
            {
                // trigger for shared state
                std::lock_guard<std::mutex> lock(k4a_mtx);
                POSE_STATE = PoseState::POSE_EMPOWERED;
                start = true;
            }
            cv.notify_one();
        }
        if (input == '1')
        {
            {
                std::lock_guard<std::mutex> lock(k4a_mtx);
                POSE_STATE = PoseState::POSE_MORGAN;
                start = true;
            }
            cv.notify_one();
        }
        if (input == '3')
        {
            {
                std::lock_guard<std::mutex> lock(k4a_mtx);
                POSE_STATE = PoseState::POSE_CUTE_STANDING;
                start = true;
            }
            cv.notify_one();
        }
        if (input == '4')
        {
            {
                std::lock_guard<std::mutex> lock(k4a_mtx);
                POSE_STATE = PoseState::POSE_LEAN_LEFT;
                start = true;
            }
            cv.notify_one();
        }
        if (input == 'd')
        {
            {
                std::lock_guard<std::mutex> lock(k4a_mtx);
                POSE_STATE = PoseState::DIST_CHECK;
                start = true;
            }
            cv.notify_one();
        }
        if (input == 'q')
        {
            keep_running = false;
            {
                std::lock_guard<std::mutex> lock(k4a_mtx);
                Kinect_shutdown = true;
            }

            cv.notify_one();
        }
    }
}

// worker
void kinect_worker(Kinect AzureKinect, CameraManager& camera_Manager)
{
    try
    {
        while (true)
        {

            std::println("Kinect Worker thread is idel");

            // Block Scope Wait and Consume Trigger
            {
                std::unique_lock<std::mutex> lock(k4a_mtx);

                // set Conditional Var and Lamda Predicate
                cv.wait(lock, []
                        { return start || Kinect_shutdown; });

                if (Kinect_shutdown)
                {
                    std::println("Kinect worker received shutdown Signal");
                    break;
                }

                //! Reset the lock for wait() to block loop
                start = false;
            }

            int p_count = 0;
            std::println("Kinect Worker Started to proccess data");

            while (p_count < 5)
            {

                // Poll Camera
                auto body = AzureKinect.getBodyPosition();

                // std::optional check for value
                if (body.has_value())
                {

                    const auto &detected_body = body.value();

                    if (PoseChecker::check(POSE_STATE, detected_body.skeleton))
                    {
                        p_count++;
                    }
                }
                // Sleep for ~33ms ~30(FPS).
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }

            // TODO: Add trigger to camera here also add reference to Camera manager
            //! CameraManager take photos with all cameras
            camera_Manager.MultiCameraCapture();
            std::println("PHOTO WOULD BE TAKEN");

            correct_pose_count.fetch_add(1, std::memory_order_relaxed);
            std::println("Kinect Worker completed a Total of : {}", correct_pose_count.load());
        }
        // TODO REMOVE DEBUG STATEMENT
        std::println("Is finished and has completed Total of : {} correct_pose_count", correct_pose_count.load());
    }
    catch (const std::runtime_error &e)
    {
        std::println(stderr, "CRITICAL ERROR: {}", e.what());
        return;
    }
};