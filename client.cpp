#include <iostream>       // Biblioteca para operaciones de entrada y salida
#include <cstring>        // Biblioteca para operaciones con cadenas de caracteres
#include <netinet/in.h>   // Biblioteca para estructuras y funciones de red
#include <unistd.h>       // Biblioteca para funciones POSIX
#include <arpa/inet.h>    // Biblioteca para funciones de conversión de direcciones de red

// Constantes para el tamaño del tablero
const int ROWS = 6;
const int COLUMNS = 7;

// Constantes para las piezas del servidor y del cliente
const char SERVER_PIECE = 'S';
const char CLIENT_PIECE = 'C';

// Estructura que representa el estado del juego
struct GameState {
    char board[ROWS][COLUMNS]; // Tablero del juego
    bool serverTurn;           // Indica si es el turno del servidor
    bool gameFinished;         // Indica si el juego ha terminado
    char winner;               // Almacena el ganador del juego ('S' para servidor, 'C' para cliente, ' ' para empate)
};

// Función para mostrar el tablero en la consola
void showBoard(const GameState &game) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLUMNS; ++j) {
            std::cout << game.board[i][j] << ' '; // Muestra cada celda del tablero
        }
        std::cout << std::endl;
    }
    std::cout << "-------------" << std::endl;
    for (int j = 0; j < COLUMNS; ++j) {
        std::cout << j + 1 << ' '; // Muestra los números de columna para la selección del jugador
    }
    std::cout << std::endl;
}

// Función para conectarse al servidor
bool connectToServer(int &clientSocket, const char* serverIP, int port) {
    // Crear el socket del cliente
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error al crear el socket\n";
        return false;
    }

    // Configurar la dirección del servidor
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddress.sin_addr);

    // Intentar conectar al servidor
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Error al conectar\n";
        close(clientSocket);
        return false;
    }

    return true;
}

// Función principal del juego
void play(GameState &game, int clientSocket) {
    while (true) {
        // Recibir el estado del juego desde el servidor
        if (recv(clientSocket, &game, sizeof(GameState), 0) <= 0) {
            std::cerr << "Error al recibir datos del servidor\n";
            break;
        }
        // Mostrar el tablero en la consola
        showBoard(game);

        // Verificar si el juego ha terminado
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

        // Si no es el turno del servidor, solicitar al usuario que elija una columna
        if (!game.serverTurn) {
            std::cout << "Elige una columna: ";
            int column;
            std::cin >> column;
            column--; // Ajustar para índice basado en 0
            send(clientSocket, &column, sizeof(int), 0); // Enviar la columna elegida al servidor
        } else {
            std::cout << "Esperando movimiento del servidor..." << std::endl;
        }
    }
}

// Función principal del programa
int main(int argc, char* argv[]) {
    // Verificar que se han proporcionado los argumentos correctos
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <IP del servidor> <puerto>\n";
        return 1;
    }

    const char* serverIP = argv[1]; // IP del servidor
    int port = std::stoi(argv[2]);  // Puerto del servidor
    int clientSocket;               // Descriptor de socket del cliente

    // Conectarse al servidor
    if (!connectToServer(clientSocket, serverIP, port)) {
        return 1;
    }

    GameState game; // Estado del juego
    play(game, clientSocket); // Iniciar el juego

    close(clientSocket); // Cerrar el socket del cliente
    return 0;
}
