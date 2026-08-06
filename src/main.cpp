#include <print>
#include <unordered_set>

#include "crow.h"
#include "crow/json.h"

#include "CameraManager.h"

#include "Kinect.h"
#include "PoseChecker.h"

#include "KinectManager.h"

// #include "utils.h"

std::unordered_set<crow::websocket::connection *> active_connections;
std::mutex clients_mutex;

int main(int argc, char const *argv[])
{
  CameraManager CamManager;

  CamManager.ConnnectMultiCamera();
  std::this_thread::sleep_for(std::chrono::seconds(2));

  Kinect AzureKinect;
  std::this_thread::sleep_for(std::chrono::seconds(5));
  std::println("Initialization Complete. Moving Kinect Azure to new thread... ");

  KinectManager Manager;

  std::thread kinect_thread(&KinectManager::kinect_pose_worker, &Manager, std::move(AzureKinect), std::ref(CamManager));
  std::thread message_thread(&KinectManager::message_listener, &Manager, std::ref(active_connections), std::ref(CamManager));

  crow::SimpleApp app;

  CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection &conn)
              {
          CROW_LOG_INFO << "New websocket connection: " << &conn;
          std::lock_guard<std::mutex> lock(clients_mutex);
          active_connections.insert(&conn);
          std::println("New Connection"); })
      .onclose([&](crow::websocket::connection &conn,
                   const std::string &reason, uint16_t /*code*/)
               {
          CROW_LOG_INFO << "Websocket disconnected: " << reason;
          std::lock_guard<std::mutex> lock(clients_mutex);
          active_connections.erase(&conn); })
      .onmessage([&](crow::websocket::connection &conn, const std::string &data, bool is_binary)
                 {
                   if (is_binary)
                   {
                     return;
                   }

                   {
                     // trigger shared message
                     std::lock_guard<std::mutex> lock(Manager.msg_mtx);
                     Manager.message_string = data;
                     Manager.new_message = true;
                   }
                   Manager.msg_cv.notify_one();


                 });

  app.port(18080).multithreaded().run();
                 
  kinect_thread.join();
  message_thread.join();
  return 0;
}
