// Edge Impulse ESP32-CAM + OV3660
// ESP32-CAM MB / AI Thinker style pinout

#include <Conveyor_belt_classification_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"

// =====================================================
// CAMERA MODEL
// =====================================================

// ESP32-CAM MB / AI Thinker
#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else

#error "Camera model not selected"

#endif


// =====================================================
// EDGE IMPULSE CAMERA SETTINGS
// =====================================================

#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3


// =====================================================
// VARIABLES
// =====================================================

static bool debug_nn = false;
static bool is_initialised = false;

uint8_t *snapshot_buf;


// =====================================================
// CAMERA CONFIGURATION
// =====================================================

static camera_config_t camera_config = {

    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,

    .pin_xclk = XCLK_GPIO_NUM,

    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,

    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,

    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,

    .frame_size = FRAMESIZE_QVGA,

    .jpeg_quality = 12,

    .fb_count = 1,

    .fb_location = CAMERA_FB_IN_PSRAM,

    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};


// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

bool ei_camera_init(void);
void ei_camera_deinit(void);

bool ei_camera_capture(
    uint32_t img_width,
    uint32_t img_height,
    uint8_t *out_buf
);

static int ei_camera_get_data(
    size_t offset,
    size_t length,
    float *out_ptr
);


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    while (!Serial);

    Serial.println();
    Serial.println("=================================");
    Serial.println("ESP32-CAM Edge Impulse");
    Serial.println("OV3660 Camera");
    Serial.println("Small / Medium / Large");
    Serial.println("=================================");

    if (ei_camera_init() == false)
    {
        Serial.println("Camera initialization FAILED!");
        return;
    }

    Serial.println("Camera initialized successfully.");

    Serial.println();
    Serial.println("Starting inference in 2 seconds...");

    ei_sleep(2000);
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    if (ei_sleep(5) != EI_IMPULSE_OK)
    {
        return;
    }

    // Allocate image buffer
    snapshot_buf = (uint8_t *)malloc(
        EI_CAMERA_RAW_FRAME_BUFFER_COLS *
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS *
        EI_CAMERA_FRAME_BYTE_SIZE
    );

    if (snapshot_buf == nullptr)
    {
        Serial.println("ERROR: Failed to allocate snapshot buffer!");
        return;
    }


    // Create Edge Impulse signal
    ei::signal_t signal;

    signal.total_length =
        EI_CLASSIFIER_INPUT_WIDTH *
        EI_CLASSIFIER_INPUT_HEIGHT;

    signal.get_data = &ei_camera_get_data;


    // Capture image
    if (
        ei_camera_capture(
            EI_CLASSIFIER_INPUT_WIDTH,
            EI_CLASSIFIER_INPUT_HEIGHT,
            snapshot_buf
        ) == false
    )
    {
        Serial.println("ERROR: Failed to capture image!");

        free(snapshot_buf);

        return;
    }


    // =================================================
    // RUN EDGE IMPULSE CLASSIFIER
    // =================================================

    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR err =
        run_classifier(
            &signal,
            &result,
            debug_nn
        );


    if (err != EI_IMPULSE_OK)
    {
        Serial.printf(
            "ERROR: Classifier failed (%d)\n",
            err
        );

        free(snapshot_buf);

        return;
    }


    // =================================================
    // PRINT RESULTS
    // =================================================

    Serial.println();
    Serial.println("---------------------------------");
    Serial.println("EDGE IMPULSE PREDICTIONS");
    Serial.println("---------------------------------");

    Serial.printf(
        "DSP: %d ms\n",
        result.timing.dsp
    );

    Serial.printf(
        "Classification: %d ms\n",
        result.timing.classification
    );

    Serial.println();


#if EI_CLASSIFIER_OBJECT_DETECTION == 1

    Serial.println("Object Detection:");

    for (
        uint32_t i = 0;
        i < result.bounding_boxes_count;
        i++
    )
    {
        ei_impulse_result_bounding_box_t bb =
            result.bounding_boxes[i];

        if (bb.value == 0)
        {
            continue;
        }

        Serial.printf(
            "%s: %.2f%%\n",
            bb.label,
            bb.value * 100
        );

        Serial.printf(
            "x=%u y=%u width=%u height=%u\n",
            bb.x,
            bb.y,
            bb.width,
            bb.height
        );
    }

