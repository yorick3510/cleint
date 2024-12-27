#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <cstring>
#include <thread>
#include <atomic>

#define SERVER_IP "127.0.0.1"
#define PORT 42069
#define BUFFER_SIZE 1024

std::atomic<bool> running(true);

class Client {
public:
    Client(const std::string& name, int port)
        : name(name), port(port), client_fd(-1) {}

    bool connect_to_server() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\n";
            return false;
        }

        client_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_fd == -1) {
            std::cerr << "Cannot create socket\n";
            WSACleanup();
            return false;
        }

        sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

        if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            std::cerr << "Cannot connect to server\n";
            closesocket(client_fd);
            WSACleanup();
            return false;
        }

        std::cout << "Connected to server.\n";
        return true;
    }

    void send_message() {
        char buffer[BUFFER_SIZE];
        while (running) {
            std::cout << "send message: ";
            std::cin.getline(buffer, BUFFER_SIZE);
            if (strcmp(buffer, "exit") == 0) {
                running = false;
                break;
            }
            std::string message = name + ": " + buffer;
            int bytes_sent = send(client_fd, message.c_str(), message.length(), 0);
            if (bytes_sent == -1) {
                std::cerr << "Failed to send message\n";
                break;
            } else {
                std::cout << "Message sent: " << message << "\n";
            }
        }
    }

    void disconnect() {
        closesocket(client_fd);
        WSACleanup();
        std::cout << "Client disconnected.\n";
    }

    std::string get_name() const { return name; }
    int get_port() const { return port; }

private:
    std::string name;
    int port;
    int client_fd;
};

int main() {
    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    Client client(name, PORT);

    if (!client.connect_to_server()) {
        return 1;
    }

    std::thread send_thread(&Client::send_message, &client);
    if (send_thread.joinable()) {
        std::cout << "Thread started successfully.\n";
        send_thread.detach();
    } else {
        std::cerr << "Failed to start thread.\n";
    }

    // Keep the main thread alive to continue sending messages
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    client.disconnect();
    return 0;
}