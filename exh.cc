#include <iostream>
#include <vector>
#include <chrono> // Llibreria per mesurar el temps d'execució
#include <climits> // Include for INT_MAX
#include <cfloat>

using namespace std;

// Estructura per representar un rectangle, amb amplada (width), altura (height) i quantitat (quantity)
struct Rectangle {
    int width, height;
    int quantity;
};

// Estructura per emmagatzemar les coordenades de posició dels rectangles
struct PosCoords {
    int lx, ly; // Coordenades superior esquerra
    int rx, ry; // Coordenades inferior dreta
};

// Variables globals
int W, N;                          // Amplada del rotlle (W) i nombre total de comandes (N)
vector<Rectangle> rectangles;      // Vector per emmagatzemar les comandes de rectangles
double bestLength = DBL_MAX;       // Longitud mínima inicial, molt gran per assegurar que qualsevol longitud calculada serà millor
vector<PosCoords> bestPlacement;   // Vector per guardar la millor col·locació dels rectangles

// Expandeix el grid afegint files noves fins a tenir com a mínim newHeight files
void expandGrid(vector<vector<bool>> &grid) {
    grid.push_back(vector<bool>(W, false)); // Afegeix una fila de mida W inicialitzada a false
}

// Comprova si es pot col·locar un rectangle en una posició específica dins del grid
bool canPlace(vector<vector<bool>> &grid, int x, int y, int w, int h) {
    // Comprova si el rectangle excedeix els límits actuals del grid
    if (x + w > W ) 
        return false;

    // Recorre cada cel·la que ocuparia el rectangle per comprovar si està lliure
    for (int i = x; i < x + w; ++i) {
        for (int j = y; j < y + h; ++j) {
            if (grid[j][i])
                return false; // Si la cel·la ja està ocupada, no es pot col·locar aquí
        }
    }
    return true; // Si totes les cel·les estan lliures, es pot col·locar el rectangle
}

// Marca les cel·les ocupades per un rectangle en el grid
void placeRectangle(vector<vector<bool>> &grid, int x, int y, int w, int h, vector<PosCoords> &placement) {
    
    // Afegeix les coordenades de posició al vector de coordenades
    PosCoords pos = {x, y, x + w, y + h}; // Definició de la posició del rectangle
    placement.push_back(pos);            // Afegeix la nova posició al vector
    for (int i = x; i < x + w; ++i) {
        for (int j = y; j < y + h; ++j) {
            grid[j][i] = true; // Marca la cel·la com a ocupada
        }
    }
}

// Funció per comprovar si val la pena continuar amb la branca actual (pruning)
bool isPromising(int currentLength) {
    int remainingArea = 0;

    // Calcula l'àrea restant dels rectangles no col·locats completament
    for (int i = 0; i < static_cast<int>(rectangles.size()); ++i) {
        remainingArea += rectangles[i].quantity * rectangles[i].width * rectangles[i].height;
    }

    // Estimació de la longitud mínima necessària per col·locar l'àrea restant
    int remainingLenEst = (remainingArea + W - 1) / W;

    // Retorna true si la branca és prometedora; sinó, false
    return (currentLength + remainingLenEst < bestLength);
}

// Funció de cerca exhaustiva per trobar la millor configuració dels rectangles
void exhaustiveSearch(vector<vector<bool>> &grid, int placedCount, int currentLength, vector<PosCoords> &placement) {
    // Cas base: si hem col·locat tots els rectangles
    
    
    if (placedCount == N) {
        if (currentLength < bestLength) {
            bestLength = currentLength;  // Actualitza la millor longitud
            bestPlacement = placement;  // Desa la millor col·locació
        }
        return; // Retorna per explorar altres configuracions
    }

    // Comprova si aquesta branca és prometedora
    if (!isPromising(currentLength))
        return; // Pruneja la branca si no és prometedora

    // Explora tots els rectangles
    for (int i = 0; i < static_cast<int>(rectangles.size()); ++i) {
        // Comprova si encara podem col·locar més còpies d'aquest rectangle
        
        if (rectangles[i].quantity <= 0)
            continue;

        Rectangle rect = rectangles[i];
        while (static_cast<int>(grid.size())-currentLength < max(rect.height,rect.width)){
            expandGrid(grid);
        }
        
        // Prova totes les orientacions possibles
        int maxOrientation = (rect.width == rect.height) ? 1 : 2;
        for (int orientation = 0; orientation < maxOrientation; ++orientation) {
            int w = (orientation == 0) ? rect.width : rect.height;
            int h = (orientation == 0) ? rect.height : rect.width;

            // Prova col·locar el rectangle a totes les posicions
            bool placed= false;
            for (int y = 0; y < static_cast<int>(grid.size()) && !placed; ++y) {
                    for (int x = 0; x < W && !placed; ++x) {
                        if (canPlace(grid, x, y, w, h)) {
                            placeRectangle(grid, x, y, w, h, placement); // Col·loca el rectangle
                            placed= true;
                            rect.quantity--; // Marca una còpia més d'aquest rectangle com utilitzada

                                    // Continua la recursió amb el següent rectangle
                            exhaustiveSearch(grid, placedCount + 1, max(currentLength, y + h), placement);

                                    // Desfés el canvi per explorar altres opcions
                            placement.pop_back();
                            for (int px = x; px < x + w; ++px) {
                                for (int py = y; py < y + h; ++py) {
                                    grid[py][px] = false;
                                        }
                                    }
                            rect.quantity++;
                                }
                            }
                        }
        }

    }
}

void readInput(vector<Rectangle> &rectangles) {
    
    cin >> W >> N;
    for (int i = 0; i < N; ++i) {
        int ni, pi, qi;
        cin >> ni >> pi >> qi;
        for (int j = 0; j < ni; ++j) {
            rectangles.push_back({pi, qi, ni});
        }
        i += ni - 1;
    }
}

int main() {
   
    readInput(rectangles);

    // Inicialitzar la graella amb una sola fila
    vector<vector<bool>> grid; // Graella inicial amb una sola fila
  // Vector per fer seguiment de quantes còpies s'han col·locat de cada rectangle
    vector<PosCoords> placement; // Coordenades dels rectangles col·locats

    // Mesura el temps d'execució
    auto start = chrono::high_resolution_clock::now();
    exhaustiveSearch(grid, 0, 0, placement); // Crida a la cerca exhaustiva
    auto end = chrono::high_resolution_clock::now();

    // Escriu la solució al fitxer de sortida
    chrono::duration<double> elapsed = end - start;
    cout << elapsed.count() << endl; // Escriu el temps d'execució
    cout << bestLength << endl;      // Escriu la millor longitud trobada
    for (const auto &pos : bestPlacement) {
        cout << pos.lx << " " << pos.ly << " " << pos.rx << " " << pos.ry << endl; // Coordenades dels rectangles
    }

    return 0;
}