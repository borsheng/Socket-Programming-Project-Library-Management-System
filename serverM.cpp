// Portions of this code were developed with assistance from OpenAI's ChatGPT. 
#include <iostream>
#include <map>
#include <string>
#include <cstring>
#include <set>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// structure for book information
struct BookInfo
{
    char code[4];
    int count;
};

// define ports for backend servers
constexpr int SERVER_S_PORT = 41162;
constexpr int SERVER_L_PORT = 42162;
constexpr int SERVER_H_PORT = 43162;
constexpr int SERVER_M_PORT = 44162;

// member list file
std::map<std::string, std::string> memberList;

// Portions of this code were developed with assistance from OpenAI's ChatGPT. 
void loadMemberList() {
    std::ifstream file("member.txt");
    std::string line, username, password;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (std::getline(iss, username, ',') && std::getline(iss, password)) {
            // remove space infront of password
            password.erase(0, password.find_first_not_of(" "));
            // remove any trailing newline characters ('\n' or '\r')
            password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());
            password.erase(std::remove(password.begin(), password.end(), '\r'), password.end());

            memberList[username] = password;
        }
    }
}

// Portions of this code were developed with assistance from OpenAI's ChatGPT. 
std::string sendBookCodeBackend(const std::string& bookCode, int destPort) {
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        std::cerr << "Error creating UDP socket" << std::endl;
        return "";
    }

    // main server information
    struct sockaddr_in serverM_udp_addr;
    memset(&serverM_udp_addr, 0, sizeof(serverM_udp_addr));
    serverM_udp_addr.sin_family = AF_INET;
    serverM_udp_addr.sin_port = htons(SERVER_M_PORT);
    serverM_udp_addr.sin_addr.s_addr = INADDR_ANY;
    // bind the socket
    if (bind(udp_sockfd, (struct sockaddr *)&serverM_udp_addr, sizeof(serverM_udp_addr)) < 0) {
        std::cerr << "Error binding UDP socket to port " << SERVER_M_PORT << std::endl;
        close(udp_sockfd);
        return "";
    }

    // backend server information
    struct sockaddr_in backend_addr;
    memset(&backend_addr, 0, sizeof(backend_addr));
    backend_addr.sin_family = AF_INET;
    backend_addr.sin_port = htons(destPort);
    backend_addr.sin_addr.s_addr = INADDR_ANY;

    // send the book code to backend
    ssize_t sent_bytes = sendto(udp_sockfd, bookCode.c_str(), bookCode.length(), 0, (struct sockaddr*)&backend_addr, sizeof(backend_addr));
    
    if (sent_bytes < 0) {
        std::cerr << "Error sending book code to backend" << std::endl;
        close(udp_sockfd);
        return "";
    }
    else {
        std::cout << "Found " << bookCode << " located at Server " << bookCode[0] << ". Send to Server " << bookCode[0] << "." << std::endl;
    }

    // set a timeout to prevent it from waiting forever
    struct timeval tv;
    tv.tv_sec = 5;  //timeout set 5 seconds
    tv.tv_usec = 0;
    setsockopt(udp_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // receive the response from backend
    char udp_buffer[1024] = {0};
    socklen_t backend_addr_len = sizeof(backend_addr);

    ssize_t udp_recv_len = recvfrom(udp_sockfd, udp_buffer, sizeof(udp_buffer), 0, (struct sockaddr*)&backend_addr, &backend_addr_len);
    
    if (udp_recv_len < 0) {
        std::cerr << "Error receiving response from backend server" << std::endl;
        close(udp_sockfd);
        return "";
    }

    close(udp_sockfd);

    // return the response
    return std::string(udp_buffer, udp_recv_len);
}

