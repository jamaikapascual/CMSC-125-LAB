// EXER 07 B - SHARED MEMORY MATRIX MULTIPLICATION READER

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SHARED_MEM_SIZE 100
#define SHARED_MEM_KEY 5678

// Function to print the matrix
void displayMatrix(int *matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

// Function to access shared memory and handle errors
int* accessSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, 0666);
    if (shmid < 0) {
        perror("Failed to access shared memory");
        exit(EXIT_FAILURE);
    }

    int *sharedMemory = (int *)shmat(shmid, NULL, 0);
    if (sharedMemory == (int *)-1) {
        perror("Failed to attach shared memory");
        exit(EXIT_FAILURE);
    }

    return sharedMemory;
}

// Function to retrieve matrix sizes and result matrix from shared memory
void retrieveMatrixSizes(int *sharedMemory, int *rowsA, int *colsA, int *rowsB, int *colsB) {
    *rowsA = sharedMemory[0];
    *colsA = sharedMemory[1];
    *rowsB = sharedMemory[2];
    *colsB = sharedMemory[3];
}

// Main function to handle matrix retrieval and display
int main() {
    int rowsA, colsA, rowsB, colsB;

    // Access shared memory
    int *sharedMemory = accessSharedMemory(SHARED_MEM_KEY, SHARED_MEM_SIZE);

    // Retrieve matrix dimensions from shared memory
    retrieveMatrixSizes(sharedMemory, &rowsA, &colsA, &rowsB, &colsB);

    // Allocate memory for the result matrix
    int *resultMatrix = (int *)malloc(rowsA * colsB * sizeof(int));
    if (resultMatrix == NULL) {
        perror("Memory allocation for result matrix failed");
        shmdt(sharedMemory);
        exit(EXIT_FAILURE);
    }

    // Copy result matrix from shared memory
    for (int i = 0; i < rowsA * colsB; i++) {
        resultMatrix[i] = sharedMemory[i + 4];
    }

    // Wait for the completion flag from the other process
    while (sharedMemory[rowsA * colsB + 4] != -1) {
        sleep(1);
    }

    // Display the resulting matrix
    printf("Resulting matrix:\n");
    displayMatrix(resultMatrix, rowsA, colsB);

    // Clean up resources
    free(resultMatrix);
    shmdt(sharedMemory);

    return 0;
}
