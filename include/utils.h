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

//receive json message
/** data {
    "action": ACTION,,
    ....
    }
 */

std::string upload_all_image(const std::vector<std::string>& data);

//Key board Listner
void keyboard_listner();

//Kinect worker
void kinect_worker(Kinect AzureKinect, CameraManager& camera_Manager);

//Kincet Distance worker
void kinect_distance_worker();

void send_message(const std::unordered_set<crow::websocket::connection *>& active_connections);




