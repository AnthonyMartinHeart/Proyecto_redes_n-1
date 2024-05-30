#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// Definición del tamaño del tablero
const int ROWS = 6;
const int COLS = 7;
const char SERVER_PIECE = 'S'; // Pieza del servidor
const char CLIENT_PIECE = 'C'; // Pieza del cliente

// Estructura para representar el estado del juego
struct GameState {
    char board[ROWS][COLS];
    bool serverTurn;
    bool gameOver;
    char winner;
};

// Función para imprimir el tablero de juego
void printBoard(GameState &game) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            std::cout << game.board[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << "-------------" << std::endl;
    for (int j = 0; j < COLS; ++j) {
        std::cout << j + 1 << ' ';
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // Verifica que se hayan proporcionado la IP del servidor y el puerto
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <IP del servidor> <puerto>\n";
        return 1;
    }

    // Obtiene la IP del servidor y el puerto desde los argumentos
    const char* serverIP = argv[1];
    int port = std::stoi(argv[2]);
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); // Crea el socket del cliente

    // Configura la dirección del servidor
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    // Intenta conectar al servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error al conectar\n";
        return 1;
    }

    GameState game;
    bool continuePlaying = true;
    while (continuePlaying) {
        // Recibe el estado del juego desde el servidor
        recv(clientSocket, &game, sizeof(GameState), 0);
        printBoard(game);

        // Verifica si el juego ha terminado
        if (game.gameOver) {
            std::cout << (game.winner == CLIENT_PIECE ? "¡Gana cliente!" : "¡Gana servidor!") << std::endl;
            std::cout << "Fin del juego." << std::endl;
            break;
        }

        // Si es el turno del cliente, solicita una columna y envía el movimiento al servidor
        if (!game.serverTurn) {
            std::cout << "Elige una columna: ";
            int col;
            std::cin >> col;
            col--; // Ajusta para índice basado en 0
            send(clientSocket, &col, sizeof(int), 0);
        } else {
            // Si es el turno del servidor, espera el movimiento del servidor
            std::cout << "Esperando movimiento del servidor..." << std::endl;
        }
    }

    close(clientSocket); // Cierra el socket del cliente
    return 0;
}
