#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_SIZE 2
#define MAX_SIZE 10

// Swap two values using pointers
void swap(unsigned short *firstValue, unsigned short *secondValue) {
    unsigned short temp = *firstValue;
    *firstValue = *secondValue;
    *secondValue = temp;
}

// Generate random matrix values (0–255)
void generateMatrix(unsigned short *matrixPtr, int matrixSize) {
    for (int index = 0; index < matrixSize * matrixSize; index++) {
        *(matrixPtr + index) = rand() % 256;
    }
}

// Display matrix with title
void printMatrix(unsigned short *matrixPtr, int matrixSize, const char *title) {
    printf("\n%s\n", title);
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++) {
            printf("%4hu ", *(matrixPtr + row * matrixSize + col));
        }
        printf("\n");
    }
}

// Rotate matrix 90° clockwise (in-place)
void rotateMatrix(unsigned short *matrixPtr, int matrixSize) {
    for (int row = 0; row < matrixSize; row++) {
        for (int col = row + 1; col < matrixSize; col++) {
            swap((matrixPtr + row * matrixSize + col), (matrixPtr + col * matrixSize + row));
        }
    }
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize / 2; col++) {
            swap((matrixPtr + row * matrixSize + col),
                 (matrixPtr + row * matrixSize + (matrixSize - 1 - col)));
        }
    }
}

// Apply 3×3 smoothing filter using pointer access
void applySmoothing(unsigned short *matrixPtr, int matrixSize) {
    unsigned short *tempRow = (unsigned short *)malloc(matrixSize * sizeof(unsigned short));
    if (!tempRow) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Create a copy of original matrix row-by-row temporarily (O(n))
    unsigned short *previousRow = (unsigned short *)malloc(matrixSize * sizeof(unsigned short));
    if (!previousRow) {
        free(tempRow);
        printf("Memory allocation failed.\n");
        return;
    }

    // Process first row separately
    for (int row = 0; row < matrixSize; row++) {
        for (int col = 0; col < matrixSize; col++) {
            int sum = 0, count = 0;

            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    int nr = row + dr, nc = col + dc;
                    if (nr >= 0 && nr < matrixSize && nc >= 0 && nc < matrixSize) {
                        sum += *(matrixPtr + nr * matrixSize + nc);
                        count++;
                    }
                }
            }
            *(tempRow + col) = (unsigned short)(sum / count);
        }

        // Store current smoothed row in previousRow to avoid overwrite effect
        for (int col = 0; col < matrixSize; col++) {
            *(previousRow + col) = *(tempRow + col);
        }

        // Update the matrix row after full row computation
        for (int col = 0; col < matrixSize; col++) {
            *(matrixPtr + row * matrixSize + col) = *(previousRow + col);
        }
    }

    free(tempRow);
    free(previousRow);
}

int main() {
    int matrixSize;

    do {
        printf("Enter matrix size (%d to %d): ", MIN_SIZE, MAX_SIZE);
        scanf("%d", &matrixSize);
        if (matrixSize < MIN_SIZE || matrixSize > MAX_SIZE)
            printf("Invalid size! Please enter between %d and %d.\n", MIN_SIZE, MAX_SIZE);
    } while (matrixSize < MIN_SIZE || matrixSize > MAX_SIZE);

    unsigned short *matrixPtr = (unsigned short *)malloc(matrixSize * matrixSize * sizeof(unsigned short));
    if (!matrixPtr) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    srand(time(0));

    generateMatrix(matrixPtr, matrixSize);
    printMatrix(matrixPtr, matrixSize, "Original Randomly Generated Matrix:");

    rotateMatrix(matrixPtr, matrixSize);
    printMatrix(matrixPtr, matrixSize, "Matrix after 90° Clockwise Rotation:");

    applySmoothing(matrixPtr, matrixSize);
    printMatrix(matrixPtr, matrixSize, "Matrix after Applying 3x3 Smoothing Filter:");

    free(matrixPtr);
    return 0;
}
