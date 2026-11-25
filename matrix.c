#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 20                      // matrix dimension (20 x 20)
#define NUM_THREADS 10              // total threads used for all three operations
#define ROWS_PER_THREAD (MAX / NUM_THREADS)   // 2 rows per thread

int matA[MAX][MAX];
int matB[MAX][MAX];

int matSumResult[MAX][MAX];
int matDiffResult[MAX][MAX];
int matProductResult[MAX][MAX];

// Describes the block of rows [startRow, endRow) that one thread is responsible for.
typedef struct {
    int threadNum;
    int startRow;
    int endRow;   // exclusive
} RowRange;

void fillMatrix(int matrix[MAX][MAX]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            matrix[i][j] = rand()%10+1;
        }
    }
}

void printMatrix(int matrix[MAX][MAX]) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the sum of the
// values at the coordinates of matA and matB.
void* computeSum(void* args) {
    RowRange* r = (RowRange*)args;
    for (int i = r->startRow; i < r->endRow; i++)
        for (int j = 0; j < MAX; j++)
            matSumResult[i][j] = matA[i][j] + matB[i][j];
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matDiffResult at the coordinates to the difference of the
// values at the coordinates of matA and matB.
void* computeDiff(void* args) {
    RowRange* r = (RowRange*)args;
    for (int i = r->startRow; i < r->endRow; i++)
        for (int j = 0; j < MAX; j++)
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matProductResult at the coordinates to the inner product
// of row i of matA and column j of matB.
void* computeProduct(void* args) {
    RowRange* r = (RowRange*)args;
    for (int i = r->startRow; i < r->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            int dot = 0;
            for (int k = 0; k < MAX; k++)
                dot += matA[i][k] * matB[k][j];
            matProductResult[i][j] = dot;
        }
    }
    return NULL;
}

// Each of the 10 threads runs this. It handles its own block of rows
// for all three operations, then frees the argument it was given.
void* worker(void* args) {
    RowRange* r = (RowRange*)args;
    computeSum(r);
    computeDiff(r);
    computeProduct(r);
    free(r);
    return NULL;
}

// Spawns NUM_THREADS (10) threads. Each thread is given a 2-row block
// (20 rows / 10 threads) and computes Sum, Difference, and Product for it.
int main() {
    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for our threads.
    pthread_t threads[NUM_THREADS];

    // 4. Create the threads, each with its own malloc'd row range.
    for (int t = 0; t < NUM_THREADS; t++) {
        RowRange* r = malloc(sizeof(RowRange));
        if (r == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        r->threadNum = t;
        r->startRow = t * ROWS_PER_THREAD;
        r->endRow = r->startRow + ROWS_PER_THREAD;

        if (pthread_create(&threads[t], NULL, worker, r) != 0) {
            perror("pthread_create");
            free(r);
            exit(EXIT_FAILURE);
        }
    }

    // 5. Wait for all threads to finish.
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // 6. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);
    return 0;
}
