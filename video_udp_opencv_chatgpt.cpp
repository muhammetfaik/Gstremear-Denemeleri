//
// Created by muhammetfaik on 09.10.2023.
//
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include <gst/gst.h>

int main() {
    // Initialize GStreamer
    gst_init(nullptr, nullptr);

    // Create a GStreamer pipeline
    GstElement *pipeline = gst_pipeline_new("opencv_gst_pipeline");
    GstElement *appsrc = gst_element_factory_make("appsrc", "source");
    GstElement *x264enc = gst_element_factory_make("x264enc", "video_encoder");
    GstElement *rtph264pay = gst_element_factory_make("rtph264pay", "payloader");
    GstElement *udpsink = gst_element_factory_make("udpsink", "sink");

    if (!pipeline || !appsrc || !x264enc || !rtph264pay || !udpsink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // Set up GStreamer pipeline
    g_object_set(G_OBJECT(udpsink), "host", "127.0.0.1", "port", 5000, nullptr);

    // Link the elements together
    gst_bin_add_many(GST_BIN(pipeline), appsrc, x264enc, rtph264pay, udpsink, nullptr);
    gst_element_link_many(appsrc, x264enc, rtph264pay, udpsink, nullptr);

    // Set up OpenCV capture from camera
    cv::VideoCapture capture(0);
    if (!capture.isOpened()) {
        std::cerr << "Error: Unable to access the camera" << std::endl;
        return -1;
    }

    // Set video properties
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // Initialize OpenCV variables
    cv::Mat frame;

    // Start the GStreamer pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    while (true) {
        // Capture a frame from the camera
        capture >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Frame is empty" << std::endl;
            break;
        }

        // Convert OpenCV frame to GStreamer buffer
        GstBuffer *buffer = gst_buffer_new_wrapped(frame.data, frame.total() * frame.elemSize());

        // Push the buffer to the pipeline
        GstFlowReturn ret;
        g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

        // Check for errors
        if (ret != GST_FLOW_OK) {
            g_printerr("Error pushing buffer to GStreamer pipeline: %s\n", gst_flow_get_name(ret));
            break;
        }

        // Display the frame using OpenCV (optional)
        cv::imshow("Camera", frame);

        // Exit on ESC key press
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    // Release resources
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));

    return 0;
}