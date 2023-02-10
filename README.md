## TCP Proxy written in C++

This code compiles a C++ TCP server that enables communication between two clients. The server acts as a proxy, allowing clients to send messages to each other. The connection between the clients can also be established if they share the same secret.

## Features:

    1. Supports two clients simultaneously
    2. Facilitates message exchange between clients
    3. Ensures secure connection by requiring clients to have the same secret if secret_mode is activated
    4. If a client is the only one connected after 7 seconds, the server will disconnect the client
    5. Clients can use commands like '[CMD]ECHOREPLY "message"' to send messages to the other client

## Usage

To run the server, compile the C++ code and execute the resulting binary file. The server will listen for incoming client connections on the specified port. When two clients are connected, they can start exchanging messages through the server. If the clients have the same secret, they will be able to establish a direct connection.

- At the root of the repo using the Makefile rule:
```sh
make re
```
Then use the ```server_side``` and ```client_side``` binaries.
Use ```-h``` flag to know the usages.

## Configuration

The server can be configured by modifying the following parameters in the code:

Port number: The port number on which the server listens for incoming connections
Secret: The secret required for clients to connect to each other

## Requirements

A C++ compiler
Network connectivity for the server and clients
The c++ Boost/Asio installed

- On Fedora:
```sh
sudo dnf -y install boost
```
- On Ubuntu and alikes:
```sh
sudo apt-get install libboost-all-dev
```
- On Mac Os: https://www.boost.org/. 

## Notes

The server is designed to handle only two clients at a time.
The server does not store messages between client connections.

