#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <signal.h>

// Global file descriptor for the serial port
int serial_fd = -1;

// Function to configure the serial port
int configure_serial(const char *serial_port, int baud_rate) {
    int fd = open(serial_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Unable to open serial port");
        exit(EXIT_FAILURE);
    }

    struct termios options;
    if (tcgetattr(fd, &options) < 0) {
        perror("Failed to get serial attributes");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Set baud rate
    switch (baud_rate) {
        case 9600:
            baud_rate = B9600;
            break;
        case 19200:
            baud_rate = B19200;
            break;
        case 38400:
            baud_rate = B38400;
            break;
        case 57600:
            baud_rate = B57600;
            break;
        case 115200:
            baud_rate = B115200;
            break;
        default:
            fprintf(stderr, "Unsupported baud rate\n");
            close(fd);
            exit(EXIT_FAILURE);
    }
    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    // Set 8N1 mode (8 data bits, no parity, 1 stop bit)
    options.c_cflag &= ~PARENB; // No parity
    options.c_cflag &= ~CSTOPB; // 1 stop bit
    options.c_cflag &= ~CSIZE;   // Clear the size bits
    options.c_cflag |= CS8;      // 8 data bits

    options.c_cflag |= CREAD | CLOCAL; // Enable receiver and ignore modem control lines
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    options.c_oflag &= ~OPOST; // Raw output

    // Apply settings
    options.c_cc[VMIN] = 1; // Minimum number of characters to read
    options.c_cc[VTIME] = 0; // Timeout

    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("Failed to set serial attributes");
        close(fd);
        exit(EXIT_FAILURE);
    }

    return fd;
}

// Signal handler for SIGINT
void handle_sigint(int sig) {
    (void)sig;  // Avoid unused variable warning
    if (serial_fd != -1) close(serial_fd);
    printf("Serial port closed. Exiting...\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s -s <serial_port> -b <baud_rate>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serial_port = NULL;
    int baud_rate = 9600; // Default baud rate

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            serial_port = argv[++i];
        } else if (strcmp(argv[i], "-b") == 0) {
            baud_rate = atoi(argv[++i]);
        }
    }

    if (serial_port == NULL) {
        fprintf(stderr, "Serial port is required.\n");
        exit(EXIT_FAILURE);
    }

    // Set up signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    // Configure the serial port
    serial_fd = configure_serial(serial_port, baud_rate);

    unsigned char buffer[1024];
    while (1) {
        ssize_t n = read(serial_fd, buffer, sizeof(buffer));
        if (n > 0) {
            // Print the received data to the screen
            write(STDOUT_FILENO, buffer, n);
        } else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Read from serial port failed");
            break;
        }
    }

    // Clean up before exiting
    close(serial_fd);
    return 0;
}
