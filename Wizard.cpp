#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <map>

struct Packet {
    std::string symbol;
    char buySellIndicator;
    int quantity;
    int price;
    int sequence;
};

class ABXClient {
public:
    ABXClient(const char* serverIp, int port) : serverIp(serverIp), port(port) {}

    void connectToServer() {
        // Create socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            perror("Socket creation failed");
            return;
        }

        // Set up server address
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        if (inet_pton(AF_INET, serverIp, &server.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            return;
        }

        // Connect to server
        if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) == -1) {
            perror("Connection failed");
            return;
        }
    }

    void requestPackets() {
        // Request to stream all packets
        char callType = 1;
        send(sockfd, &callType, 1, 0);

        // Receive and process response packets
        while (true) {
            Packet packet;
            if (recvPacket(packet)) {
                packets[packet.sequence] = packet;
            } else {
                break;
            }
        }

        // Handle missing sequences
        handleMissingSequences();

        // Close connection
        close(sockfd);
    }

    void handleMissingSequences() {
        // Request specific packets that were missed
        for (int i = 1; i <= expectedSequence; ++i) {
            if (packets.find(i) == packets.end()) {
                requestSpecificPacket(i);
            }
        }
    }

    void requestSpecificPacket(int sequence) {
        // Request to resend a specific packet with a given sequence number
        char callType = 2;
        send(sockfd, &callType, 1, 0);
        send(sockfd, &sequence, sizeof(int), 0);

        // Receive and process the requested packet
        Packet packet;
        if (recvPacket(packet)) {
            packets[packet.sequence] = packet;
        }
    }

    void printPackets() {
        // Print received packets
        for (const auto& pair : packets) {
            const Packet& packet = pair.second;
            std::cout << "Symbol: " << packet.symbol
                      << ", Buy/Sell Indicator: " << packet.buySellIndicator
                      << ", Quantity: " << packet.quantity
                      << ", Price: " << packet.price
                      << ", Sequence: " << packet.sequence << std::endl;
        }
    }

private:
    const char* serverIp;
    int port;
    int sockfd;
    struct sockaddr_in server;
    std::map<int, Packet> packets;
    int expectedSequence = 1;

    bool recvPacket(Packet& packet) {
        // Receive and parse a packet from the server
        char buffer[21]; // Packet size is 20 bytes as per the specification
        ssize_t bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);

        if (bytesRead == 20) {
            // Parse received data into Packet structure
            buffer[4] = '\0'; // Null-terminate the symbol string
            packet.symbol = std::string(buffer);
            packet.buySellIndicator = buffer[4];
            packet.quantity = ntohl(*(reinterpret_cast<int*>(buffer + 5)));
            packet.price = ntohl(*(reinterpret_cast<int*>(buffer + 9)));
            packet.sequence = ntohl(*(reinterpret_cast<int*>(buffer + 13)));

            // Update expected sequence number for handling missing sequences
            expectedSequence = packet.sequence + 1;

            return true;
        } else {
            return false;
        }
    }
};

int main() {
    const char* serverIp = "127.0.0.1"; // Replace with the actual server IP address
    int port = 3000; // Port number as per the specification

    ABXClient client(serverIp, port);
    client.connectToServer();
    client.requestPackets();
    client.printPackets();

    return 0;
}