#else

    // ================================================
    // CLASSIFICATION
    // ================================================

    float highest_probability = 0.0;
    const char *best_label = "Unknown";

    for (
        uint16_t i = 0;
        i < EI_CLASSIFIER_LABEL_COUNT;
        i++
    )
    {
        float probability =
            result.classification[i].value;

        Serial.printf(
            "%s: %.2f%%\n",
            ei_classifier_inferencing_categories[i],
            probability * 100
        );

        // Find highest prediction
        if (probability > highest_probability)
        {
            highest_probability = probability;

            best_label =
                ei_classifier_inferencing_categories[i];
        }
    }


    // ================================================
    // BEST CLASS
    // ================================================

    Serial.println();

    Serial.print(">>> RESULT: ");
    Serial.println(best_label);

    Serial.print(">>> CONFIDENCE: ");
    Serial.print(highest_probability * 100);
    Serial.println("%");

#endif


#if EI_CLASSIFIER_HAS_ANOMALY

    Serial.print("Anomaly: ");
    Serial.println(result.anomaly);

#endif


    Serial.println("---------------------------------");

    free(snapshot_buf);
}


// =====================================================
// CAMERA INITIALIZATION
// =====================================================

bool ei_camera_init(void)
{
    if (is_initialised)
    {
        return true;
    }


    esp_err_t err =
        esp_camera_init(&camera_config);


    if (err != ESP_OK)
    {
        Serial.printf(
            "Camera init failed with error 0x%x\n",
            err
        );

        return false;
    }


    sensor_t *s =
        esp_camera_sensor_get();


    // OV3660 settings
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);
        s->set_brightness(s, 1);
        s->set_saturation(s, 0);
    }


    is_initialised = true;

    return true;
}


// =====================================================
// CAMERA DEINITIALIZATION
// =====================================================

void ei_camera_deinit(void)
{
    esp_err_t err =
        esp_camera_deinit();

    if (err != ESP_OK)
    {
        Serial.println(
            "Camera deinitialization failed"
        );

        return;
    }

    is_initialised = false;
}


// =====================================================
// CAPTURE IMAGE
// =====================================================

bool ei_camera_capture(
    uint32_t img_width,
    uint32_t img_height,
    uint8_t *out_buf
)
{
    if (!is_initialised)
    {
        Serial.println(
            "ERROR: Camera is not initialized"
        );

        return false;
    }


    camera_fb_t *fb =
        esp_camera_fb_get();


    if (!fb)
    {
        Serial.println(
            "ERROR: Camera capture failed"
        );

        return false;
    }


    // Convert JPEG to RGB888
    bool converted =
        fmt2rgb888(
            fb->buf,
            fb->len,
            PIXFORMAT_JPEG,
            snapshot_buf
        );


    esp_camera_fb_return(fb);


    if (!converted)
    {
        Serial.println(
            "ERROR: JPEG to RGB conversion failed"
        );

        return false;
    }


    // Resize image if necessary
    bool do_resize = false;


    if (
        img_width !=
        EI_CAMERA_RAW_FRAME_BUFFER_COLS ||

        img_height !=
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS
    )
    {
        do_resize = true;
    }


    if (do_resize)
    {
        ei::image::processing::
            crop_and_interpolate_rgb888(

                out_buf,

                EI_CAMERA_RAW_FRAME_BUFFER_COLS,
                EI_CAMERA_RAW_FRAME_BUFFER_ROWS,

                out_buf,

                img_width,
                img_height
            );
    }


    return true;
}


// =====================================================
// EDGE IMPULSE IMAGE DATA
// =====================================================

static int ei_camera_get_data(
    size_t offset,
    size_t length,
    float *out_ptr
)
{
    size_t pixel_ix =
        offset * 3;

    size_t pixels_left =
        length;

    size_t out_ptr_ix = 0;


    while (pixels_left != 0)
    {
        // Convert BGR to RGB

        out_ptr[out_ptr_ix] =
            (
                snapshot_buf[pixel_ix + 2]
                << 16
            )
            +
            (
                snapshot_buf[pixel_ix + 1]
                << 8
            )
            +
            snapshot_buf[pixel_ix];


        out_ptr_ix++;

        pixel_ix += 3;

        pixels_left--;
    }


    return 0;
}


// =====================================================
// CHECK EDGE IMPULSE CAMERA MODEL
// =====================================================

#if !defined(EI_CLASSIFIER_SENSOR) || \
    EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA

#error "Invalid model for current sensor"

#endif
