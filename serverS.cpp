// Portions of this code were developed with assistance from OpenAI's ChatGPT. 
#include <iostream>
#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

std::map<std::string, int> scienceBooks;

struct BookInfo
{
    char code[4];
    int count;
};

constexpr int SERVER_S_PORT = 41162;

// Portions of this code were developed with assistance from OpenAI's ChatGPT. 
void readBooksFromInput() {
    std::ifstream file("science.txt");
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string code;
        int count;
        if (!(iss >> code >> count)) {
            break;
        }
        // remove the comma
        if (!code.empty() && code.back() == ',') {
            code.pop_back();
        }

        scienceBooks[code] = count;
    }
}

void updateBookInventory(const std::string& code) {
    if (scienceBooks[code] > 0) {
        scienceBooks[code]--;
    }
}

int main() {

    // initialize server
    std::cout << "Server S is up and running using UDP on port " << SERVER_S_PORT <<"." << std::endl;

    // read books from input file
    readBooksFromInput();

    // Phase 3: Forwarding request to Backend Servers
    // create a UDP socket
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        std::cerr << "Error creating UDP socket" << std::endl;
        return 1;
    }

    // server information
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(41162);

    // bind
    if (bind(udp_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(udp_sockfd);
        return 1;
    }

    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[1024];

    // loop to continue listening
    while (true) {
        ssize_t recv_len = recvfrom(udp_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (recv_len < 0) {
            std::cerr << "Error receiving data!" << std::endl;
            continue;
        }
        buffer[recv_len] = '\0';
        std::cout << "Server S received " << buffer << " code from the Main Server." << std::endl;

        std::string response;
        if (scienceBooks.find(buffer) != scienceBooks.end()) {
            // numbrer of books avalible
            int count = scienceBooks[buffer];
            if (count > 0) {
                response = "Number of books " + std::string(buffer) + " available is: " + std::to_string(count);
                updateBookInventory(buffer);
            }
            else {
                response = "The requested book is not available";
            }
        }
        else {
            response = "Not able to find the book";
        }

        // send response back to main server
        ssize_t sent_bytes = sendto(udp_sockfd, response.c_str(), response.length(), 0, (struct sockaddr*)&client_addr, client_addr_len);
        if (sent_bytes < 0) {
            std::cerr << "Error sending availability status to the Main Server" << std::endl;
        } else {
            std::cout << "Server S finished sending the availability status of code " << buffer << " to the Main Server using UDP on port " << ntohs(server_addr.sin_port) << "." << std::endl;
        }

    }

    close(udp_sockfd);

    return 0;
}