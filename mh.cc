#include <iostream>
#include <vector>
#include <tuple>
#include <iomanip>
#include <cstdlib>
#include <algorithm>
#include <random>
#include <cmath>
#include <fstream> // File output
#include <chrono> 

using namespace std;
mt19937 rng(random_device{}());

struct Rectangle {
   int width, height;
   int x1, y1, x2, y2;
};

// Check if a rectangle can be placed
bool canPlace(const vector<vector<bool>>& matrix, int x, int y, int width, int height) {
   int L = int(matrix.size());
   int W = int(matrix[0].size());
   
   // Check bounds
   if (x < 0 || x + width > W || y < 0 || y + height > L) return false;
   
   // Check if all cells are free
   for (int i = y; i < y + height; i++) {
       for (int j = x; j < x + width; j++) {
           if (matrix[i][j]) {
               return false;
           }
       }
   }
   return true;
}

// Mark occupied space
void markOccupied(vector<vector<bool>>& matrix, int x, int y, int width, int height) {
   for (int i = y; i < y + height; i++) {
       for (int j = x; j < x + width; j++) {
           matrix[i][j] = true;
       }
   }
}

void assign_coordinates(Rectangle& rect, int x, int y) {
    rect.x1 = x;
    rect.y1 = y;
    rect.x2 = x + rect.width - 1;
    rect.y2 = y + rect.height - 1;
}

// Place rectangles optimally in the cloth roll (matrix)
void placeRectangles(vector<Rectangle>& rectangles, int W, vector<vector<bool>>& matrix) {   
   for (auto& rect : rectangles) {
       bool placed = false;
       
       // Try to place in current matrix
       for (int y = 0; y < int(matrix.size()) && !placed; y++) {
           for (int x = 0; x <= W - rect.width && !placed; x++) {
               // Try normal orientation if it fits width
               if (rect.width <= W && canPlace(matrix, x, y, rect.width, rect.height)) {
                   assign_coordinates(rect, x, y);
                   markOccupied(matrix, x, y, rect.width, rect.height);
                   placed = true;
               }
               // Try rotated if it would fit 
               else if (rect.height <= W && canPlace(matrix, x, y, rect.height, rect.width)) {
                   swap(rect.width, rect.height);
                   assign_coordinates(rect, x, y);
                   markOccupied(matrix, x, y, rect.width, rect.height);
                   placed = true;
               }
           }
       }
       
       // If not placed, add new rows to place the rectangle
       if (!placed) {
           int newY = int(matrix.size());
           
           // For vertical strips (1xN), try to place them vertically first
           if (rect.width == 1 && rect.height <= W) {
               swap(rect.width, rect.height);
           }
           
           // Add necessary rows
           matrix.resize(newY + rect.height, vector<bool>(W, false));
           
           if (rect.width <= W) {
               assign_coordinates(rect, 0, newY);
               markOccupied(matrix, 0, newY, rect.width, rect.height);
           }
           else if (rect.height <= W) {
               swap(rect.width, rect.height);
               assign_coordinates(rect, 0, newY);
               markOccupied(matrix, 0, newY, rect.width, rect.height);
           }
       }
   }
}

// Create a random initial solution 
tuple<int, vector<Rectangle>> generate_initial_solution(const vector<Rectangle>& positions, int W) {
    vector<Rectangle> initialSolution = positions;  
    vector<vector<bool>> matrix(1, vector<bool>(W, false));
    std::shuffle(initialSolution.begin(), initialSolution.end(), rng); // Shuffle to order rectangles randomly
    placeRectangles(initialSolution, W, matrix);
    return {int(matrix.size()), initialSolution};
}

// Take a random neighbor solution: swap two rectangles and rearrange the rectangles following the new order
tuple<int, vector<Rectangle>> random_neighbor(const vector<Rectangle>& current_layout, int W) {
    vector<Rectangle> neighbor = current_layout;
    std::uniform_int_distribution<> dist(0, neighbor.size() - 1);
    
    int ind1 = dist(rng);
    int ind2 = dist(rng);
    // Make sure the two indices are different
    while (ind1 == ind2) {
        ind2 = dist(rng);
    }
    swap(neighbor[ind1], neighbor[ind2]);
     
    // Rearrange all rectangles
    vector<vector<bool>> matrix(1, vector<bool>(W, false));
    placeRectangles(neighbor, W, matrix);
    return {int(matrix.size()), neighbor}; // New length and new layout (rectangles positions)
}

void print_solution(double time, int optimalLength, vector<Rectangle>& bestLayout, const string& outputFileN) {
    ofstream outputFile(outputFileN, ios::trunc); // Open file in truncate mode to overwrite
    
    outputFile << fixed << setprecision(1) << time << endl;
    outputFile << optimalLength << endl;
    for (const auto& rect : bestLayout) {
        outputFile << rect.x1 << " " << rect.y1 << " " << rect.x2 << " " << rect.y2 << endl;
    }
    outputFile.close();
}

// Metaheuristic algorithm: Simulated annealing 
void simulated_annealing(const vector<Rectangle>& rectangles, int W, const string& outputFile) {
    const auto start_time = std::chrono::steady_clock::now();

    // Calculate time once 
    auto get_elapsed_time = [&start_time]() {
        return std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start_time).count();
    };

    // Generate initial solution
    auto [current_length, current_layout] = generate_initial_solution(rectangles, W);
    print_solution(get_elapsed_time(), current_length, current_layout, outputFile);
    
    double T = 1000.0; // Initial temperature
    double alpha = 0.99; // Cooling rate
    int k = 0; // Current iteration
    const int max_iterations = 50000;

    // Keep track of global best solution
    int global_best_length = current_length;
    vector<Rectangle> global_best_layout = current_layout;

    while (k <= max_iterations) {
        // Generate neighbor solution
        auto [neighbor_length, neighbor_layout] = random_neighbor(current_layout, W);

        // Acceptance criteria
        if (neighbor_length < current_length) {
            current_layout = neighbor_layout; 
            current_length = neighbor_length;
            // Update global best solution
            if (current_length < global_best_length) {
                global_best_length = current_length;
                global_best_layout = current_layout;
                print_solution(get_elapsed_time(), global_best_length, global_best_layout, outputFile);
            }
        } 
        else {
            // Probability with Boltzmann distribution
            double p = exp(-(neighbor_length - current_length) / T);
            std::uniform_real_distribution<double> probability(0.0, 1.0);
            if (probability(rng) < p) {
                current_layout = neighbor_layout; 
                current_length = neighbor_length; 
            }
        }
        // Update temperature
        T *= alpha;
        ++k;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        return 1;
    }
    ifstream inputFile(argv[1]);

    // Input
    int W, N;
    inputFile >> W >> N;
    vector<Rectangle> rectangles;

    // Read rectangles
    for (int i = 0; i < N; ++i) {
        int ni, pi, qi;
        inputFile >> ni >> pi >> qi;
        for (int j = 0; j < ni; ++j) {
            rectangles.push_back({pi, qi, 0, 0, 0, 0});
        }
        i += ni - 1;
    }
    inputFile.close();

    simulated_annealing(rectangles, W, argv[2]);
    return 0;
}