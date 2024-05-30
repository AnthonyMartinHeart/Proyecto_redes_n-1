#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <netinet/in.h>
#include <unistd.h>

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

// Inicializa el tablero con espacios vacíos
void initializeBoard(GameState &game) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            game.board[i][j] = ' ';
        }
    }
    // Determina aleatoriamente quién empieza
    game.serverTurn = rand() % 2 == 0;
    game.gameOver = false;
    game.winner = ' ';
}

// Función para dejar caer una pieza en una columna
bool dropPiece(GameState &game, int col, char piece) {
    if (col < 0 || col >= COLS || game.board[0][col] != ' ') return false; // Verifica si la columna es válida
    for (int i = ROWS - 1; i >= 0; --i) {
        if (game.board[i][col] == ' ') {
            game.board[i][col] = piece;
            return true;
        }
    }
    return false;
}

// Verifica si hay una condición de victoria
bool checkWin(GameState &game, char piece) {
    // Revisa las posibles condiciones de victoria: horizontal, vertical y diagonal
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (game.board[i][j] == piece) {
                // Horizontal
                if (j + 3 < COLS &&
                    game.board[i][j + 1] == piece &&
                    game.board[i][j + 2] == piece &&
                    game.board[i][j + 3] == piece)
                    return true;
                // Vertical
                if (i + 3 < ROWS &&
                    game.board[i + 1][j] == piece &&
                    game.board[i + 2][j] == piece &&
                    game.board[i + 3][j] == piece)
                    return true;
                // Diagonal
                if (i + 3 < ROWS && j + 3 < COLS &&
                    game.board[i + 1][j + 1] == piece &&
                    game.board[i + 2][j + 2] == piece &&
                    game.board[i + 3][j + 3] == piece)
                    return true;
                if (i - 3 >= 0 && j + 3 < COLS &&
                    game.board[i - 1][j + 1] == piece &&
                    game.board[i - 2][j + 2] == piece &&
                    game.board[i - 3][j + 3] == piece)
                    return true;
            }
        }
    }
    return false;
}

// Maneja la conexión con un cliente
void handleClient(int clientSocket, const std::string &clientInfo) {
    std::cout << "Juego nuevo[" << clientInfo << "]" << std::endl;

    GameState game;
    initializeBoard(game);

    // Imprime quién empieza el juego
    if (game.serverTurn) {
        std::cout << "Juego [" << clientInfo << "]: inicia juego el servidor." << std::endl;
    } else {
        std::cout << "Juego [" << clientInfo << "]: inicia juego el cliente." << std::endl;
    }

    while (!game.gameOver) {
        // Muestra el tablero en el servidor
        std::cout << "Tablero en el servidor:" << std::endl;
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

        // Envia el estado del juego al cliente
        send(clientSocket, &game, sizeof(GameState), 0);

        if (!game.serverTurn) {
            // Obtiene el movimiento del cliente
            int col;
            recv(clientSocket, &col, sizeof(int), 0);

            std::cout << "Juego [" << clientInfo << "]: cliente juega columna " << col + 1 << "." << std::endl;

            char piece = CLIENT_PIECE;
            if (dropPiece(game, col, piece)) {
                if (checkWin(game, piece)) {
                    game.gameOver = true;
                    game.winner = piece;
                    std::cout << "Juego [" << clientInfo << "]: gana cliente." << std::endl;
                }
                game.serverTurn = !game.serverTurn;
            }
        } else {
            // Turno del servidor
            int col = rand() % COLS;

            std::cout << "Juego [" << clientInfo << "]: servidor juega columna " << col + 1 << "." << std::endl;

            char piece = SERVER_PIECE;
            if (dropPiece(game, col, piece)) {
                if (checkWin(game, piece)) {
                    game.gameOver = true;
                    game.winner = piece;
                    std::cout << "Juego [" << clientInfo << "]: gana el servidor." << std::endl;
                }
                game.serverTurn = !game.serverTurn;
            }
        }

        // Verifica si hay empate
        bool draw = true;
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                if (game.board[i][j] == ' ') {
                    draw = false;
                    break;
                }
            }
            if (!draw) break;
        }
        if (draw) {
            game.gameOver = true;
            std::cout << "Juego [" << clientInfo << "]: empate." << std::endl;
        }
    }

    std::cout << "Juego [" << clientInfo << "]: fin del juego." << std::endl;

    // Envia el estado final del juego al cliente
    send(clientSocket, &game, sizeof(GameState), 0);
    close(clientSocket);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Asigna la dirección y el puerto al socket
    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    // Pone el socket en modo de escucha
    listen(serverSocket, 5);

    std::cout << "Esperando conexiones ..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        // Acepta nuevas conexiones
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        // Convierte la dirección y puerto del cliente a una cadena
        std::string clientInfo = std::to_string(clientAddr.sin_addr.s_addr) + ":" + std::to_string(ntohs(clientAddr.sin_port));
        // Crea un nuevo hilo para manejar la conexión del cliente
        std::thread(handleClient, clientSocket, clientInfo).detach();
    }

    close(serverSocket);
    return 0;
}