#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Structure to hold the packet data
struct Packet {
    char symbol[4];
    char buySellIndicator;
    int quantity;
    int price;
    int packetSequence;
};

int main() {
    // Server connection details
    const char* serverIp = "127.0.0.1"; // Replace with the actual server IP address
    int serverPort = 3000;

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error: Could not create socket\n";
        return 1;
    }

    // Server address structure
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp, &(serverAddress.sin_addr));

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error: Could not connect to the server\n";
        close(clientSocket);
        return 1;
    }

    // Prepare and send a request payload (Stream All Packets)
    uint8_t callType = 1;
    send(clientSocket, &callType, sizeof(callType), 0);

    // Receive and process response
    Packet packet;
    while (recv(clientSocket, &packet, sizeof(Packet), 0) > 0) {
        // Process the received packet
        // TODO: Handle packet data as needed, e.g., print or store in data structures
        // For example:
        std::cout << "Symbol: " << packet.symbol << std::endl;
        std::cout << "Buy/Sell Indicator: " << packet.buySellIndicator << std::endl;
        std::cout << "Quantity: " << packet.quantity << std::endl;
        std::cout << "Price: " << packet.price << std::endl;
        std::cout << "Packet Sequence: " << packet.packetSequence << std::endl;
    }

    // Close the connection
    close(clientSocket);

    return 0;
}
