#ifndef SERVER_H
#define SERVER_H

class Server {
public:
    Server(int port);
    ~Server();
    void startListening();

private:
    int port;
    int serverSocket;

    void handleClient(int clientSocket);
};

#endif // SERVER_H
