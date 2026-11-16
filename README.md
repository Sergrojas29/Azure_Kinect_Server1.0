
## Introduction 
### What is my goal?
`For this term, my goal was to develop an edge AI capstone project that demonstrates my proficiency in hardware-software integration.`

Throughout my time in this program, my interests have moved toward systems engineering and edge computing. I am specifically drawn to fields like autonomous driving, IoT networks, and local AI engineering. These careers all demand a strong understanding of high performance, resource constrained systems. They require building, testing local hardware with software, deploying local AI models, and utilizing concurrent programming.

To put these concepts into practice, I originally planned to build a simple embedded system. However, gaining access to a Microsoft Azure Kinect allowed me to scale my ambition and build a complete Edge AI node. The Kinect features many sensors but most importantly a Time-of-Flight sensor that can run a local ONNX Runtime AI model for body tracking. I decided to use this spatial data, alongside a locally hosted voice recognition model, to trigger and control a multi-camera photography system.

Building this system allowed me to level up my programming skills and apply concepts directly from my reading of Concurrency in Action. The core backend was developed in modern C++, utilizing multithreading and the newly learned (thread, atomic, memory, condition_variable) libraries to optimize performance and prevent hardware lockups. Finally, to ensure the system was user friendly and fully integrated, I built a custom React user interface that communicates with the C++ backend in real time via a WebSocket server. The final deliverable is a localized hardware and software system that successfully combines edge computing, AI, and concurrent system design.
## Background 
`How is this important to you and your professional development?`
I can still remember my first online lesson writing a simple "Hello World" to the console in C++. Since that day, I tested the waters of frontend development by learning JavaScript and React, and then dived much deeper into C++ to master pointers and complex algorithms. Eventually, my coursework at the University of Colorado Boulder led me toward close-to-metal computer systems programming and Natural Language Processing to better understand AI models.
This capstone project is incredibly important to my professional development because it required me to combine all of those separate skills into a single, functional product. It gave me a reason to step outside the standard curriculum and independently learn advanced concepts like multithreading and WebSocket integration. What excites me most about completing this system is the realization that there is still so much more to learn. This project has given me a strong foundation, and it makes me eager to tackle the complex expectations of a professional systems engineering role.
### Vision:

### Methodology, Materials and Methods 
## Methodology:
I used Rapid Application Development (RAD) methodology for this project. As I worked as a solo developer. Creating multiple iterations allowed me to focus on building, and testing smaller prototypes for each section before integrating them into the final build. Because the project spanned hardware control, UI development, and spatial data tracking, this approach helped root out and isolate bugs during testing. 


## Software Roadmap - Project was divided into several repos and developed in phases, over the 13-week term:
 - Weeks 1 – 4  	- 	Research Lidar/AI models
 - Weeks 5 – 10  	-  	Azure Kinect Testing
 - Weeks 6 – 7  	- 	Azure-Graph Tool
 - Week  10 	    -	Crow Websocket Testing
 - Week 11 	        -	Camera Server 2.0
 - Weeks 9–12 	    -	Pose App
 - Weeks 11–13 	    -	Azure Kinect Server 1.0

### Physical Materials
| Material / Component | Description |
|---------------------|-------------|
| Tempo Fit Studio | The main housing for the project. A large freestanding cabinet with a 42-inch touchscreen that contains the internal computer and sensors. |
| Internal Embedded PC | The repurposed computer installed inside the Tempo Studio that runs the entire local server. Specifications: Ubuntu 24.04 LTS, NVIDIA GTX 1050 Ti, 16 GB RAM, Intel Core i5 processor. |
| Microsoft Azure Kinect | A 3D time-of-flight sensor built into the studio. Responsible for native skeletal tracking and depth mapping to determine when a pose is correctly held. |
| Canon Rebel T3 | DSLR camera used for photo capture. Receives commands directly from the embedded PC via libgphoto2 over USB. |
| Canon Rebel T5i | DSLR camera used for photo capture. Receives commands directly from the embedded PC via libgphoto2 over USB. |
| Reference Text | *Concurrency in Action, Second Edition* - Primary technical reference for the multithreaded C++ backend. |





## Results Final Integration

`Currently working on a video Demo: Link to be updated later`

