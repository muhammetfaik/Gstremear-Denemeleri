//
// Created by muhammetfaik on 05.10.2023.
//
#include <gst/gst.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GstElement *pipeline, *v4l2src, *videosink, *udpsink,*appsrc;
    GstCaps *caps;
    gchar *descr;

    // Create elements
    pipeline = gst_pipeline_new("webcam-udp-stream");
    appsrc = gst_element_factory_make ("appsrc", "source");

    v4l2src = gst_element_factory_make("v4l2src", "v4l2-source");
    videosink = gst_element_factory_make("autovideosink", "video-sink");
    udpsink = gst_element_factory_make("udpsink", "udp-sink");

    if (!pipeline || !appsrc || !videosink || !udpsink) {
        g_printerr("One or more elements could not be created. Exiting.\n");
        return -1;
    }

    // Create a caps filter to specify video format and resolution
    caps = gst_caps_from_string("video/x-raw, width=640, height=480");
    g_object_set (G_OBJECT (appsrc), "caps",
                  gst_caps_new_simple ("video/x-raw",
                                       "format", G_TYPE_STRING, "RGB16",
                                       "width", G_TYPE_INT, 384,
                                       "height", G_TYPE_INT, 288,
                                       "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                       "framerate", GST_TYPE_FRACTION, 0, 1,
                                       NULL), NULL);
    // Set the UDP sink properties
    g_object_set(G_OBJECT(udpsink), "host", "192.168.1.27", "port", 5000, NULL);

    // Create the pipeline description and add elements
    descr = g_strdup_printf("appsrc ! videoconvert ! videoscale ! %s ! udpsink", gst_caps_to_string(caps));
    gst_bin_add_many(GST_BIN(pipeline), appsrc, videosink, udpsink, NULL);

    // Create and link the elements
    GstElement *pipe = gst_parse_launch(descr, NULL);
    if (!pipe) {
        g_printerr("Pipeline could not be created. Exiting.\n");
        return -1;
    }

    // Set the pipeline to PLAYING state
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Main loop
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // Parse message
    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
                g_clear_error(&err);
                g_free(debug_info);
                break;
            case GST_MESSAGE_EOS:
                g_print("End of stream reached.\n");
                break;
            default:
                // Ignore other messages
                break;
        }

        gst_message_unref(msg);
    }

    // Free resources
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}