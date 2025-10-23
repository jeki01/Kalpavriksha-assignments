#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function to swap two integers
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to generate random N×N matrix
void generateMatrix(int *matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            *(matrix + i * n + j) = rand() % 256; // 0–255
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
            swap(&(*(matrix + i * n + j)), &(*(matrix + j * n + i)));
        }
    }

    // Reverse each row
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n / 2; j++) {
            swap(&(*(matrix + i * n + j)), &(*(matrix + i * n + (n - 1 - j))));
        }
    }
}

// Function to apply 3×3 smoothing filter 
void applySmoothing(int *matrix, int n) {
    for (int i = 1; i < n - 1; i++) {
        for (int j = 1; j < n - 1; j++) {
            int sum = 0, count = 0;

            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    sum += *(matrix + (i + x) * n + (j + y));
                    count++;
                }
            }
            *(matrix + i * n + j) = sum / count;
        }
    }
}

int main() {
    int n;

    // Retry loop for valid size input
    do {
        printf("Enter matrix size (2 to 10): ");
        scanf("%d", &n);
        if (n < 2 || n > 10) {
            printf("Invalid size! Please enter between 2 and 10.\n");
        }
    } while (n < 2 || n > 10);

    // Allocate memory
    int *matrix = (int *)malloc(n * n * sizeof(int));
    if (matrix == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    srand(time(0));

    // Function calls
    generateMatrix(matrix, n);
    printMatrix(matrix, n, "Original Matrix:");

    rotateMatrix(matrix, n);
    printMatrix(matrix, n, "Matrix after 90 degree rotation:");

    applySmoothing(matrix, n);
    printMatrix(matrix, n, "Matrix after 3x3 smoothing filter:");

    free(matrix);
    return 0;
}
