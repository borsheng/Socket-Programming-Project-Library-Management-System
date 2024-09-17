// Portions of this code were developed with assistance from OpenAI's ChatGPT. 
#include <iostream>
#include <string>
#include <cstring>
#include <limits>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// Portions of this code were developed with assistance from OpenAI's ChatGPT. 
std::string encrypt(const std::string& input) {
    std::string encrypted = input;
    for (char& c : encrypted) {
        if ((c >= 'a' && c <= 'u') || (c >= 'A' && c <= 'U') || (c >= '0' && c <= '4')) {
            c += 5;
        }
        else if ((c >= 'v' && c <= 'z') || (c >= 'V' && c <= 'Z') || (c >= '5' && c <= '9')) {
            c -= 21;
        }
    }
    return encrypted;
}

int main() {

    struct sockaddr_in servaddr;
    // server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // server IP
    servaddr.sin_port = htons(45162); // main server port

    // initialize server and set up TCP
    std::cout << "Client is up and running." << std::endl;

    bool isAuthenticated = false;
    std::string username, password, unencrypted_name;

    while (!isAuthenticated) {
        // Portions of this code were developed with assistance from OpenAI's ChatGPT. 
        // create TCP socket 
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd <0) {
            std::cerr << "Error creating socket!" << std::endl;
            return 1;
        }

        // connect to main server
        if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            std::cerr << "Connect failed" << std::endl;
            return 1;
        }

        // retrieve the dynamically-assigned port
        struct sockaddr_in my_addr;
        socklen_t addrlen = sizeof(my_addr);
        if (getsockname(sockfd, (struct sockaddr*)&my_addr, &addrlen) == -1) {
            perror("getsockname");
            exit(1);
        }

        std::cout << "Please enter the username: ";
        std::cin >> username;
        std::cout << "Please enter the password: ";
        std::cin >> password;

        unencrypted_name = username;
        username = encrypt(username);
        password = encrypt(password);

        // send it the main server
        std::string credentials = username + "," + password;
        send(sockfd, credentials.c_str(), credentials.length(), 0);
        std::cout << unencrypted_name << " sent an authentication request to the Main Server." << std::endl;

        // receive the authentication result from main server
        char buffer[1024] = {0};
        recv(sockfd, buffer, sizeof(buffer), 0);
        // print the authentication result
        std::cout << unencrypted_name << buffer << std::endl;

        // if success, ask for book code
        if (std::string(buffer).find("Authentication is successful") != std::string::npos) {
            isAuthenticated = true;
        }
        else {
            // if fail, try again
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // if login success, ask and send book code to main server
        while (isAuthenticated) {
            std::string bookCode;
            std::cout << "Please enter book code to query: ";
            std::cin >> bookCode;

            // send book code to main server
            send(sockfd, bookCode.c_str(), bookCode.length(), 0);
            std::cout << unencrypted_name << " sent the request to the Main Server." << std::endl;

            // receive response from main server
            char tcp_buffer[1024] = {0};
            ssize_t tcp_recv_len = recv(sockfd, tcp_buffer, sizeof(tcp_buffer), 0);

            if (tcp_recv_len < 0) {
                std::cerr << "Error receiving response from Main Server" << std::endl;
            }
            else {
                tcp_buffer[tcp_recv_len] = '\0';
                std::cout << "Response received from the Main Server on TCP port: " << ntohs(servaddr.sin_port) << std::endl;
            }

            if (std::string(tcp_buffer).find("available is") != std::string::npos) {
                std::cout << "The requested book " << bookCode << " is available in the library." << std::endl;
            }
            else if (std::string(tcp_buffer).find("not available") != std::string::npos) {
                std::cout << "The requested book " << bookCode << " is NOT available in the library." << std::endl;
            }
            else if (std::string(tcp_buffer).find("wrong code") != std::string::npos || std::string(tcp_buffer).find("Not able") != std::string::npos) {
                std::cout << "Not able to find the book-code " << bookCode << " in the system." << std::endl;
            }

            std::cout << "\n—- Start a new query —-" << std::endl;
        }

        close(sockfd);
        
    }
    
    return 0;
}