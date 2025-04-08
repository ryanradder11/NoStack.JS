#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

int main() {


    // server_fd is like the doorbell — it listens for visitors (clients).
    // When someone rings (connects), we accept() and open the door.
    // The result of accept() is new_socket, which is like starting an actual conversation with that visitor.
    //So server_fd = always listening.
    int server_fd;              // File descriptor for the server socket (listening socket)

    //new_socket = per-client connection where the real data transfer happens.
    int new_socket;             // File descriptor for the client connection socket

    // sockaddr_is not cheating because it's just a wrapper.
    struct sockaddr_in address; // Structure to hold server address info (IPv4)

    int opt = 1;                // Option value for setsockopt (SO_REUSEADDR)
    int addrlen = sizeof(address); // Size of the address struct
    char buffer[30000] = {0};   // Buffer to store incoming request data (zero-initialized);

    //This line creates a “door” for your server — a way for clients to knock and ask to connect.
    //    If it works, you get a handle (server_fd) that lets your program listen for those incoming connections.
    //    If it fails, the program prints an error and stops, because without this door, your server can’t talk to anyone.
    // Create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind the socket to the specified IP address and port.
    // This tells the OS: “I want this socket (server_fd) to listen on this address.”
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Bind failed");     // Print error if binding fails (e.g., port already in use)
        exit(EXIT_FAILURE);        // Exit the program since we can't proceed without a bound socket
    }

    // Mark the socket as passive, ready to accept incoming connections.
    // The number 3 is the backlog: max number of queued connection attempts.
    listen(server_fd, 3);

    std::cout << "Listening on port 8080...\n";

    // Wait (block) until a client connects to the server_fd.
    // accept() creates a new socket (new_socket) for the incoming connection.
    // This is where the actual client/server communication will happen.
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    // Read data sent by the client into the buffer.
    // This will usually be an HTTP request (e.g., GET / HTTP/1.1).
    read(new_socket, buffer, 30000);
    std::cout << "Received request:\n" << buffer << std::endl;

    // Send minimal HTTP response
    const char *response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World";

    write(new_socket, response, strlen(response));
    close(new_socket);
    close(server_fd);

    return 0;
}