int main() {

    // initialize server
    std::cout << "Main Server is up and running." << std::endl;

    // Phase2: Login and confirmation
    loadMemberList();
    std::cout << "Main Server loaded the member list." << std::endl;

    // set up TCP for client;
    int tcp_sockfd, new_socket;
    struct sockaddr_in tcp_servaddr, tcp_cliaddr;
    socklen_t tcp_clilen;

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) {
        std::cerr << "TCP Socket creation failed" << std::endl;
        return 1;
    }

    // Portions of this code were developed with assistance from OpenAI's ChatGPT. 
    // set socket to be reusable
    int yes = 1;
    if (setsockopt(tcp_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        std::cerr << "setsockopt SO_REUSEADDR failed" << std::endl;
        close(tcp_sockfd); 
        return 1;
    }

    // TCP server information
    memset(&tcp_servaddr, 0, sizeof(tcp_servaddr));
    tcp_servaddr.sin_family = AF_INET; // IPv4
    tcp_servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    tcp_servaddr.sin_port = htons(45162); // main server port

    // bind TCP address with server address
    if (bind(tcp_sockfd, (const struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr)) < 0) {
        std::cerr << "TCP bind failed" << std::endl;
        return 1;
    }

    listen(tcp_sockfd, 5);

    // Portions of this code were developed with assistance from OpenAI's ChatGPT. 
    // use getsockname to get dynamically assigned port number
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(tcp_sockfd, (struct sockaddr *)&sin, &len) == -1) {
        perror("getsockname");
        close(tcp_sockfd);
        exit(1);
    }
    
    bool isAuthenticated = false;
    while (!isAuthenticated) {

        // accept client connection
        tcp_clilen = sizeof(tcp_cliaddr);
        new_socket = accept(tcp_sockfd, (struct sockaddr *)&tcp_cliaddr, &tcp_clilen);
        if (new_socket < 0) {
            std::cerr << "TCP accept failed" << std::endl;
            return 1;
        }

        // receive encrypted credentials from client
        char buffer[1024] = {0};
        recv(new_socket, buffer, sizeof(buffer), 0);

        std::istringstream iss(buffer);
        std::string encrypted_username, encrypted_password, temp;
        std::getline(iss, encrypted_username, ',');
        std::getline(iss, encrypted_password);
        std::string auth_result;

        std::cout << "Main Server received the username and password from the client using TCP over port " << ntohs(tcp_servaddr.sin_port) <<"." << std::endl;

        if (memberList.find(encrypted_username) == memberList.end()) {
            auth_result = " received the result of authentication from Main Server using TCP over port " + std::to_string(ntohs(tcp_cliaddr.sin_port)) + ".\nAuthentication failed: Username not found.";
        }
        else if (memberList[encrypted_username] != encrypted_password) {
            auth_result = " received the result of authentication from Main Server using TCP over port " + std::to_string(ntohs(tcp_cliaddr.sin_port)) + ".\nAuthentication failed: Password does not match.";
        }
        else {
            auth_result = " received the result of authentication from Main Server using TCP over port " + std::to_string(ntohs(tcp_cliaddr.sin_port)) + ".\nAuthentication is successful.";
        }

        if (auth_result.find("Authentication is successful") != std::string::npos) {
            std::cout << "Password " << encrypted_password << " matches the username. Send a reply to the client." << std::endl;
            isAuthenticated = true;
            send(new_socket, auth_result.c_str(), auth_result.length(), 0);
        }
        else if (auth_result.find("Username") != std::string::npos) {
            std::cout << encrypted_username << " is not registered. Send a reply to the client." << std::endl;
            send(new_socket, auth_result.c_str(), auth_result.length(), 0); 
            // close the socket and listen for new connections
            close(new_socket); 
        }
        else {
            std::cout << "Password " << encrypted_password << " does not match the username. Send a reply to the client." << std::endl;
            send(new_socket, auth_result.c_str(), auth_result.length(), 0); 
            // close the socket and listen for new connections
            close(new_socket); 
        }
    }

    // Phase 3: Forwarding request to Backend Servers
    while (true) {
        // receive book code from client
        char bookCodeBuffer[1024] = {0};
        ssize_t bookCodeLength = recv(new_socket, bookCodeBuffer, sizeof(bookCodeBuffer), 0);
        if (bookCodeLength < 0) {
            std::cerr << "Error receiving book code from client" << std::endl;
            close(new_socket);
            close(tcp_sockfd);
            return 1;
        }

        // ensure it is null-terminated
        bookCodeBuffer[bookCodeLength] = '\0';
        std::string bookCode(bookCodeBuffer);
        std::cout << "Main Server received the book request from client using TCP over port 45162" << std::endl;
        
        // determine what backend server
        int destPort = 0;
        if (bookCode[0] == 'S') {
            destPort = SERVER_S_PORT;
        }
        else if (bookCode[0] == 'L') {
            destPort = SERVER_L_PORT;
        }
        else if (bookCode[0] == 'H') {
            destPort = SERVER_H_PORT;
        }
        else {
            std::cout << "Did not find " << bookCode << " in the book code list." << std::endl;
        }

        // Phase 4
        std::string response = "wrong code";

        if (destPort != 0) {
            response = sendBookCodeBackend(bookCode, destPort);

            if (response.empty()) {
                std::cerr << "Error receiving book status from backend" << std::endl;
            }
            else {
                std::cout << "Main Server received from server " << bookCode[0] << " the book status result using UDP over port " << SERVER_M_PORT << ":" << std::endl;
                std::cout << response << std::endl;
            }

        }

        // send response to client over TCP
        send(new_socket, response.c_str(), response.length(), 0);
        std::cout << "Main Server sent the book status to the client." << std::endl;
    }

    close(new_socket);
    close(tcp_sockfd);

    return 0;

}