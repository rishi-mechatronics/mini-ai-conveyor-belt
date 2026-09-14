# Mini AI Conveyor Belt

## Demo

A mini automated conveyor belt that uses computer vision and artificial intelligence to classify objects into **Small**, **Medium**, and **Large** categories. 

The system utilizes an **ESP32-CAM** paired with an **OV3660 camera** to capture real-time images of objects moving along the belt. An AI classification model trained using **Edge Impulse** processes the incoming camera data on the edge to determine the object's size.

---

## Features
* **AI-Based Object Size Classification:** Dynamically classifies objects into predefined scale categories.
* **Smart Vision Pipeline:** Uses an ESP32-CAM with an OV3660 camera for image collection and inference.
* **Edge Computing:** Runs an optimized Edge Impulse machine learning model directly on the microcontroller.
* **Mechatronic Prototyping:** A custom mini conveyor belt system built with integrated automated sorting potential.

---

## Project Structure & Hardware

### Folder Layout
* `/src` — Contains the embedded C++ Arduino camera classification code.
* `/hardware` — Arduino + step motor wiring, motor driver configurations, and [Wiring Diagrams](./hardware).
* `/prototype` — Physical build documentation and [Dataset Collection Photos](./prototype).
* `/ai` — Holds the training visualizations and model snapshots.

### Hardware Components
* **Microcontroller:** ESP32-CAM (AI-Thinker) + ARDUINO UNO
* **Camera Sensor:** OV3660 Camera module
* **Programming Interface:** ESP32-CAM-MB programmer shield
* **Actuation & Drive:** Conveyor belt mechanism, step motor, and motor driver
* **Sorting Mechanism:** AI trained via edge impulse
* **Infrastructure:** Dedicated power supply & laptop

### Software Stack
* **Development Environment:** Arduino IDE
* **Languages:** C++
* **Machine Learning Platform:** Edge Impulse Studio
* **Core Framework:** ESP32 development framework + ESP32-CAM driver library

---

## How It Works
1. **Feeding:** An object is placed onto the physical conveyor belt.
2. **Positioning:** The conveyor moves the object directly into the camera's field of view.
3. **Capture:** The OV3660 camera captures a frame from the conveyor path.
4. **Inference:** The deployed Edge Impulse AI model analyzes the raw frame bytes.
5. **Categorization:** The object is grouped into one of three classes:
   * **Small** (Small-sized object)
   * **Medium** (Medium-sized object)
   * **Large** (Large-sized object)
6. **Action:** The final classification output can be passed to actuators to control a downstream sorting mechanism.

---

## Machine Learning Model & Training Metrics

The vision model was trained and validated using **Edge Impulse Studio**. Below are the dataset profiles and on-device deployment targets:

### Dataset Profile
* **Total Data Collected:** 109 items
* **Dataset Split:** 72% Training / 28% Testing
* **Classes:** `small`, `medium`, `large`
* **Data Source:** Raw images captured at 320x240 pixels directly on the conveyor rig setup.

![Dataset Summary](./ai/dataset.png)

### Model Validation Performance
* **Unoptimized Model (Float32) Accuracy:** 91.30%
* **F1 Scores:** `SMALL`: 1.00 (100% accuracy) | `MEDIUM`: 0.92 | `LARGE`: 0.86

![Training Results](./ai/training.png)

### On-Device Target Performance (ESP32-CAM Profile)
To optimize the network for execution on edge microcontroller hardware, the model was converted into an 8-bit integer quantized layout using the **Edge Impulse EON™ Compiler (RAM optimized)**:
* **Quantized (Int8) Accuracy:** 75.0%
* **Inferencing Time (Latency):** 14 ms (allowing rapid real-time frame rates)
* **Peak RAM Usage:** 228.0 KB
* **Flash Storage Usage:** 545.9 KB

![On-Device Benchmarks](./ai/transfer%20learning%20+%20accuracy.png)

---

## AI Model & Deployment
The machine learning model was developed and optimized using **Edge Impulse** to target tight embedded memory restrictions.

| Class | Description |
| :--- | :--- |
| **Small** | Small-sized object dataset profiles |
| **Medium** | Medium-sized object dataset profiles |
| **Large** | Large-sized object dataset profiles |

---

## Current Development Status
The project is currently a **Prototype in development**. 

* [x] Train AI model (Small / Medium / Large classifications)
* [x] Set up ESP32-CAM & test camera streaming pipelines
* [x] Deploy finalized Edge Impulse AI model to hardware
* [x] Connect live AI classification outputs to the conveyor control logic
* [x] Real time classification
* [ ] Implement automatic robotic sorting hand
* [ ] Assemble the final mechanical enclosure

---

## Project Goal
The ultimate goal of this project is to develop a robust, small-scale demonstration of an AI-powered automated conveyor system. It serves to mirror the complex, computer-vision sorting environments heavily utilized across industrial automation and smart logistics.

---

## Planned improvements include:

* Integrate a robotic arm for automated pick-and-place operations
* Use the AI classification result to determine the robot's sorting destination
* Create separate boxes for Small, Medium, and Large objects
* Synchronise the conveyor, camera, AI classification, and robotic arm
* Improve AI classification accuracy
* Increase conveyor speed and system reliability
* Add object detection and position tracking
* Improve the mechanical design of the conveyor and robotic arm

## Author
**rishi**  
*Mechatronics and Robotics Engineering Student*
