#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

const int MAX_PIXEL_VALUE = 256;

void runSonarMatrixSimulation();
int getValidMatrixSize();
bool isValidMatrixSize(int matrixSize);
void generateSonarImage(int* sonarImage, int matrixSize);
void displaySonarImage(int* sonarImage, int matrixSize);
void rotateSonarImage(int* sonarImage, int matrixSize);
void swap4ClockWise(int* top, int* right, int* bottom, int* left);
void smoothingSonarImage(int* sonarImage, int matrixSize);
int getNeighborAverage(int* sonarImage, int matrixSize, int row, int col);
void encodeNewValue(int* sonarImage, int row, int col, int newValue, int matrixSize);
void decodeMatrix(int* sonarImage, int matrixSize);

int main()
{
  runSonarMatrixSimulation();
  return 0;
}

void runSonarMatrixSimulation()
{
  int matrixSize = getValidMatrixSize();

  int* sonarImage = (int*)malloc(matrixSize * matrixSize * sizeof(int));
  if (sonarImage == NULL)
  {
    printf("Memory allocation failed!\n");
    return;
  }

  generateSonarImage(sonarImage, matrixSize);
  printf("Original\n");
  displaySonarImage(sonarImage, matrixSize);

  printf("Rotated:\n");
  rotateSonarImage(sonarImage, matrixSize);
  displaySonarImage(sonarImage, matrixSize);

  printf("Final Output:\n");
  smoothingSonarImage(sonarImage, matrixSize);
  displaySonarImage(sonarImage, matrixSize);

  free(sonarImage);
}

int getValidMatrixSize()
{
  int matrixSize;
  while (1)
  {
    printf("Enter side of sonar image matrix: ");
    scanf("%d", &matrixSize);

    if (isValidMatrixSize(matrixSize))
    {
      return matrixSize;
    }

    printf("Matrix size must be between 2 and 10.\n");
  }
}

bool isValidMatrixSize(int matrixSize)
{
  return (matrixSize >= 2 && matrixSize <= 10);
}

void generateSonarImage(int* sonarImage, int matrixSize)
{
  srand(time(0));

  for (int i = 0; i < matrixSize; i++)
  {
    for (int j = 0; j < matrixSize; j++)
    {
      *(sonarImage + i * matrixSize + j) = rand() % MAX_PIXEL_VALUE;
    }
  }
}

void displaySonarImage(int* sonarImage, int matrixSize)
{
  for (int i = 0; i < matrixSize; i++)
  {
    for (int j = 0; j < matrixSize; j++)
    {
      printf("%3d ", *(sonarImage + i * matrixSize + j));
    }
    printf("\n");
  }
  printf("\n");
}

void rotateSonarImage(int* sonarImage, int matrixSize)
{
  for (int layer = 0; layer < matrixSize / 2; layer++)
  {
    int first = layer;
    int last = matrixSize - 1 - layer;

    for (int i = first; i < last; i++)
    {
      int offset = i - first;

      int* top = sonarImage + first * matrixSize + i;
      int* right = sonarImage + i * matrixSize + last;
      int* bottom = sonarImage + last * matrixSize + (last - offset);
      int* left = sonarImage + (last - offset) * matrixSize + first;

      swap4ClockWise(top, right, bottom, left);
    }
  }
}

void swap4ClockWise(int* top, int* right, int* bottom, int* left)
{
  int temp = *top;
  *top = *left;
  *left = *bottom;
  *bottom = *right;
  *right = temp;
}

void smoothingSonarImage(int* sonarImage, int matrixSize)
{
  for (int row = 0; row < matrixSize; row++)
  {
    for (int col = 0; col < matrixSize; col++)
    {
      int avg = getNeighborAverage(sonarImage, matrixSize, row, col);
      encodeNewValue(sonarImage, row, col, avg, matrixSize);
    }
  }

  decodeMatrix(sonarImage, matrixSize);
}

int getNeighborAverage(int* sonarImage, int matrixSize, int row, int col)
{
  int sum = 0, count = 0;
  for (int deltaRow = -1; deltaRow <= 1; deltaRow++)
  {
    for (int deltaCol = -1; deltaCol <= 1; deltaCol++)
    {
      int newRow = row + deltaRow;
      int newCol = col + deltaCol;
      if (newRow >= 0 && newRow < matrixSize && newCol >= 0 && newCol < matrixSize)
      {
        int neighborValue = *(sonarImage + newRow * matrixSize + newCol) % MAX_PIXEL_VALUE;
        sum += neighborValue;
        count++;
      }
    }
  }
  return sum / count;
}

void encodeNewValue(int* sonarImage, int row, int col, int newValue, int matrixSize)
{
  *(sonarImage + row * matrixSize + col) += newValue * MAX_PIXEL_VALUE;
}

void decodeMatrix(int* sonarImage, int matrixSize)
{
  for (int row = 0; row < matrixSize; row++)
  {
    for (int col = 0; col < matrixSize; col++)
    {
      *(sonarImage + row * matrixSize + col) /= MAX_PIXEL_VALUE;
    }
  }

}
