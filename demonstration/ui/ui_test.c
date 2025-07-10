#include "ui_module.h"
#include "../../software/data_fusion/data_fusion.h"
#include "../../software/path_planning/path_planning.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("UI Module Test Start.\n");

    // Initialize UI module (assuming buzzer GPIO pin 0 for test)
    if (ui_init(0, 0) != 0) { // Assuming I2C bus 0 and buzzer GPIO pin 0 for test
        fprintf(stderr, "Failed to initialize UI module.\n");
        return -1;
    }

    // Create dummy data for testing
    EnvironmentalData_t dummy_env_data = {
        .tvoc_ppb = 100,
        .eco2_ppm = 450,
        .mq2_voltage = 0.5,
        .mq2_concentration = 50.0
    };

    MotionData_t dummy_motion_data = {
        .accel_x_g = 0.1,
        .accel_y_g = 0.2,
        .accel_z_g = 9.8,
        .gyro_x_dps = 1.0,
        .gyro_y_dps = 2.0,
        .gyro_z_dps = 3.0,
        .pitch = 5.0
    };

    // Create a dummy path (using graph_node_t as per path_t definition)
    path_t dummy_path;
    dummy_path.num_nodes = 2;
    // Initialize graph_node_t members explicitly
    dummy_path.nodes[0].node_id = 1;
    dummy_path.nodes[0].area_id = 101;
    dummy_path.nodes[0].x = 0;
    dummy_path.nodes[0].y = 0;

    dummy_path.nodes[1].node_id = 2;
    dummy_path.nodes[1].area_id = 102;
    dummy_path.nodes[1].x = 10;
    dummy_path.nodes[1].y = 0;

    dummy_path.total_distance = 10.0;
    dummy_path.total_risk = 0.1;
    dummy_path.timestamp = 0; // Placeholder

    printf("Updating UI with dummy data...\n");
    // Test ui_update with dummy data and path
    if (ui_update(&dummy_env_data, &dummy_motion_data, &dummy_path) != 0) {
        fprintf(stderr, "Failed to update UI.\n");
    }

    // Test ui_set_page
    printf("Setting UI page to STATUS...\n");
    ui_set_page(UI_PAGE_STATUS);
    sleep(2);

    printf("Setting UI page to MAIN...\n");
    ui_set_page(UI_PAGE_MAIN);
    sleep(2);

    // Test ui_trigger_alert
    printf("Triggering alert...\n");
    ui_trigger_alert(1000); // Trigger alert with duration parameter
    sleep(1); // Alert for 1 second

    printf("Cleaning up UI module.\n");
    ui_cleanup();

    printf("UI Module Test End.\n");
    return 0;
}


