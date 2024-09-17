# Socket Programming Project - Library Management System

## Project Overview

The project implements a **Library Management System** using **TCP** and **UDP** socket communications between a client, a main server, and multiple backend servers.

The system allows users to log in, check the availability of books in specific departments (Science, Literature, History), and borrow books if available. Additionally, it supports an inventory management feature for library staff.

## Table of Contents

- [Project Overview](#project-overview)
- [Project Structure](#project-structure)
- [Features](#features)
- [Message Formats](#message-formats)
- [How to Run](#how-to-run)
- [Idiosyncrasies](#idiosyncrasies)
- [Reused Code](#reused-code)
- [License](#license)

## Project Structure

The project contains the following main components:

- `client.cpp`: A client program used to authenticate users and send book queries to the main server.
- `serverM.cpp`: The main server responsible for managing client connections, user authentication, and forwarding book requests to backend servers.
- `serverS.cpp`: Backend server for the **Science** department. It handles book availability requests for Science-related books.
- `serverL.cpp`: Backend server for the **Literature** department. It handles book availability requests for Literature-related books.
- `serverH.cpp`: Backend server for the **History** department. It handles book availability requests for History-related books.
- `Makefile`: A makefile that compiles all the code files into executables.

## Features

1. **Client-Server Communication**:
    - The client communicates with the main server using **TCP**.
    - The main server forwards book requests to backend servers via **UDP**.

2. **User Authentication**:
    - The client encrypts login information before sending it to the main server.
    - The main server verifies credentials using the `member.txt` file.

3. **Book Availability Check**:
    - After logging in, users can query the availability of books by entering a book code.
    - The system checks for the availability in the corresponding department server (Science, Literature, or History).

4. **Inventory Management**:
    - For extra credit, staff members can log in using the credentials `Admin:Admin` and check the total availability of books in the library.

## Message Formats

- **Authentication Request**: 
    - Username and password are concatenated by a comma and sent over TCP.
  
- **Book Query**: 
    - Book codes are sent as text strings over TCP from the client to the main server.

- **Backend Server Response**: 
    - Book availability status is sent as a string from the backend servers to the main server over UDP, and the main server forwards the response to the client via TCP.

## How to Run

### Requirements

- Ubuntu 22.04 or compatible environment (e.g., the provided EE450 student VM).
- GCC or G++ compiler.

### Compilation

Run the following command to compile all the code files:

```bash
make all
```

### Starting the Servers and Client

1. Start the main server:

   ```bash
   ./serverM
   ```

2. Start the backend servers in separate terminals:

   ```bash
   ./serverS
   ./serverL
   ./serverH
   ```

3. Start the client:

   ```bash
   ./client
   ```

4. Follow the on-screen prompts to log in and query book availability.

### Clean Up

To remove the compiled executables, run:

```bash
make clean
```

## Idiosyncrasies

- **Order of Execution**: The backend servers (`serverS`, `serverL`, `serverH`) should be running before the main server (`serverM`) can forward book queries.
- **Concurrency**: The servers currently process one client request at a time. The system does not handle multiple concurrent requests.

## Reused Code

Parts of the project reference code from Beej's guide:

- `serverM.cpp`: `loadMemberList()`, `sendBookCodeBackend()`, and part of the `main()` function.
- `serverS.cpp`, `serverL.cpp`, `serverH.cpp`: `readBooksFromInput()` and part of the `main()` function.
- `client.cpp`: `encrypt()` and part of the `main()` function.
