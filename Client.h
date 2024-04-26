#ifndef CLIENT_H
#define CLIENT_H

class Client {
public:
    Client(char* ip, int port);
    ~Client();
    void start();

private:
    char* ip;
    int port;
};

#endif // CLIENT_H
