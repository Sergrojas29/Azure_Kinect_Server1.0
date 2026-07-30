#include <print>
#include <Kinect.h>

class PoseChecker{
    public:
    //Public Static Class
    PoseChecker() = delete;

    static check( std::string name_Pose ,const k4abt_body_t& body ){
        return true;
    }

}


void mainLoop(Kinect AzureKinect , std::string name_Pose){
    try{
        while (true)
            {
                // Poll Camera
                auto body = AzureKinect.getBodyPosition();

                // std::optional check for value
                if (body.has_value())
                {

                    const auto &detected_body = body.value();
                    return PoseChecker::check(name_Pose, detected_body.skeleton)                
                }

                // Sleep for ~33ms ~30(FPS).
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }
            return;
        }catch (const std::runtime_error &e){
            std::println("CRITICAL ERROR: {}", e.what());
            return;
        }

}






int main(int argc, char const *argv[])
{
    Kinect AzureKinect();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::println("Initialization Complete. Moving Kinect Azure to new thread... ");
    std::thread kinect_thread(mainLoop, std::move(AzureKinect));




}
