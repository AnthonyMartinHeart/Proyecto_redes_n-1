#include <iostream>         // Librería estándar para entrada y salida
#include <thread>           // Librería para utilizar hilos
#include <vector>           // Librería para utilizar el contenedor vector
#include <string>           // Librería para utilizar la clase string
#include <cstdlib>          // Librería para utilizar funciones de C estándar, como rand()
#include <ctime>            // Librería para manejar tiempo
#include <netinet/in.h>     // Librería para manejar direcciones de internet
#include <unistd.h>         // Librería para utilizar funciones POSIX
#include <arpa/inet.h>      // Librería para convertir direcciones IP
#include <netdb.h>          // Librería para manejar datos de red

// Constantes para el tamaño del tablero y las piezas del juego
const int ROWS = 6;
const int COLUMNS = 7;
const char SERVER_PIECE = 'S';
const char CLIENT_PIECE = 'C';

// Clase que representa el juego
class Game {
public:
    Game() : serverTurn(rand() % 2 == 0), gameFinished(false), winner(' ') {
        initializeBoard();
    }

    // Inicializa el tablero con espacios vacíos
    void initializeBoard() {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                board[i][j] = ' ';
            }
        }
    }

    // Inserta una pieza en una columna específica
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

    // Verifica si hay una victoria con la pieza dada
    bool checkVictory(char piece) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLUMNS; ++j) {
                if (board[i][j] == piece) {
                    if (countPieces(i, j, 0, 1, piece) || // Horizontal
                        countPieces(i, j, 1, 0, piece) || // Vertical
                        countPieces(i, j, 1, 1, piece) || // Diagonal descendente
                        countPieces(i, j, 1, -1, piece))  // Diagonal ascendente
                        return true;
                }
            }
        }
        return false;
    }

    // Verifica si el tablero está lleno
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

    // Devuelve si el juego ha terminado
    bool isGameFinished() const {
        return gameFinished;
    }

    // Establece si el juego ha terminado
    void setGameFinished(bool value) {
        gameFinished = value;
    }

    // Devuelve el ganador del juego
    char getWinner() const {
        return winner;
    }

    // Establece el ganador del juego
    void setWinner(char value) {
        winner = value;
    }

    // Devuelve si es el turno del servidor
    bool isServerTurn() const {
        return serverTurn;
    }

    // Establece el turno del servidor
    void setServerTurn(bool value) {
        serverTurn = value;
    }

    // Devuelve una fila del tablero
    const char* getBoardRow(int row) const {
        return board[row];
    }

private:
    char board[ROWS][COLUMNS];  // Matriz que representa el tablero
    bool serverTurn;            // Indica si es el turno del servidor
    bool gameFinished;          // Indica si el juego ha terminado
    char winner;                // Indica el ganador del juego

    // Cuenta las piezas consecutivas del mismo tipo en una dirección específica
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

// Función que maneja la comunicación con un cliente
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

// Función para obtener la IP del servidor
std::string getServerIP() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));  // Obtener el nombre del host
    struct hostent* hostInfo = gethostbyname(hostname);  // Obtener información del host
    if (hostInfo == nullptr) {
        std::cerr << "Error obteniendo la IP del servidor" << std::endl;
        exit(1);
    }
    struct in_addr* address = (struct in_addr*)hostInfo->h_addr;
    return inet_ntoa(*address);  // Convertir la dirección a una cadena de caracteres
}

// Función principal
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <puerto>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);  // Crear el socket del servidor

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));  // Vincular el socket a la dirección y puerto
    listen(serverSocket, 5);  // Escuchar conexiones entrantes

    std::string serverIP = getServerIP();

    std::cout << "Esperando conexiones ..." << std::endl;

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);  // Aceptar una conexión entrante

        std::string clientIP = inet_ntoa(clientAddress.sin_addr);  // Obtener la IP del cliente
        int clientPort = ntohs(clientAddress.sin_port);  // Obtener el puerto del cliente

        std::thread(handleClient, clientSocket, clientIP, clientPort, serverIP, port).detach();  // Manejar al cliente en un nuevo hilo
    }

    close(serverSocket);  // Cerrar el socket del servidor
    return 0;
}
