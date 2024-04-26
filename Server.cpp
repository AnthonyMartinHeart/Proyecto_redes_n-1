#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

using namespace std;

const int ROWS = 6;
const int COLUMNS = 7;

Server::Server(int port) : port(port) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Error al crear el socket del servidor" << endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Error en el enlace" << endl;
        exit(EXIT_FAILURE);
    }
}

Server::~Server() {
    close(serverSocket);
}

void Server::startListening() {
    if (listen(serverSocket, 3) < 0) {
        cerr << "Error al escuchar" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Esperando conexiones ..." << endl;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            cerr << "Error al aceptar la conexión" << endl;
            continue;
        }

        cout << "Juego nuevo [" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "]" << endl;

        thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void Server::handleClient(int clientSocket) {
    char board[ROWS][COLUMNS];
    memset(board, ' ', sizeof(board)); // Inicializamos el tablero con espacios en blanco

    while (true) {
        // Mostrar el tablero actual al cliente
        send(clientSocket, board, sizeof(board), 0);

        // Recibir la jugada del cliente
        int column;
        recv(clientSocket, &column, sizeof(column), 0);

        // Realizar la jugada en el tablero
        int row = ROWS - 1;
        while (row >= 0 && board[row][column - 1] != ' ') {
            row--;
        }
        if (row >= 0) {
            board[row][column - 1] = 'S'; // Marcar la jugada del servidor en el tablero
        }

        // Verificar si hay un ganador o empate
        char winner = checkWinner(board);
        if (winner != ' ') {
            // Enviar el resultado al cliente
            string resultMsg;
            if (winner == 'S') {
                resultMsg = "Gana el servidor";
            } else if (winner == 'C') {
                resultMsg = "Gana el cliente";
            } else {
                resultMsg = "Empate";
            }
            send(clientSocket, resultMsg.c_str(), resultMsg.length() + 1, 0);
            break; // Salir del bucle si hay un ganador o empate
        }

        // Implementar el resto de la lógica del juego según sea necesario
    }

    close(clientSocket);
}

char Server::checkWinner(const char board[ROWS][COLUMNS]) {
    // Verificar si hay un ganador en las filas
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLUMNS - 3; ++j) {
            if (board[i][j] != ' ' &&
                board[i][j] == board[i][j + 1] &&
                board[i][j] == board[i][j + 2] &&
                board[i][j] == board[i][j + 3]) {
                return board[i][j];
            }
        }
    }

    // Verificar si hay un ganador en las columnas
    for (int i = 0; i < ROWS - 3; ++i) {
        for (int j = 0; j < COLUMNS; ++j) {
            if (board[i][j] != ' ' &&
                board[i][j] == board[i + 1][j] &&
                board[i][j] == board[i + 2][j] &&
                board[i][j] == board[i + 3][j]) {
                return board[i][j];
            }
        }
    }

    // Verificar si hay un ganador en las diagonales ascendentes
    for (int i = 3; i < ROWS; ++i) {
        for (int j = 0; j < COLUMNS - 3; ++j) {
            if (board[i][j] != ' ' &&
                board[i][j] == board[i - 1][j + 1] &&
                board[i][j] == board[i - 2][j + 2] &&
                board[i][j] == board[i - 3][j + 3]) {
                return board[i][j];
            }
        }
    }

    // Verificar si hay un ganador en las diagonales descendentes
    for (int i = 3; i < ROWS; ++i) {
        for (int j = 3; j < COLUMNS; ++j) {
            if (board[i][j] != ' ' &&
                board[i][j] == board[i - 1][j - 1] &&
                board[i][j] == board[i - 2][j - 2] &&
                board[i][j] == board[i - 3][j - 3]) {
                return board[i][j];
            }
        }
    }

    // Verificar si hay un empate
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLUMNS; ++j) {
            if (board[i][j] == ' ') {
                return ' '; // Todavía hay casillas vacías, el juego continúa
            }
        }
    }

    return 'T'; // Empate
}
