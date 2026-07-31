#include "utils.h"

// Shared Data
std::atomic<int> correct_pose_count{0};

// Lock and Conditional Variable for Pose Kinect
std::condition_variable cv;
std::mutex k4a_mtx;
// Shared Data for Kinect Pose Worker
bool start = false;
bool Kinect_shutdown = false;
PoseState POSE_STATE;

// Not really to important but Kinect Live Distance
std::condition_variable cv_dist;
std::mutex k4a_live_mtx;

// Shared Data for Kinect Distance Worker
bool kinect_live_shutdown = false;
bool kinect_live_start = false;

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
                std::lock_guard<std::mutex> lock(k4a_live_mtx);
                Kinect_shutdown = true;
                kinect_live_shutdown = true;
            }

            cv.notify_one();
            cv_dist.notify_one();
        }
    }
}

// Kinect Pose worker
void kinect_pose_worker(Kinect AzureKinect, CameraManager &camera_Manager)
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

std::string upload_all_image(const std::string &data)
{
    auto message = crow::json::load(data);

    if (message.has("src"))
    {
        std::string APIKEY = "f19e946fe179e9fb2da37e8ec4126157";
        std::string url = "https://api.imgbb.com/1/upload";

        std::vector<std::string> uploadPath;

        for (auto &item : message)
        {
            try
            {

                std::string file_path = item["src"].s();

                cpr::Response res = cpr::Post(
                    cpr::Url{url},
                    cpr::Multipart{
                        {"key", APIKEY},
                        {"image", cpr::File{file_path}}});

                if (res.error || res.status_code != 200)
                {
                    std::cout << "Response text: " << res.text << std::endl;
                    throw std::runtime_error("Error uploading Image");
                }

                auto data = crow::json::load(res.text);
                if (!data)
                {
                    throw std::runtime_error("API did not return valid JSON");
                }

                // Review Response from Img.bb
                // url - full resolution
                // display_url - medium resolution
                if (data["data"].has("display_url") && data["data"].has("url"))
                {
                    std::string url_viewer = data["data"]["display_url"].s();
                    std::string url_full_size = data["data"]["url"].s();

                    // THIS IS FORMATING THE URL WITH "url_viewer"
                    //  (add quotation for parsing in Javascript)
                    url_viewer.insert(0, "\"");
                    url_viewer += "\"";

                    // Add to the list of urls
                    uploadPath.push_back(url_viewer);
                }
                else
                {
                    throw std::runtime_error(
                        "API JSON was missing expected URL keys");
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception caught: " << e.what() << std::endl;
            }
        } // End of For Message loop

        std::string display_Url =
            "https://sergrojas29.github.io/Image_dispaly/?images=[";

        std::string combined_images = "";
        for (size_t i = 0; i < uploadPath.size(); ++i)
        {
            combined_images += uploadPath[i];
            if (i < uploadPath.size() - 1)
            {
                combined_images += ",";
            }
        }

        combined_images += "]";
        // For Debug Logging
        std::println("\n Http url : {}", display_Url);
        return display_Url += combined_images;
    }
    std::println("Error - No 'src' param was found");
    return "https://sergrojas29.github.io/Image_dispaly/";
}

// Kinect Pose worker
void kinect_distance_worker(Kinect &AzureKinect, CameraManager &camera_Manager)
{
    try
    {
        while (true)
        {

            std::println("Kinect Live Distance Worker thread is idel");

            // Block Scope Wait and Consume Trigger
            {
                std::unique_lock<std::mutex> lock(k4a_live_mtx);

                // set Conditional Var and Lamda Predicate
                cv.wait(lock, []
                        { return kinect_live_start || kinect_live_shutdown; });

                if (kinect_live_shutdown)
                {
                    std::println("Kinect Live Distance Worker received shutdown Signal");
                    break;
                }

                //! Reset the lock for wait() to block loop
                kinect_live_start = false;
            }

            // Try to Achive goal
            bool goal_not_achieved = true;
            std::println("Kinect Worker Started to proccess data");

            while (goal_not_achieved)
            {

                // Poll Camera
                auto body = AzureKinect.getBodyPosition();
                
                if (body.has_value())
                {
                    const auto &detected_body = body.value();

                    //! case PoseState::DIST_CHECK:

                    float z_spine_navel = detected_body.skeleton.joints[K4ABT_JOINT_SPINE_NAVEL].position.xyz.z;

                    float tolerance = 10.0;

                    // TODO: REMOVE DEBUG STATEMENT
                    std::println("Distance : {}", z_spine_navel);

                    float goal = 2000.0;

                    // TODO: ADD send message of distance at 30hz

                    if (std::abs(z_spine_navel - goal) < tolerance)
                    {
                        std::println("Distance GOAL REACHED");
                        goal_not_achieved = false;
                    }
                }
                    // Sleep for ~33ms ~30(FPS).
                    std::this_thread::sleep_for(std::chrono::milliseconds(33));
                }

                // TODO: Add message Of success to UI
            }
        }
        catch (const std::runtime_error &e)
        {
            std::println(stderr, "CRITICAL ERROR: {}", e.what());
            return;
        }
    };
