#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

// Definición de códigos de colores ANSI para la consola
const string RESET       = "\033[0m";
const string COLOR_WHITE = "\033[1;97m"; // Blanco brillante
const string COLOR_BLACK = "\033[1;30m"; // Gris oscuro / Negro (visible en fondo oscuro)
const string COLOR_GRID  = "\033[1;34m"; // Azul brillante para los bordes del tablero

// Estructura para registrar cada movimiento realizado
struct Move {
    int r1, c1, r2, c2;
};

class Checkers {
private:
    char board[8][8];
    char turn; // 'b' = Blanco, 'n' = Negro
    vector<Move> history;

    // Función para formatear las fichas con color al imprimirlas
    string getColoredPiece(char p) const {
        switch (p) {
            case 'b': return COLOR_WHITE + "b" + RESET;
            case 'B': return COLOR_WHITE + "B" + RESET;
            case 'n': return COLOR_BLACK + "n" + RESET;
            case 'N': return COLOR_BLACK + "N" + RESET;
            default:  return ".";
        }
    }

public:
    Checkers() {
        resetBoard();
    }

    void resetBoard() {
        turn = 'n'; // En damas inglesas el color oscuro (Negro) inicia la partida
        history.clear();

        for (int r = 0; r < 8; ++r) {
            for (int c = 0; c < 8; ++c) {
                board[r][c] = '.';
                // Colocar piezas solo en casillas oscuras (suma de índices impar)
                if ((r + c) % 2 != 0) {
                    if (r < 3) {
                        board[r][c] = 'n'; // Ficha Negra
                    } else if (r > 4) {
                        board[r][c] = 'b'; // Ficha Blanca
                    }
                }
            }
        }
    }

    void displayBoard() const {
        cout << "\n    0 1 2 3 4 5 6 7  (Columnas)\n";
        cout << COLOR_GRID << "  +-----------------+\n" << RESET;
        for (int r = 0; r < 8; ++r) {
            cout << r << COLOR_GRID << " | " << RESET;
            for (int c = 0; c < 8; ++c) {
                cout << getColoredPiece(board[r][c]) << ' ';
            }
            cout << COLOR_GRID << "|\n" << RESET;
        }
        cout << COLOR_GRID << "  +-----------------+\n" << RESET;
        cout << "Leyenda: " << COLOR_WHITE << "b/B" << RESET << " = Blancas | " 
             << COLOR_BLACK << "n/N" << RESET << " = Negras (Mayúsculas = Damas)\n";
        cout << "Turno actual: " << (turn == 'b' ? COLOR_WHITE + "BLANCO ('b'/'B')" : COLOR_BLACK + "NEGRO ('n'/'N')") << RESET << "\n\n";
    }

    bool isOwnPiece(char p, char player) const {
        if (player == 'b') return p == 'b' || p == 'B';
        if (player == 'n') return p == 'n' || p == 'N';
        return false;
    }

    bool isOpponentPiece(char p, char player) const {
        if (player == 'b') return p == 'n' || p == 'N';
        if (player == 'n') return p == 'b' || p == 'B';
        return false;
    }

    bool makeMove(int r1, int c1, int r2, int c2, bool record = true) {
        if (r1 < 0 || r1 >= 8 || c1 < 0 || c1 >= 8 || r2 < 0 || r2 >= 8 || c2 < 0 || c2 >= 8) {
            return false;
        }

        char piece = board[r1][c1];
        if (!isOwnPiece(piece, turn)) return false;
        if (board[r2][c2] != '.') return false;

        int dr = r2 - r1;
        int dc = abs(c2 - c1);

        if (dc != abs(dr)) return false;

        bool isKing = (piece == 'B' || piece == 'N');
        int forwardDirection = (turn == 'b') ? -1 : 1;

        // 1. Movimiento simple
        if (abs(dr) == 1 && dc == 1) {
            if (!isKing && (dr != forwardDirection)) return false;

            board[r2][c2] = piece;
            board[r1][c1] = '.';
        }
        // 2. Captura / Salto
        else if (abs(dr) == 2 && dc == 2) {
            if (!isKing && (dr / 2 != forwardDirection)) return false;

            int midR = r1 + dr / 2;
            int midC = c1 + (c2 - c1) / 2;
            char midPiece = board[midR][midC];

            if (!isOpponentPiece(midPiece, turn)) return false;

            board[midR][midC] = '.';
            board[r2][c2] = piece;
            board[r1][c1] = '.';
        } else {
            return false;
        }

        // Coronación a Dama (Rey)
        if (turn == 'b' && r2 == 0) board[r2][c2] = 'B';
        if (turn == 'n' && r2 == 7) board[r2][c2] = 'N';

        if (record) {
            history.push_back({r1, c1, r2, c2});
        }

        turn = (turn == 'b') ? 'n' : 'b';
        return true;
    }

    void saveGame(const string& filename) const {
        ofstream file(filename);
        if (!file.is_open()) {
            cout << "Error: No se pudo crear el archivo para guardar.\n";
            return;
        }

        for (const auto& m : history) {
            file << m.r1 << " " << m.c1 << " " << m.r2 << " " << m.c2 << "\n";
        }

        file.close();
        cout << "Partida guardada con exito en '" << filename << "'.\n";
    }

    void replayGame(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cout << "Error: No se pudo encontrar o abrir el archivo '" << filename << "'.\n";
            return;
        }

        resetBoard();
        cout << "\n=========================================";
        cout << "\n=== INICIANDO REPETICION DE PARTIDA ===";
        cout << "\n=========================================\n";
        displayBoard();

        int r1, c1, r2, c2;
        int moveNum = 1;

        while (file >> r1 >> c1 >> r2 >> c2) {
            cout << "Movimiento " << moveNum << ": de (" << r1 << "," << c1 << ") a (" << r2 << "," << c2 << ")...\n";
            
            this_thread::sleep_for(chrono::milliseconds(1500));

            makeMove(r1, c1, r2, c2, false);
            displayBoard();
            moveNum++;
        }

        file.close();
        cout << "=== FIN DE LA REPETICION ===\n\n";
    }
};

int main() {
    Checkers game;
    int option;

    while (true) {
        cout << "=== JUEGO DE DAMAS INGLESAS ===\n";
        cout << "1. Jugar nueva partida\n";
        cout << "2. Cargar y repetir partida guardada\n";
        cout << "3. Salir\n";
        cout << "Seleccione una opcion: ";
        cin >> option;

        if (option == 1) {
            game.resetBoard();
            while (true) {
                game.displayBoard();
                cout << "Ingrese origen y destino (fila_orig col_orig fila_dest col_dest)\n";
                cout << "o ingrese '-1' para guardar la partida y salir al menu: ";

                int r1;
                cin >> r1;

                if (r1 == -1) {
                    string filename;
                    cout << "Nombre del archivo para guardar (ejemplo: partida.txt): ";
                    cin >> filename;
                    game.saveGame(filename);
                    break;
                }

                int c1, r2, c2;
                cin >> c1 >> r2 >> c2;

                if (!game.makeMove(r1, c1, r2, c2)) {
                    cout << "\n[!] Movimiento invalido. Intente de nuevo.\n";
                }
            }
        } else if (option == 2) {
            string filename;
            cout << "Ingrese el nombre del archivo de la partida guardada: ";
            cin >> filename;
            game.replayGame(filename);
        } else if (option == 3) {
            cout << "¡Gracias por jugar!\n";
            break;
        } else {
            cout << "Opcion no valida.\n\n";
        }
    }

    return 0;
}