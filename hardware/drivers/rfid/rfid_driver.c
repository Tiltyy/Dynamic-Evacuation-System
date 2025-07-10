#include "rfid_driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Helper function to send command to RFID module
static int rfid_send_command(int fd, uint8_t cmd, const uint8_t *data, int data_len) {
    uint8_t buffer[BUFFER_SIZE];
    int len = 0;

    // Example: Simple command packet structure (adjust as per your module's protocol)
    buffer[len++] = 0xAA; // Start byte
    buffer[len++] = cmd;  // Command byte
    buffer[len++] = data_len; // Data length
    if (data_len > 0 && data != NULL) {
        memcpy(&buffer[len], data, data_len);
        len += data_len;
    }
    // Add checksum or end byte if required by protocol
    buffer[len++] = 0xBB; // End byte (example)

    ssize_t bytes_written = write(fd, buffer, len);
    if (bytes_written != len) {
        fprintf(stderr, "Error writing to RFID reader: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

// Helper function to receive response from RFID module
static int rfid_receive_response(int fd, uint8_t *buffer, int max_len) {
    ssize_t bytes_read = read(fd, buffer, max_len);
    if (bytes_read < 0) {
        fprintf(stderr, "Error reading from RFID reader: %s\n", strerror(errno));
        return -1;
    }
    return bytes_read;
}

int rfid_init(const char *uart_dev_path) {
    int fd = open(uart_dev_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        fprintf(stderr, "Error opening RFID UART device %s: %s (errno: %d)\n", uart_dev_path, strerror(errno), errno);
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Error from tcgetattr: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B9600); // Set baud rate to 9600 (adjust as needed)
    cfsetispeed(&tty, B9600);

    tty.c_cflag |= (CLOCAL | CREAD); // Enable receiver, ignore modem control lines
    tty.c_cflag &= ~CSIZE; // Clear data size bits
    tty.c_cflag |= CS8;    // 8 data bits
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CRTSCTS; // No hardware flow control

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    tty.c_oflag &= ~OPOST; // Raw output

    tty.c_cc[VMIN] = 0;  // Non-blocking read
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        fprintf(stderr, "Error from tcsetattr: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    printf("RFID reader initialized on %s.\n", uart_dev_path);
    return fd;
}

int rfid_read_tag(int fd, char *tag_id, int max_len) {
    uint8_t response_buffer[BUFFER_SIZE];
    int bytes_received;

    // Example: Send command to read tag
    if (rfid_send_command(fd, CMD_READ_TAG, NULL, 0) < 0) {
        return -1;
    }

    // Example: Receive response
    bytes_received = rfid_receive_response(fd, response_buffer, BUFFER_SIZE);
    if (bytes_received < 0) {
        return -1;
    }

    // Example: Parse response (adjust as per your module's response format)
    // Assuming response_buffer contains the tag ID directly after a header
    if (bytes_received > 2 && response_buffer[0] == 0xAA && response_buffer[1] == CMD_READ_TAG) {
        int tag_len = response_buffer[2]; // Assuming byte 2 is length
        if (tag_len > 0 && tag_len < max_len) {
            memcpy(tag_id, &response_buffer[3], tag_len);
            tag_id[tag_len] = '\0'; // Null-terminate the string
            return tag_len;
        } else if (tag_len == 0) {
            return 0; // No tag found or empty tag
        }
    }
    return -1; // Invalid response
}

int rfid_write_tag(int fd, const char *tag_id, const uint8_t *data, int data_len) {
    // This function would involve sending CMD_WRITE_TAG with tag_id and data
    // Implementation depends heavily on the specific RFID module's protocol
    fprintf(stderr, "rfid_write_tag not fully implemented for generic RFID module.\n");
    // Example: Construct data for write command (simplified)
    uint8_t cmd_data[BUFFER_SIZE];
    int cmd_data_len = 0;

    // Copy tag_id (example: first 8 bytes of tag_id)
    int id_len = strlen(tag_id);
    if (id_len > 8) id_len = 8;
    memcpy(&cmd_data[cmd_data_len], tag_id, id_len);
    cmd_data_len += id_len;

    // Copy actual data to write
    if (data_len > 0 && data != NULL) {
        memcpy(&cmd_data[cmd_data_len], data, data_len);
        cmd_data_len += data_len;
    }

    if (rfid_send_command(fd, CMD_WRITE_TAG, cmd_data, cmd_data_len) < 0) {
        return -1;
    }

    uint8_t response_buffer[BUFFER_SIZE];
    int bytes_received = rfid_receive_response(fd, response_buffer, BUFFER_SIZE);
    if (bytes_received < 0) {
        return -1;
    }
    // Parse response for success/failure
    if (bytes_received > 0 && response_buffer[0] == 0xAA && response_buffer[1] == CMD_WRITE_TAG && response_buffer[2] == 0x00) { // Assuming 0x00 for success
        return 0;
    }
    return -1;
}

void rfid_close(int fd) {
    if (fd != -1) {
        close(fd);
        printf("RFID reader closed.\n");
    }
}

// Example usage (for testing purposes)
#ifdef RFID_TEST
int main() {
    int rfid_fd = rfid_init("/dev/ttyUSB0"); // Adjust this to your RFID reader's UART device
    if (rfid_fd < 0) {
        fprintf(stderr, "RFID test failed to initialize.\n");
        return -1;
    }

    char tag_id[64];
    printf("Waiting for RFID tag... (Press Ctrl+C to exit)\n");

    while (1) {
        int len = rfid_read_tag(rfid_fd, tag_id, sizeof(tag_id));
        if (len > 0) {
            printf("Detected Tag ID: %s\n", tag_id);
        } else if (len == 0) {
            // printf("No tag detected.\n");
        } else {
            fprintf(stderr, "Error reading tag.\n");
        }
        usleep(500000); // Check every 500ms
    }

    rfid_close(rfid_fd);
    return 0;
}
#endif // RFID_TEST
