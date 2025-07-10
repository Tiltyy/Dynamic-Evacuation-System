#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include <stdint.h>

// Define buffer size for RFID communication
#define BUFFER_SIZE 256

// RFID Commands (example values, adjust as per your RFID module's datasheet)
#define CMD_READ_TAG        0x01
#define CMD_WRITE_TAG       0x02
#define CMD_GET_VERSION     0x03
#define CMD_SET_ANTENNA     0x04

/**
 * @brief Initializes the RFID reader.
 * @param uart_dev_path Path to the UART device (e.g., "/dev/ttyUSB0").
 * @return File descriptor if successful, -1 otherwise.
 */
int rfid_init(const char *uart_dev_path);

/**
 * @brief Reads an RFID tag.
 * @param fd File descriptor of the RFID reader.
 * @param tag_id Buffer to store the tag ID.
 * @param max_len Maximum length of the tag_id buffer.
 * @return Length of the tag ID if successful, -1 on error, 0 if no tag found.
 */
int rfid_read_tag(int fd, char *tag_id, int max_len);

/**
 * @brief Writes data to an RFID tag.
 * @param fd File descriptor of the RFID reader.
 * @param tag_id Tag ID to write to.
 * @param data Data to write.
 * @param data_len Length of the data.
 * @return 0 if successful, -1 otherwise.
 */
int rfid_write_tag(int fd, const char *tag_id, const uint8_t *data, int data_len);

/**
 * @brief Closes the RFID reader.
 * @param fd File descriptor of the RFID reader.
 */
void rfid_close(int fd);

#endif // RFID_DRIVER_H
