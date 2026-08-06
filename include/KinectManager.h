#pragma once
#include "Kinect.h"
#include "PoseChecker.h"
#include "CameraManager.h"
#include <print>
#include <chrono>
#include <thread>
#include <exception>
#include <cmath>
#include <algorithm>
#include <csignal>
#include <atomic>
#include <iostream>
#include <cpr/cpr.h>
#include <unordered_set>
#include "crow.h"

class KinectManager
{
private:
    // Shared Data
    std::atomic<int> correct_pose_count{0};

    // Lock and Conditional Variable for Pose Kinect
    std::condition_variable cv;
    std::mutex k4a_mtx;

    // Shared Data for Kinect Pose Worker
    bool start = false;
    bool Kinect_shutdown = false;
    PoseState POSE_STATE;

    // Client mtx
    std::mutex client_mutex;

    // Message Listener
    std::atomic<bool> keep_running{true};
    
    public:
    KinectManager() = default;
    ~KinectManager() = default;
    
    
    bool new_message = false;
    std::mutex msg_mtx;
    std::condition_variable msg_cv;
    std::string message_string;

    std::string upload_all_image(const crow::json::rvalue &message);
    void kinect_pose_worker(Kinect AzureKinect, CameraManager &camera_Manager);

    void send_message(const std::unordered_set<crow::websocket::connection *> &active_connections, std::string message);

    void message_listener(const std::unordered_set<crow::websocket::connection *> &active_connections, CameraManager& CamManager);
};
