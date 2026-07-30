#pragma once
#include "Kinect.h"
#include "PoseChecker.h"
#include <print>
#include <chrono>
#include <thread>
#include <exception>
#include <cmath>
#include <algorithm>
#include <csignal>
#include <atomic>  
#include <iostream>

//Key board Listner
void keyboard_listner();

//Kinect worker
void kinect_worker(Kinect AzureKinect);


