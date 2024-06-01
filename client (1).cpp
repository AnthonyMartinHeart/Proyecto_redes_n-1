#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

const int ROWS = 6;
const int COLUMNS = 7;
const char SERVER_PIECE = 'S';
const char CLIENT_PIECE = 'C';

struct GameState {
    char board[ROWS][COLUMNS];
    bool serverTurn;
    bool gameFinished;
    char winner;
};

void showBoard(const GameState &game) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLUMNS; ++j) {
            std::cout << game.board[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << "-------------" << std::endl;
    for (int j = 0; j < COLUMNS; ++j) {
        std::cout << j + 1 << ' ';
    }
    std::cout << std::endl;
}

bool connectToServer(int &clientSocket, const char* serverIP, int port) {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error al crear el socket\n";
        return false;
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error al conectar\n";
        close(clientSocket);
        return false;
    }

    return true;
}

void play(GameState &game, int clientSocket) {
    while (true) {
        // Receive game state
        if (recv(clientSocket, &game, sizeof(GameState), 0) <= 0) {
            std::cerr << "Error al recibir datos del servidor\n";
            break;
        }
        showBoard(game);

        if (game.gameFinished) {
            if (game.winner == CLIENT_PIECE) {
                std::cout << "¡Gana el Cliente!" << std::endl;
            } else if (game.winner == SERVER_PIECE) {
                std::cout << "¡Gana el Servidor!" << std::endl;
            } else {
                std::cout << "¡Empate!" << std::endl;
            }
            std::cout << "Fin del juego." << std::endl;
            break;
        }

        if (!game.serverTurn) {
            std::cout << "Elige una columna: ";
            int column;
            std::cin >> column;
            column--; // Adjust for 0-based index
            send(clientSocket, &column, sizeof(int), 0);
        } else {
            std::cout << "Esperando movimiento del servidor..." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <IP del servidor> <puerto>\n";
        return 1;
    }

    const char* serverIP = argv[1];
    int port = std::stoi(argv[2]);
    int clientSocket;

    if (!connectToServer(clientSocket, serverIP, port)) {
        return 1;
    }

    GameState game;
    play(game, clientSocket);

    close(clientSocket);
    return 0;
}
