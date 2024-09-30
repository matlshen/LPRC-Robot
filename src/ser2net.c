#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

// Global variables for file descriptors
int serial_fd = -1;
int sockfd = -1;
int client_fd = -1;

// Mutex for synchronizing access to file descriptors
pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    // Set 8N1 mode (8 data bits, no parity, 1 stop bit)
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    // no flow control
    options.c_cflag &= ~CRTSCTS;

    options.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    options.c_oflag &= ~OPOST; // make raw

    // Apply settings
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &options) < 0) {
        perror("Failed to set serial attributes");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Flush input and output buffers
    if (tcflush(fd, TCIOFLUSH) < 0) {
        perror("Failed to flush serial port");
        close(fd);
        exit(EXIT_FAILURE);
    }

    return fd;
}

// Signal handler for SIGINT
void handle_sigint(int sig) {
    (void)sig;  // Avoid unused variable warning

    if (client_fd != -1) close(client_fd);
    if (sockfd != -1) close(sockfd);
    if (serial_fd != -1) close(serial_fd);

    printf("Resources freed. Exiting...\n");
    exit(EXIT_SUCCESS);
}

// Thread function for handling serial port data
void *serial_thread(void *arg) {
    unsigned char buffer[1024];
    while (1) {
        ssize_t n = read(serial_fd, buffer, sizeof(buffer));
        // Print contents of buffer characters
        for (int i = 0; i < n; i++) {
            printf("%c", buffer[i]);
        }
        if (n > 0) {
            pthread_mutex_lock(&fd_mutex);
            ssize_t writen = write(client_fd, buffer, n);
            pthread_mutex_unlock(&fd_mutex);
        } else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Read from serial port failed");
            break;
        }
    }
    return NULL;
}

// Thread function for handling TCP data
void *tcp_thread(void *arg) {
    unsigned char buffer[1024];
    while (1) {
        ssize_t n = read(client_fd, buffer, sizeof(buffer));
        if (n > 0) {
            pthread_mutex_lock(&fd_mutex);
            ssize_t writen = write(serial_fd, buffer, n);
            pthread_mutex_unlock(&fd_mutex);
        } else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Read from client failed");
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s -s <serial_port> -b <baud_rate> -p <tcp_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *serial_port = NULL;
    int baud_rate = 9600;
    int tcp_port = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            serial_port = argv[++i];
        } else if (strcmp(argv[i], "-b") == 0) {
            baud_rate = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0) {
            tcp_port = atoi(argv[++i]);
        }
    }

    if (serial_port == NULL || tcp_port == -1) {
        fprintf(stderr, "Serial port and TCP port are required.\n");
        exit(EXIT_FAILURE);
    }

    // Set up signal handler
    signal(SIGINT, handle_sigint);

    serial_fd = configure_serial(serial_port, baud_rate);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        close(serial_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(tcp_port);

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(serial_fd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(serial_fd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 1) < 0) {
        perror("Listen failed");
        close(serial_fd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening for connections on port %d...\n", tcp_port);

    client_fd = accept(sockfd, NULL, NULL);
    if (client_fd < 0) {
        perror("Accept failed");
        close(serial_fd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Create threads for handling serial and TCP data
    pthread_t serial_tid, tcp_tid;

    if (pthread_create(&serial_tid, NULL, serial_thread, NULL) != 0) {
        perror("Failed to create serial thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&tcp_tid, NULL, tcp_thread, NULL) != 0) {
        perror("Failed to create TCP thread");
        exit(EXIT_FAILURE);
    }

    // Wait for threads to finish
    pthread_join(serial_tid, NULL);
    pthread_join(tcp_tid, NULL);

    // Clean up resources on normal exit
    close(client_fd);
    close(sockfd);
    close(serial_fd);

    return 0;
}
