#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

const int ROWS = 6;
const int COLUMNS = 7;
const char SERVER_PIECE = 'S';
const char CLIENT_PIECE = 'C';

class Game {
public:
    Game() : serverTurn(rand() % 2 == 0), gameFinished(false), winner(' ') {
        initializeBoard();
    }

    void initializeBoard() {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                board[i][j] = ' ';
            }
        }
    }

    bool insertPiece(int column, char piece) {
        if (column < 0 || column >= COLUMNS || board[0][column] != ' ') return false;
        for (int i = ROWS - 1; i >= 0; --i) {
            if (board[i][column] == ' ') {
                board[i][column] = piece;
                return true;
            }
        }
        return false;
    }

    bool checkVictory(char piece) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                if (board[i][j] == piece) {
                    if (countPieces(i, j, 0, 1, piece) || // Horizontal
                        countPieces(i, j, 1, 0, piece) || // Vertical
                        countPieces(i, j, 1, 1, piece) || // Descending diagonal
                        countPieces(i, j, 1, -1, piece))  // Ascending diagonal
                        return true;
                }
            }
        }
        return false;
    }

    bool isBoardFull() const {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                if (board[i][j] == ' ') {
                    return false;
                }
            }
        }
        return true;
    }

    bool isGameFinished() const {
        return gameFinished;
    }

    void setGameFinished(bool value) {
        gameFinished = value;
    }

    char getWinner() const {
        return winner;
    }

    void setWinner(char value) {
        winner = value;
    }

    bool isServerTurn() const {
        return serverTurn;
    }

    void setServerTurn(bool value) {
        serverTurn = value;
    }

    const char* getBoardRow(int row) const {
        return board[row];
    }

private:
    char board[ROWS][COLUMNS];
    bool serverTurn;
    bool gameFinished;
    char winner;

    bool countPieces(int row, int column, int dRow, int dColumn, char piece) {
        int counter = 0;
        for (int k = 0; k < 4; ++k) {
            int newRow = row + k * dRow;
            int newColumn = column + k * dColumn;
            if (newRow >= 0 && newRow < ROWS &&
                newColumn >= 0 && newColumn < COLUMNS &&
                board[newRow][newColumn] == piece) {
                counter++;
            } else {
                break;
            }
        }
        return counter == 4;
    }
};

void handleClient(int clientSocket, const std::string& clientIP, int clientPort, const std::string& serverIP, int serverPort) {
    std::cout << "Cliente conectado desde IP: " << clientIP << " y puerto: " << clientPort << std::endl;

    Game game;

    // Mostrar quién empieza primero
    if (game.isServerTurn()) {
        std::cout << "El Servidor (" << serverIP << ":" << serverPort << ") empieza primero." << std::endl;
    } else {
        std::cout << "El Cliente (" << clientIP << ":" << clientPort << ") empieza primero." << std::endl;
    }

    while (!game.isGameFinished()) {
        // Enviar tablero y turno al cliente
        send(clientSocket, &game, sizeof(Game), 0);

        if (!game.isServerTurn()) {
            // Obtener movimiento del cliente
            int column;
            recv(clientSocket, &column, sizeof(int), 0);
            std::cout << "Cliente (" << clientIP << ":" << clientPort << ") juega columna: " << column + 1 << std::endl;

            char piece = CLIENT_PIECE;
            if (game.insertPiece(column, piece)) {
                if (game.checkVictory(piece)) {
                    game.setGameFinished(true);
                    game.setWinner(piece);
                    std::cout << "¡Gana el Cliente (" << clientIP << ":"<< clientPort << ")!" << std::endl;
                } else if (game.isBoardFull()) {
                    game.setGameFinished(true);
                    game.setWinner(' ');
                    std::cout << "¡Empate!" << std::endl;
                }
                game.setServerTurn(!game.isServerTurn());
            }
        } else {
            // Turno del servidor
            int column = rand() % COLUMNS;
            std::cout << "Servidor (" << serverIP << ":" << serverPort << ") juega columna: " << column + 1 << std::endl;

            char piece = SERVER_PIECE;
            if (game.insertPiece(column, piece)) {
                if (game.checkVictory(piece)) {
                    game.setGameFinished(true);
                    game.setWinner(piece);
                    std::cout << "¡Gana el Servidor (" << serverIP << ":" << serverPort << ")!" << std::endl;
                } else if (game.isBoardFull()) {
                    game.setGameFinished(true);
                    game.setWinner(' ');
                    std::cout << "¡Empate!" << std::endl;
                }
                game.setServerTurn(!game.isServerTurn());
            }
        }
    }

    // Enviar estado final del juego al cliente
    send(clientSocket, &game, sizeof(Game), 0);
    close(clientSocket);
}

std::string getServerIP() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    struct hostent* hostInfo = gethostbyname(hostname);
    if (hostInfo == nullptr) {
        std::cerr << "Error obteniendo la IP del servidor" << std::endl;
        exit(1);
    }
    struct in_addr* address = (struct in_addr*)hostInfo->h_addr;
    return inet_ntoa(*address);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);

    std::string serverIP = getServerIP();

    std::cout << "Esperando conexiones ..." << std::endl;

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

        std::string clientIP = inet_ntoa(clientAddress.sin_addr);
        int clientPort = ntohs(clientAddress.sin_port);

        std::thread(handleClient, clientSocket, clientIP, clientPort, serverIP, port).detach();
    }

    close(serverSocket);
    return 0;
}

