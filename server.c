#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <time.h> // For time() and ctime()
#include <json-c/json.h> // For JSON handling (you might need to install this library)

// Global connection for WebSocket (simplified for example)
static struct mg_connection *s_websocket_conn = NULL;

// Event handler for Mongoose
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_OPEN) {
        // c->is_hexdumping = 1; // Uncomment to enable traffic hexdumping
    } else if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message * ) ev_data;
        if (mg_http_match_uri(hm, "/websocket" )) {
            mg_ws_upgrade(c, hm, NULL); // Upgrade to WebSocket
        } else {
            // Serve static files from the current directory
            struct mg_http_serve_opts opts = {.root_dir = "."};
            mg_http_serve(c, hm, opts );
        }
    } else if (ev == MG_EV_WS_OPEN) {
        s_websocket_conn = c; // Store the WebSocket connection
        mg_printf(c, "{\"status\": \"WebSocket connection established\"}");
        printf("WebSocket client connected!\n");
    } else if (ev == MG_EV_WS_MSG) {
        // struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
        // printf("Received WebSocket message: %.*s\n", (int) wm->data.len, wm->data.ptr);
        // mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT); // Echo received message
    } else if (ev == MG_EV_CLOSE) {
        if (c == s_websocket_conn) {
            s_websocket_conn = NULL;
            printf("WebSocket client disconnected!\n");
        }
    }
    (void) fn_data;
}

// Function to send data over WebSocket
void send_sensor_data_to_websocket(const char *json_data) {
    if (s_websocket_conn) {
        mg_ws_send(s_websocket_conn, json_data, strlen(json_data), WEBSOCKET_OP_TEXT);
    }
}

// This function would be called from your main loop
// In your main.c, you would call this periodically
void mock_send_data() {
    // Example: Create a JSON object with dummy sensor data
    json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "tvoc", json_object_new_int(rand() % 1000));
    json_object_object_add(jobj, "eco2", json_object_new_int(400 + rand() % 1000));
    json_object_object_add(jobj, "mq2", json_object_new_double((double)(rand() % 2000) / 10.0));
    json_object_object_add(jobj, "pitch", json_object_new_double((double)(rand() % 180) - 90.0));
    json_object_object_add(jobj, "status", json_object_new_string("Normal")); // Or "Warning", "Evacuation"

    // Example path data (simplified for demonstration)
    json_object *jpath_nodes = json_object_new_array();
    json_object *node1 = json_object_new_object();
    json_object_object_add(node1, "lat", json_object_new_double(39.9042));
    json_object_object_add(node1, "lng", json_object_new_double(116.4074));
    json_object_object_add(node1, "gas_conc", json_object_new_double(50.0)); // Gas concentration at this node
    json_object_array_add(jpath_nodes, node1);

    json_object *node2 = json_object_new_object();
    json_object_object_add(node2, "lat", json_object_new_double(39.9050));
    json_object_object_add(node2, "lng", json_object_new_double(116.4080));
    json_object_object_add(node2, "gas_conc", json_object_new_double(150.0));
    json_object_array_add(jpath_nodes, node2);

    json_object *node3 = json_object_new_object();
    json_object_object_add(node3, "lat", json_object_new_double(39.9060));
    json_object_object_add(node3, "lng", json_object_new_double(116.4090));
    json_object_object_add(node3, "gas_conc", json_object_new_double(250.0));
    json_object_array_add(jpath_nodes, node3);

    json_object_object_add(jobj, "path", jpath_nodes);

    const char *json_string = json_object_to_json_string(jobj);
    send_sensor_data_to_websocket(json_string);
    json_object_put(jobj); // Free the JSON object
}


int main_mongoose_server() {
    struct mg_mgr mgr; // Mongoose event manager
    mg_mgr_init(&mgr); // Init manager
    // mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL ); // Listen on port 8000
    // For WebSocket, we need to listen on a specific path
    mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL ); // Listen on port 8000 for HTTP and WebSocket upgrade
    printf("Mongoose server started on http://0.0.0.0:8000\n" );

    // In your actual application, you would integrate mg_mgr_poll into your main loop
    // For this example, we'll run it in a loop
    // while (true) {
    //     mg_mgr_poll(&mgr, 1000); // Poll for events, 1000ms timeout
    //     mock_send_data(); // Send mock data periodically
    //     sleep(1); // Simulate sensor reading interval
    // }
    // mg_mgr_free(&mgr); // Free resources
    return 0;
}

