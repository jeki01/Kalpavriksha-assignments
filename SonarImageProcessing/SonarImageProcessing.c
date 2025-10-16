#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to generate random N×N matrix
void generateMatrix(int *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int value = rand() % 256; 
            *(matrix + i * n + j) = value;
        }
    }
}

// Function to print matrix
void printMatrix(int *matrix, int n, const char *msg) {
    printf("\n%s\n", msg);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%4d ", *(matrix + i * n + j));
        }
        printf("\n");
    }
}

// Function to rotate matrix 90° clockwise
void rotateMatrix(int *matrix, int n) {
    // Transpose
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            int temp = *(matrix + i * n + j);
            *(matrix + i * n + j) = *(matrix + j * n + i);
            *(matrix + j * n + i) = temp;
        }
    }

    // Reverse each row
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n / 2; j++) {
            int temp = *(matrix + i * n + j);
            *(matrix + i * n + j) = *(matrix + i * n + (n - 1 - j));
            *(matrix + i * n + (n - 1 - j)) = temp;
        }
    }
}

// Function to apply 3×3 smoothing filter
void applySmoothing(int *matrix, int n) {
    int *temp = (int *)malloc(n * n * sizeof(int));

    // For each element, calculate the average of valid neighbors
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            int count = 0;

            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    int ni = i + x;
                    int nj = j + y;

                    // Check if neighbor exists (no out of bound)
                    if (ni >= 0 && ni < n && nj >= 0 && nj < n) {
                        sum += *(matrix + ni * n + nj);
                        count++;
                    }
                }
            }

            int avg = sum / count;

        
            if (avg < 0) avg = 0;
            if (avg > 255) avg = 255;

            *(temp + i * n + j) = avg;
        }
    }

    // Copy smoothed values back to matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            *(matrix + i * n + j) = *(temp + i * n + j);
        }
    }

    free(temp);
}

int main() {
    int n;

    printf("Enter matrix size (2 to 10): ");
    scanf("%d", &n);

    // Constraint check for matrix size
    if (n < 2 || n > 10) {
        printf("Invalid size! Please enter between 2 and 10.\n");
        return 0;
    }

    // Allocate memory
    int *matrix = (int *)malloc(n * n * sizeof(int));
    srand(time(0)); // Random seed

    // Function calls
    generateMatrix(matrix, n);
    printMatrix(matrix, n, "Original Matrix:");

    rotateMatrix(matrix, n);
    printMatrix(matrix, n, "Matrix after 90 degree rotation:");

    applySmoothing(matrix, n);
    printMatrix(matrix, n, "Matrix after 3x3 smoothing filter:");

    // Free memory
    free(matrix);
    return 0;
}
