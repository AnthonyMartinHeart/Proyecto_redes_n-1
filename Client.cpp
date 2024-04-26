#include "client.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

const int ROWS = 6;
const int COLUMNS = 7;

Client::Client(char* ip, int port) : ip(ip), port(port) {}

Client::~Client() {}

void Client::start() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr << "Error al crear el socket del cliente" << endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error al conectar con el servidor" << endl;
        exit(EXIT_FAILURE);
    }

    char board[ROWS][COLUMNS];
    memset(board, ' ', sizeof(board)); // Inicializamos el tablero con espacios en blanco

    while (true) {
        // Recibir y mostrar el tablero actual del servidor
        recv(clientSocket, board, sizeof(board), 0);
        cout << "TABLERO" << endl;
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                cout << board[i][j] << " ";
            }
            cout << endl;
        }

        // Solicitar la columna al jugador
        int column;
        cout << "Ingrese el número de columna (1-7): ";
        cin >> column;

        // Enviar la columna seleccionada al servidor
        send(clientSocket, &column, sizeof(column), 0);

        // Recibir el resultado del servidor
        char resultMsg[100];
        recv(clientSocket, resultMsg, sizeof(resultMsg), 0);
        cout << resultMsg << endl;

        // Salir del bucle si hay un ganador, empate o si el servidor indica continuar el juego
        if (strcmp(resultMsg, "Gana el servidor") == 0 || strcmp(resultMsg, "Gana el cliente") == 0 || strcmp(resultMsg, "Empate") == 0) {
            break;
        }
    }

    close(clientSocket);
}
