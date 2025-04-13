#include <iostream>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <netinet/in.h>
#include <fstream>
#include <sstream>

#include <dirent.h>
#include <iostream>

int listFiles(const char* path, char files[][256], int maxFiles) {
    DIR* dir = opendir(path);
    if (!dir) return 0;

    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr && count < maxFiles) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strncpy(files[count], entry->d_name, 255);
            files[count][255] = '\0'; // ensure null-termination
            count++;
        }
    }
    closedir(dir);
    return count;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

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
    while (true) {
        std::cout << "Waiting for a connection...\n";

        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        read(new_socket, buffer, 30000);
        std::cout << "Received request:\n" << buffer << std::endl;

        std::string request(buffer); // Convert the char array to a std::string

        // Find the first line (request line)
        size_t lineEnd = request.find("\r\n"); // The request line ends with \r\n
        std::string requestLine = request.substr(0, lineEnd);

        // Split the request line by spaces
        size_t firstSpace = requestLine.find(" ");
        size_t secondSpace = requestLine.find(" ", firstSpace + 1);

        std::string method = requestLine.substr(0, firstSpace);
        std::string path = requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        std::string httpVersion = requestLine.substr(secondSpace + 1);

        std::cout << "Method: " << method << std::endl;
        std::cout << "Path: " << path << std::endl;
        std::cout << "HTTP Version: " << httpVersion << std::endl;

        if (method == "GET" ) {
            // Serve index.html
            std::cout << "GET request" << std::endl;

            if(path == "/") {
                std::ifstream file("index.html"); // assumes the file is in the same directory
                if (!file.is_open()) {
                    std::cerr << "Failed to open index.html\n";
                    const char *notFound =
                        "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 13\r\n"
                        "\r\n"
                        "404 Not Found";
                    write(new_socket, notFound, strlen(notFound));
                } else {
                    // Read file into string
                    std::stringstream bufferStream;
                    bufferStream << file.rdbuf();
                    std::string body = bufferStream.str();
                    file.close();

                    // Build full response
                    std::string response =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: " + std::to_string(body.length()) + "\r\n"
                        "\r\n" + body;

                    write(new_socket, response.c_str(), response.length());
                }
            }else{
                std::string filePath = "." + path; // e.g. "/style.css" → "./style.css"
                std::ifstream file(filePath, std::ios::binary);

                if (!file.is_open()) {
                    // File not found, generate directory listing
                    char fileList[8192];// Buffer to store generated HTML for file list
                    fileList[0] = '\0';               // Initialize buffer to an empty string

                    // Begin HTML document with a simple header and unordered list
                    strcat(fileList, "<html><body><h1>Directory Listing</h1><ul>");

                    DIR* dir = opendir(".");// Open current directory
                    if (dir != nullptr) {
                        struct dirent* entry;
                        while ((entry = readdir(dir)) != nullptr) {
                            // Skip "." and ".." entries (current and parent directory)
                            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                                char line[512];// Buffer for one <li> HTML line per file
                                // Create HTML line with clickable link to the file
                                snprintf(line, sizeof(line),
                                         "<li><a href='/%s'>%s</a></li>",
                                         entry->d_name, entry->d_name);
                                strcat(fileList, line);// Append line to fileList
                            }
                        }
                        closedir(dir);// Close the directory after reading
                    }

                    strcat(fileList, "</ul></body></html>"); // Finish the HTML document

                    char response[10000];// Final response buffer (headers + body)
                    snprintf(response, sizeof(response),
                             "HTTP/1.1 404 Not Found\r\n"
                             "Content-Type: text/html\r\n"
                             "Content-Length: %lu\r\n"
                             "\r\n%s",
                             strlen(fileList), fileList); // Compose full HTTP response

                    write(new_socket, response, strlen(response)); // Send the response to the client
                } else {
                    // Read file
                    std::ostringstream ss;
                    ss << file.rdbuf();
                    std::string body = ss.str();
                    file.close();

                    // Determine Content-Type
                    std::string contentType = "text/plain";
                    if (endsWith(path, ".html")) contentType = "text/html";
                    else if (endsWith(path, ".css")) contentType = "text/css";
                    else if (endsWith(path, ".js")) contentType = "application/javascript";
                    else if (endsWith(path, ".json")) contentType = "application/json";
                    else if (endsWith(path, ".png")) contentType = "image/png";
                    else if (endsWith(path, ".jpg") || endsWith(path, ".jpeg")) contentType = "image/jpeg";

                    // Build and send response
                    std::string response =
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: " + contentType + "\r\n"
                        "Content-Length: " + std::to_string(body.size()) + "\r\n"
                        "\r\n" + body;

                    write(new_socket, response.c_str(), response.size());
                }
            }

            close(new_socket);
        } else if (method == "POST") {
            // Parse body and handle submission
            std::cout << "POST request" << std::endl;
        } else if (method == "PUT") {
            // Simulate update
            std::cout << "PUT request" << std::endl;
        } else if (method == "DELETE") {
            // Simulate deletion
            std::cout << "DELETE request" << std::endl;
        } else {
            std::cout << "Unsupported method: " << method << std::endl;

            const char *response_405 =
                "HTTP/1.1 405 Method Not Allowed\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 18\r\n"
                "\r\n"
                "Method Not Allowed";

            write(new_socket, response_405, strlen(response_405));
            close(new_socket);
            continue;
        }

    }
}

