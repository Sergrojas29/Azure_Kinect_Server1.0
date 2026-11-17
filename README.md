# Azure Kinect Server 1.0
## Introduction 

This is futher develompent of create a local network API to allow the control of Azure Kinect and Multiple DSLR cameras with the clear goal to create a modular and portable classes for each control. Class are meant to have clear ownership and encapsulation for methods and variables.

The server is unique dive into websocket intergration for a live connection with inter network active commincation. This will create a live API that can communcate between process without polling. Also allow abstraction from the more complex execution from the server to the client. The server connect a concurrent stream of data from the Azure Kinect to allow the local `ONNX` AI model to track any human skeleton with 32 joints. The server can connect to multiple (test up to 5 cameras) dslr cameras to trigger in parallel from any trigger by the API.

## Problems


### Concurrent Runtime
Since there need to be different process running at the same time the need for concurrency is need

**Solution:** Multi-threading to the rescue! Since we need to listen for message, control Kinect and trigger cameras. Each needs to start their own thread to allow for concurrency.

### Syncronization of Communcation

Issue running multithread programe is race conditions, IPC, Deadlock .ect. 

**Solution:** Using mutex locks, conditional variable, and atomic operations. 

### Limited GPU

The nivida gtx 1050 is far from the peak of computing power, with only a very limited vram of 4G.

**Solution** The kinect is set to try to keep a 30fps exectution but that taxing and there are points when all that compute power isn't need. The use of block from `cv` to only retart the computing process when signaled. And Use of a API listener to change an atomic variable when ever allows the process to work in a threadsafe exectuion that allows instant response time from both client and server. 

1. Kinect spins to up to run and waits 
    - status:  `wait`
    - Kinect status:  `blocked`

2. API will listen form the websocket connection to change a atomic variable. This will trigger a conitional variable to restart exectution of the kinect loop
    - status: `ready`
    - Kinect status:  `running`
3. The Kinect will reach a trigger to change the atomic variable again to `wait`. And will be aviable for more work
    - status: `wait`
    - Kinect status:  `blocked`

Another saving use of atomic varaible it to trigger a `shutdown`. In case there is a error that will not allow the kinect loop from running indefinatly or prevent the clean up of memory leading to a memory leak

### C librarys

C is prefect langue... But the C++ allows abrastaction, class encalsulatopnm and modern clean of memory to prevent memory leaks.
But, the two main sdk for the Azure Kinect and gphotolab2 are written in C and in the case of the `K4a` is propriatiy and cannot be changed.

**Solution** Modern C++ allow the creation of unique pointer to allow better ownership or dynamically allocated memory. That automatically deletes to resource when the pointer goes out of scope. Creation of Deleters `std::unique_ptr<T,Deleter>` when initation of a unique pointer. Allow a wrapper and transfer of ownership after the library like `k4a` or `gphotolib2` allocates and initiated the memory raw pointer.

I the case both library require quite a few Deleters. 

| Kinect Deleter | Camera Deleter|
| ---| ---|
|
```C++
struct K4aDeviceDeleter
{
    void operator()(_k4a_device_t* device) const {
        //Check for nullptr (typical for all Deleters)
        if (device) {
            k4a_device_stop_cameras(device);
            k4a_device_close(device);
        }
    }
};

// Helper shut down and destroy the AI tracker
struct K4aTrackerDeleter
{
    void operator()(_k4abt_tracker_t* tracker) const {
        if (tracker) {
            k4abt_tracker_shutdown(tracker);
            k4abt_tracker_destroy(tracker);
        }
    }
};

// release raw sensor captures
struct K4aSensorCaptureDeleter
{
    void operator()(_k4a_capture_t* sensor_capture) const {
        if (sensor_capture) {
            k4a_capture_release(sensor_capture);
        }
    }
};

// release processed body tracking frames
struct K4aBodyFrameDeleter
{
    void operator()(_k4abt_frame_t* body_frame) const {
        if (body_frame) {
            k4abt_frame_release(body_frame);
        }
    }
};
```| sdfsdf |


## Features

## Prerequisites

## Project Structure

## Concurrent System design

## Classes

## Testing

## Project results? 

## Conclusion 



