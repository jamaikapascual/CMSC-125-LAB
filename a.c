// EXER 07 A - SHARED MEMORY MATRIX MULTIPLICATION

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHARED_MEM_SIZE 100
#define SHARED_MEM_KEY 5678

// Function to read matrix elements from file
void readMatrixFromFile(FILE *file, int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%d", &matrix[i * cols + j]);
        }
    }
}

// Function to allocate memory and handle errors
int* allocateMatrix(int rows, int cols) {
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    if (matrix == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    return matrix;
}

// Function to multiply matrices
void multiplyMatrices(int *matrixA, int *matrixB, int *resultMatrix, int rowsA, int colsA, int colsB) {
    for (int i = 0; i < rowsA; i++) {
        for (int j = 0; j < colsB; j++) {
            resultMatrix[i * colsB + j] = 0;
            for (int k = 0; k < colsA; k++) {
                resultMatrix[i * colsB + j] += matrixA[i * colsA + k] * matrixB[k * colsB + j];
            }
        }
    }
}

// Function to set up shared memory and handle errors
int* setupSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Shared memory allocation failed");
        exit(EXIT_FAILURE);
    }

    int *shmPtr = (int *)shmat(shmid, NULL, 0);
    if (shmPtr == (int *)-1) {
        perror("Shared memory attachment failed");
        exit(EXIT_FAILURE);
    }

    return shmPtr;
}

int main() {
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Read matrix A dimensions and elements
    int rowsA, colsA;
    fscanf(file, "%d %d", &rowsA, &colsA);
    int *matrixA = allocateMatrix(rowsA, colsA);
    readMatrixFromFile(file, matrixA, rowsA, colsA);

    // Read matrix B dimensions and elements
    int rowsB, colsB;
    fscanf(file, "%d %d", &rowsB, &colsB);
    int *matrixB = allocateMatrix(rowsB, colsB);
    readMatrixFromFile(file, matrixB, rowsB, colsB);
    fclose(file);

    // Check if matrix multiplication is possible
    if (colsA != rowsB) {
        fprintf(stderr, "Matrix multiplication not possible (incompatible dimensions).\n");
        free(matrixA);
        free(matrixB);
        return EXIT_FAILURE;
    }

    // Setup shared memory and store matrix dimensions
    int *sharedMemory = setupSharedMemory(SHARED_MEM_KEY, SHARED_MEM_SIZE);
    sharedMemory[0] = rowsA;
    sharedMemory[1] = colsA;
    sharedMemory[2] = rowsB;
    sharedMemory[3] = colsB;

    // Perform matrix multiplication and store results in shared memory
    int *resultMatrix = allocateMatrix(rowsA, colsB);
    multiplyMatrices(matrixA, matrixB, resultMatrix, rowsA, colsA, colsB);
    
    // Copy result to shared memory
    for (int i = 0; i < rowsA * colsB; i++) {
        sharedMemory[i + 4] = resultMatrix[i];
    }
    
    // Signal completion
    sharedMemory[rowsA * colsB + 4] = -1;

    // Clean up
    shmdt(sharedMemory);
    free(matrixA);
    free(matrixB);
    free(resultMatrix);

    return 0;
}
