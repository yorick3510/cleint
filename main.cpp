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

int messageCount = 0;
std::atomic<bool> running(true);

class Message {
public:
    int id;
    char name[50]{};
    char message[250]{};
    char reciever[50]{};
    Message(int i, const char* n, const char* m, const char* r) : id(i) {
        strncpy(name, n, sizeof(name));
        strncpy(message, m, sizeof(message));
        strncpy(reciever, r, sizeof(reciever)); // Corrected this line
    }
};

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

        // Stuur de naam van de client naar de server
        send(client_fd, name.c_str(), name.length(), 0);

        return true;
    }

    void send_message() {
        char buffer[BUFFER_SIZE];
        while (running) {
            std::cout << "Send message: ";
            std::cin.getline(buffer, BUFFER_SIZE);
            if (strcmp(buffer, "exit") == 0) {
                running = false;
                break;
            }
            messageCount++;
            std::string message = buffer;
            std::string reciever;
            std::cout << "Enter name reciever: ";
            std::getline(std::cin, reciever);
            Message msg(messageCount, name.c_str(), message.c_str(), reciever.c_str());
            int bytes_sent = send(client_fd, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
            if (bytes_sent == -1) {
                std::cerr << "Failed to send message\n";
                break;
            } else {
                std::cout << "Message sent: " << message << "\n";
            }
        }
    }

    void receive_message() {
        char buffer[BUFFER_SIZE];
        while (running) {
            int bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                Message msg(0, "", "", "");
                memcpy(&msg, buffer, sizeof(Message));
                std::cout << "Received from " << msg.name << ": " << msg.message << "\n";
            } else if (bytes_received == 0) {
                std::cout << "Server closed connection\n";
                running = false;
                break;
            } else {
                std::cerr << "Receive failed\n";
                running = false;
                break;
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
    std::thread receive_thread(&Client::receive_message, &client);

    if (send_thread.joinable()) {
        send_thread.detach();
    } else {
        std::cerr << "Failed to start send thread.\n";
    }

    if (receive_thread.joinable()) {
        receive_thread.detach();
    } else {
        std::cerr << "Failed to start receive thread.\n";
    }

    // Keep the main thread alive to continue sending and receiving messages
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    client.disconnect();
    return 0;
}