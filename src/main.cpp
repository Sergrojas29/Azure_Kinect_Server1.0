#include <print>
#include <atomic>

#include "Kinect.h"
#include "PoseChecker.h"

#include "utils.h"


int main(int argc, char const *argv[])
{
    
    Kinect AzureKinect;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::println("Initialization Complete. Moving Kinect Azure to new thread... ");

    std::thread kinect_thread(kinect_worker, std::move(AzureKinect));

    std::thread keyboard_thread(keyboard_listner);



    //! Need this on Websocket
    // TODO: On message of on "START" begin the distance Calculation
    // TODO: After 

    kinect_thread.join();
    keyboard_thread.join();

    return 0;
}
