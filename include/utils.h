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

//receive json message
/** data {
    "action": ACTION,,
    ....
    }
 */

std::string upload_all_image(const std::string& data);

//Key board Listner
void keyboard_listner();

//Kinect Pose worker
void kinect_pose_worker(Kinect AzureKinect, CameraManager& camera_Manager);

//Kincet Distance worker
void kinect_distance_worker();

//Probable another worker
//! need another mutex?
void send_message_worker(const std::unordered_set<crow::websocket::connection *>& active_connections);




