## Deployment Configuration

The finalized TinyML model was deployed from Edge Impulse Studio directly to the microcontroller environment.

* **Deployment Format:** Exported as a specialized **C++ / Arduino Library**.
* **Target Hardware Platform:** ESP32-CAM (AI-Thinker module) equipped with an OV3660 camera sensor.
* **Optimization Profile:** Quantized (**int8**) neural network to ensure high inference speeds and low RAM consumption on the edge.
* **Inference Pipeline:** Implemented locally inside the firmware (`conveyor_ai.ino`) via the `#include <Conveyor_belt_classification_inferencing.h>` header file to run offline without an internet connection.
