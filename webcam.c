//
// Created by muhammetfaik on 04.10.2023.
//
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create a GStreamer pipeline
    GstElement *pipeline, *source, *encoder, *sink;
    GstCaps *caps;
    GMainLoop *loop;

    pipeline = gst_pipeline_new("webcam-stream");
    source = gst_element_factory_make("v4l2src", "source");
    encoder = gst_element_factory_make("x264enc", "encoder");
    sink = gst_element_factory_make("udpsink", "sink");
    caps = gst_caps_new_simple("video/x-raw",
                               "format", G_TYPE_STRING, "I420",
                               "width", G_TYPE_INT, 640,
                               "height", G_TYPE_INT, 480,
                               "framerate", GST_TYPE_FRACTION, 30, 1, NULL);

    if (!pipeline || !source || !encoder || !sink) {
        g_printerr("Elements could not be created. Exiting.\n");
        return -1;
    }

    // Set source properties (e.g., device, caps)
    g_object_set(G_OBJECT(source), "device", "/dev/video0", NULL);
    g_object_set(G_OBJECT(source), "caps", caps, NULL);

    // Set the destination UDP address and port
    g_object_set(G_OBJECT(sink), "host", "127.0.0.1", "port", 5000, NULL);

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, encoder, sink, NULL);

    // Link elements
    if (gst_element_link_filtered(source, encoder, caps) != TRUE ||
        gst_element_link(encoder, sink) != TRUE) {
        g_printerr("Elements could not be linked. Exiting.\n");
        return -1;
    }

    // Create a GMainLoop for the pipeline
    loop = g_main_loop_new(NULL, FALSE);

    // Set the pipeline to playing
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Start the GMainLoop
    g_main_loop_run(loop);

    // Clean up
    g_main_loop_unref(loop);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}