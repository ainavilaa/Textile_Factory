#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <fstream> 
using namespace std;

struct Rectangle {
   int width, height;
   int x1, y1, x2, y2;
};

// Sort rectangles
bool compareRectangles(const Rectangle &a, const Rectangle &b) {
   if (max(a.height, a.width) != max(b.height, b.width))
       return max(a.height, a.width) > max(b.height, b.width);
   return min(a.height, a.width) > min(b.height, b.width);
}

// Check if a rectangle can be placed
bool canPlace(const vector<vector<bool>>& matrix, int x, int y, int width, int height) {
   int L = int(matrix.size());
   int W = int(matrix[0].size());
   
   // Check bounds
   if (x < 0 || x + width > W || y < 0 || y + height > L) {
       return false;
   }
   
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

int main(int argc, char* argv[]) {
    if (argc != 3) return 1;
    ifstream inputFile(argv[1]);
    ofstream outputFile(argv[2]);

    double timeStart = clock();
    
    int W, N;
    inputFile >> W >> N;
    vector<Rectangle> rectangles;
    
    for (int i = 0; i < N; ++i) {
        int ni, pi, qi;
        inputFile >> ni >> pi >> qi;
        for (int j = 0; j < ni; ++j) {
            rectangles.push_back({pi, qi, 0, 0, 0, 0});
        }
        i += ni - 1;
    }

    sort(rectangles.begin(), rectangles.end(), compareRectangles);
    // Cloth roll
    vector<vector<bool>> matrix(1, vector<bool>(W, false));
    placeRectangles(rectangles, W, matrix);
    
    int L = int(matrix.size()); // Roll length
    
    double timeEnd = clock();
    double timeTaken = (timeEnd - timeStart) / CLOCKS_PER_SEC;
    
    outputFile << fixed << setprecision(1) << timeTaken << endl;
    outputFile << L << endl;
    
    for (const auto& rect : rectangles) {
        outputFile << rect.x1 << " " << rect.y1 << " " << rect.x2 << " " << rect.y2 << endl;
    }
    inputFile.close();
    outputFile.close();
    return 0;
}