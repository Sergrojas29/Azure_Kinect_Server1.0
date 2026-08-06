#include "KinectManager.h"

// Kinect Pose worker
void KinectManager::kinect_pose_worker(Kinect AzureKinect, CameraManager &camera_Manager)
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
                cv.wait(lock, [this]
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

void KinectManager::message_listener(const std::unordered_set<crow::websocket::connection *> &active_connections, CameraManager &CamManager)
{
    char input;
    auto message = crow::json::load("");

    while (keep_running)
    {
        // Block Scope Wait and Consume Trigger
        {
            std::unique_lock<std::mutex> lock(msg_mtx);

            // set Conditional Var and Lamda Predicate
            msg_cv.wait(lock, [this]
                        { return new_message; });

            message = crow::json::load(message_string);

            new_message = false; //! Reset
        }

        if (message["action"] == "START")
        {
            std::println("Start was sent to the thread and is being handled");
            {
                // trigger for shared state
                std::lock_guard<std::mutex> lock(k4a_mtx);
                POSE_STATE = PoseState::POSE_EMPOWERED;
                start = true;
            }
            cv.notify_one();

            std::println("Pose is set to:{}", std::to_underlying(POSE_STATE));
        }
        if (message["action"] == "NEXT")
        {
            {
                std::lock_guard<std::mutex> lock(k4a_mtx);
                ++POSE_STATE;
                start = true;
            }
            cv.notify_one();

            std::println("New pose is :{}", std::to_underlying(POSE_STATE));
        }
        if (message["action"] == "REVIEW")
        {
            std::println("Sending image paths...");
            std::vector<std::string> paths = CamManager.Get_allImagePaths();

            crow::json::wvalue data;
            data["action"] = "ImagePath";
            data["count"] = paths.size();
            data["image_paths"] = paths;
            send_message(active_connections, data.dump());
        }

        if (message["action"] == "UPLOAD")
        {
            std::println("Sending QR code ...");
            std::string qr_url = upload_all_image(message["payload"]);

            crow::json::wvalue data;
            data["action"] = "QR_CODE";
            data["url"] = qr_url;
            send_message(active_connections, data.dump());
        }

        if (message["action"] == "AGAIN")
        {
            {
                std::lock_guard<std::mutex> lock(k4a_mtx);
                start = true;
            }
            cv.notify_one();
        }

        if (message["action"] == "QUIT")
        {
            keep_running = false;
            {
                std::lock_guard<std::mutex> lock1(k4a_mtx);
                Kinect_shutdown = true;
            }

            cv.notify_one();
            break;
        }
    }
}

void KinectManager::send_message(const std::unordered_set<crow::websocket::connection *> &active_connections, std::string message)
{
    std::lock_guard<std::mutex> lock(client_mutex);
    for (auto &&client : active_connections)
    {
        client->send_text(message);
    }
};

std::string KinectManager::upload_all_image(const crow::json::rvalue &message)
{


    // 1. Check if it's a valid, non-empty list
    if (!message || message.t() != crow::json::type::List || message.size() == 0)
    {
        std::println("Error - Payload is invalid or empty");
        return "https://sergrojas29.github.io/Image_dispaly/";
    }

    // 2. Error out if the first element is NOT an Object, OR if it's missing 'src'
    if (message[0].t() != crow::json::type::Object || !message[0].has("src"))
    {
        std::println("Error - Payload's first element is invalid or missing 'src'");
        return "https://sergrojas29.github.io/Image_dispaly/";
    }

    std::string APIKEY = "f19e946fe179e9fb2da37e8ec4126157";
    std::string url = "https://api.imgbb.com/1/upload";

    std::vector<std::string> uploadPath;

    for (auto &item : message)
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
    }

    std::string display_Url = "https://sergrojas29.github.io/Image_dispaly/?images=[";

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
    display_Url += combined_images;
    std::println("\n Http url : {}", display_Url);
    return display_Url;
}