The Minimum Viable Product (MVP), I had to combine all the modules into one cohesive Edge AI node. This required state management and cross thread communication. I implemented a KinectManager class to handle multithreading and flag the pose-checking thread, alongside a CameraManager class to control the hardware cameras. The Crow++ server managed the WebSocket connections and its own internal multithreading.
Because the WebSocket UI sends messages as plain text, the backend was responsible for parsing and formatting both sides of the communication. The server architecture had to perfectly command the Kinect processing loop, control the concurrent camera triggers, track the local photo file paths, execute the cloud uploads, and generate the final delivery URL.
### Server Start & Initialization
Once the server boots, the Azure Kinect is initialized and offloaded to its own dedicated worker thread. Shortly after, the connected DSLR cameras are detected, and a CameraClient object is created for each. These objects are handed over to the CameraManager, which assigns them to separate threads for parallel processing, capturing, and uploading. Finally, the Crow WebSocket server spins up and opens the connection port for the UI. This entire hardware and software initialization process takes approximately 10 to 15 seconds.
### User Start - Thread Unblocking 
Once the connection is established (indicated by the debug bar on the UI), the user can tap the "Start" icon on the touch screen. This action sends a structured JSON message ({"action": "start"}) over the WebSocket. The Crow backend utilizes a built-in JSON library and a custom message handler to parse this command.
This is where the implementation of std::condition_variable (CV), std::mutex, and std::atomic flags becomes critical. Before the user hits start, the Kinect thread is intentionally stalled, waiting for a CV flag. If this lock did not exist, the GPU would continuously process hundreds of frames of Time-of-Flight (ToF) spatial data, spinning up the fans and wasting computational resources. By using a mutex to lock the thread, the system remains highly optimized. Parsing the "start" message triggers the CV, unlocks the atomic boolean, and begins executing the pose-checking loop for the first requested pose.
### Pose Checker (Technical Validation) 
With the thread unblocked, the user steps back to the optimal 1 meter distance and attempts to recreate the pose shown on the UI. 
The C++ backend continuously checks this spatial data against the required pose parameters. It calculates the relative 3D geometry between joints (measuring angles in radians and accounting for the Kinect's negative Y-axis). To prevent false positives from random motion, the pose is only considered "held" once the user satisfies all criteria for five frames.
Upon validation the Kinect thread fires another CV flag specifically for the camera system. The CameraManager instantly uses OpenMP (#pragma omp parallel for) to trigger all connected cameras concurrently. The CameraManager then writes and stores the physical file paths of the resulting photos within the camera class for later use. Afterward, the Kinect thread returns to a blocked, waiting state to save processing power.

### Voice Control & Pose Iterations
At this point the user has three voice activated options processed by the local Teachable Machine model:
- **"Next"**:  Moves to the next pose in the array.
- **"Again"**: Retriggers the camera for the current pose.
- **"Continue"**: Flags the camera thread to retrieve all stored photo file paths, format them into a JSON message, and send them back to the UI.

    *Hardware Limitation: Because the Kinect is physically embedded inside the Tempo Studio cabinet, its 360 microphone array is partially obstructed by the steel frame. While the voice recognition still functions, it requires the user to speak their commands loudly and clearly to register above the hardware constraint. This loop continues until the user issues the "Continue" command.*

### Review Page 
When the "Continue" command is parsed, the CameraManager sends the formatted JSON data containing the photo locations back to the UI. The Review Page populates a grid with these images. The user can step closer to the touch screen, tap on individual photos to enlarge them, verify the pose names, and select their favorite shots for upload. Clicking the on-screen "Continue" button finalizes the selection.








### Social Sharing & Cloud Upload 
The final UI action sends a command containing the selected file paths to the backend, initiating the cloud upload via ImgBB. The UI immediately transitions to a loading state on the Social Page.
Because the upload process is not instantaneous, the C++ server uploads each image individually and parses the JSON response from the ImgBB API. It extracts and saves both the medium and full quality URLs for every selected photo. Once all images are successfully uploaded, the backend concatenates these URLs into a single, parameter string pointing to a custom GitHub Pages web app designed specifically to parse and display those images.
The server sends this final URL string back to the UI via WebSocket. The UI instantly generates and renders a QR code. The user can then scan the screen with their mobile device to view their gallery and download the high-resolution photos directly to their phone.
### Testing Stage:
Summarized- please click link for more detailed results
The testing phase was quite a lot of development. From verifying the spatial math, building multiple custom classes, C++ command line interface to capture pose data and export it as JSON, and a 3D point visualizer in React to map the coordinates. I conducted user testing with family members to refine the visual instructions for the poses. Hardware testing was also a long process. For example, testing revealed that the Canon EDSDK caused hardware zombie states on 64-bit Linux, which required a rapid pivot to the libgphoto2 library to ensure system stability.
## What did you learn? How do you know you have learned it?
I learned how to architect a complex, concurrent application in modern C++ by applying concepts from Concurrency in Action. I learned how to manage shared data across threads using manager classes (like KinectManager) and modern features like std::thread. Furthermore, I learned how to bridge frontend web technologies with backend C++ hardware servers using WebSockets.

I know I have learned it because the final system operates smoothly without blocking the main execution loop. The UI updates instantly based on server messages, the cameras fire without locking up the system, and the application processes high-bandwidth sensor data without crashing. The successful integration of these disparate parts into a working demo is the direct proof of this learning. 
 

## Discussion / Reflection 
### What were your project assessments, and did you reach those goals? 
Yes, I successfully met the main goal of the term: to create a Edge AI capstone project that I can proudly showcase in my portfolio.
This overarching goal was accomplished through the following milestones:
- **Increased Knowledge of C++:** I leveled up my backend programming skills by implementing modern OOP principles, smart pointer encapsulation, and advanced multithreading to make sure a highly optimized application was stable.
- **Edge AI System:** Ultimately, I delivered a local system that handles complex machine learning workloads (e.g. computer vision and voice recognition) at the edge, demonstrating my readiness for professional systems engineering roles.
- **Connecting Hardware**: I successfully connected physical components like a 3D Time-of-Flight sensor, two DSLR cameras, and a 42-inch touchscreen cabinet. To create  a singular and connected user experience.
- **Refinement of Previous Knowledge**: I improved upon my existing C/C++ knowledge from previous projects and development skills. As well as, React and TypeScript to build an interactive, responsive frontend that communicates low-level backend.
- **Creation of Custom Tools**: I had to create custom tools like the command-line data extractor and the azure-graph 3D visualizer to solve specific mathematical and spatial challenges. 
- **Incorporation of SDKs**: I connected and managed  complex third-party libraries, specifically the Azure Kinect SDK K4A_SDK for spatial body tracking and libgphoto2 for multi-camera hardware control.

### How do you feel about the project results? 
I feel accomplished and glad that I pushed through to complete a presentable project. It feels great to see how every new project builds upon the knowledge I have gained over the past 3.5 years. I’ve learned that applying what I read to a project is the best way to reinforce the concepts. 
When I started, I didn’t imagine adding concurrency or a voice model, but they are great additions. I believe this project would have been significantly more complex without the use of either.
## Conclusion 
### Look to the past, the present, and look to the future. How did this confirm/change your outlook? What could you do next? 

There was an initial vision that was formed in my mind at the start of the term which was just the end goal. Throughout the development of each segment of this project there was something I didn’t foresee or a problem that needed a tool or work around.

While I began the term with an initial vision for the end goal, the actual development process came with challenges that constantly required tools or research. From initially setting up the Tempo Fit studio environment to the final integration of all the project segments. There were so many areas that required extensive review to function correctly. I am glad that all the pieces I created to be as portable as possible in class structures and few interdependencies. 

Looking to the past, there are huge portions of this project that would have stalled completely if I could not have drawn on previous coursework. For example, if I hadn't learned the pitfalls of raw pointers while building my K-D tree project in Data Structures 2, I wouldn't have known to learn and use smart pointers to create a memory safe application in my spare time. That knowledge gave me the confidence to reach out to a young local developer here in Austin to create a knowledge exchange. From that I started using Linux development environments and C APIs for raw camera control. Similarly, my Computer Systems class taught me how to debug in assembly, as well as how to handle concurrency and parallelism using worker threads and OpenMP. Looking back, I was only able to manage the complexity of this project because of the culmination of knowledge I've gained over the years. 
My biggest regret however is that I did not implement basic unit testing. Instead, I had to rely on rapid development and manual testing to catch errors. I wish I had taken the time to further develop my test driven development mindset using frameworks like Google Test.
Beyond testing, there are several areas where this project can be improved. I have broken these down into existing issues and additional features for future development.
### Known Issues
- **Zombie Threads**: When combining all project segments, I had to hand off control to the UI. If the UI failed or didn't shut down properly, it would leave either the cameras or the Azure Kinect in a zombie state because the threads were never safely terminated. The fix is clear to me now, but I lacked the time to write and test the necessary code.
- **InDepth Camera Control**: After a weekend of testing, I learned that the Canon EDSDK for 64bit Linux systems lacks some connection stability. Because the SDK is event-driven, the camera needs to continuously send and receive handshakes with the PC. A failure at any point leaves the camera in a zombie state. Without switching my development environment to ARM Linux or macOS, there was no real solution. Due to this I lost the use of autofocus on the Canon T3, as well as PC control over settings like flash and ISO. 
- **Voice Activation**: This was a last-minute addition. Since it used TensorFlow, it integrated with TypeScript and React easily. Although, the issue lies in controlling the flow of activation. I learned that voice control requires specific trigger phrases and confirmation steps to avoid erroneous inputs. For example, if the model was 90% sure it heard the word "Quit" or "Continue" from background noise, it could easily trigger a soft-lock, causing confusion between the user and the server.
- **Refining the Multi-threading Library**: By the end of the project, my use of atomics, condition variables, locks, and mutexes started to become redundant. Given another week, I could clean up this code.
- **Better Encapsulation: I should upgrade the multithreading approach to use std:**:jthread (available in modern C++ 23) rather than older threading standards & created a better, more legible multithreading class with stronger encapsulation to increase the overall portability of my code.
Additional Features for Future Development
- **UI Improvements**: The on-screen instructions could be clearer, and the placeholder photos need updating. Based on feedback, I would also like to add a video tutorial on the UI, implement an alert/countdown before a photo is taken (to prevent people from blinking), and add a burst-shot mode to capture motion poses.
- **Advanced Dynamic Poses**: I didn't want to get ahead of myself during development, so I kept the required poses relatively simple but still more dynamic than a comparative 2D model can actually recognize. With what I know now, I want to add highly dynamic poses that act as "hair triggers" poses that need to be captured within a millisecond. Combining this with burst shooting would significantly increase the non-trivial nature of my project.
- **Sony Camera Integration**: I never got around to incorporating the Sony SDK, mainly due to time and budget constraints. Sony cameras that work with their official SDK are much more expensive, and they do not interface well with the libgphoto2 library I am currently using. Adding dedicated Sony support would be a major future addition. 
- **Inspiration From Classmate's project**: I had been following the progress of one of my classmate Mitchell Avery exploration into OpenGL. During the testing phase of my own tool, I built a efficient data structure using smart pointers, which allowed me to continuously update and rewrite body position data without reallocation at around 30 FPS. Combining live framerate data like this with a graphics engine like OpenGL could lead to some cool, interactive body tracking visualizations. 






## References or Bibliography 

**Azure Kinect Body Tracking SDK**

    Microsoft. (n.d.). Azure Kinect body tracking SDK documentation (Version 1.1.x). https://microsoft.github.io/Azure-Kinect-Body-Tracking/release/1.1.x/index.html

**Azure Kinect DK**

    Microsoft. (n.d.). Azure Kinect DK. https://azure.microsoft.com/en-us/products/kinect-dk

**C++ Concurrency in Action (2nd Edition)**

    Williams, A. (2019). C++ concurrency in action (2nd ed.). Manning Publications.

**Canon EOS Digital SDK (EDSDK)**

    Canon Inc. (n.d.). Canon EOS Digital SDK (EDSDK) [Computer software].

**Crow C++ Microframework**

    CrowCpp. (n.d.). Crow: A fast and easy to use C++ microframework for the web [Computer software]. https://github.com/CrowCpp/Crow

**gPhoto2 / gPhotoLab**

    gPhoto Project. (2026). libgphoto2 (Version 2.5.34) [Computer software]. http://www.gphoto.org/

 

| Feature | Preview |
| :--- | :---: |
| **Dashboard** | ![Dashboard screenshot](https://mamp.one/wp-content/uploads/2024/09/image-resources2.jpg) |
| **Settings** | ![Settings screenshot](https://example.com) |