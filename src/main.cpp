#include <print>
#include <unordered_set>

#include "crow.h"
#include "crow/json.h"

#include "CameraManager.h"

#include "Kinect.h"
#include "PoseChecker.h"

#include "utils.h"


std::unordered_set<crow::websocket::connection *> active_connections;
std::mutex clients_mutex;


int main(int argc, char const *argv[])
{
    CameraManager Manager;

    Manager.ConnnectMultiCamera();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    Kinect AzureKinect;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::println("Initialization Complete. Moving Kinect Azure to new thread... ");

    std::thread kinect_thread(kinect_worker, std::move(AzureKinect), std::ref(Manager));

    std::thread keyboard_thread(keyboard_listner);

    crow::SimpleApp app;

    CROW_WEBSOCKET_ROUTE(app, "/ws")
        .onopen([&](crow::websocket::connection &conn) {
          CROW_LOG_INFO << "New websocket connection: " << &conn;
          std::lock_guard<std::mutex> lock(clients_mutex);
          active_connections.insert(&conn);
          std::println("New Connection");
        })
        .onclose([&](crow::websocket::connection &conn,
                     const std::string &reason, uint16_t /*code*/) {
          CROW_LOG_INFO << "Websocket disconnected: " << reason;
          std::lock_guard<std::mutex> lock(clients_mutex);
          active_connections.erase(&conn);
        })
        .onmessage([](crow::websocket::connection &conn, const std::string& data, bool is_binary) {
            
            if (is_binary) {
              return;
            }
            auto message = crow::json::load(data);

            // std::println("{}", messaage.dump());
        });

    app.port(18080).multithreaded().run();  



    // TODO: On message of on "START" begin the distance Calculation
    // TODO: After 

    
    kinect_thread.join();
    keyboard_thread.join();
    return 0;
}
