#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

pthread_mutex_t mutex;

// May be any values >= 0
#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 3
// Available amount of each resource
int available[NUM_RESOURCES];
// Maximum demand of each customer
int max[NUM_CUSTOMERS][NUM_RESOURCES];
// Amount currently allocated to each customer
int allocation[NUM_CUSTOMERS][NUM_RESOURCES];
// Remaining need of each customer (max-allocation)
int need[NUM_CUSTOMERS][NUM_RESOURCES];

// Customer functions
bool request_res(int n_customer, int request[]);
bool release_res(int n_customer, int release[]);

void fill_matrix();


int main(int argc, char *argv[]){

    // pthread_t threads[NUM_CUSTOMERS];
    // int rc;

    for (int i = 1; i < argc; i++){
        available[i] =  atoi(argv[i]);
    }
    
    fill_matrix();

    // while (true){
    //     for (int i = 0; i < NUM_CUSTOMERS; i++){
    //         rc = pthread_create(&threads[i], request_res, i, allocation);
    //         if (rc){
    //             printf("ERROR; return code from pthread_create() is %d\n", rc);
    //             exit(-1);
    //         }
    //     }
    //     break;
    // }
}

void fill_matrix(){

    for (int i = 0; i < NUM_CUSTOMERS; i++){
        for (int j = 0; j < NUM_RESOURCES; j++){
            allocation[i][j] = (rand() % 4);
            max[i][j] = (rand() % 7) + 3;
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }

    printf("Allocation     Max     Need\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++){
            printf("%d%d%d            %d%d%d     %d%d%d\n", allocation[i][0],allocation[i][1],allocation[i][2],max[i][0],max[i][1],max[1][2],need[i][0],need[i][1],need[i][2]);
    }
}

// // Does the acutal bankers algorithm
// bool request_res(int n_customer, int request[]){

//     for (int i = 0; i < NUM_RESOURCES; i++){
//         printf("%d", request[i]);
//     }
// }

// // Changes the value of available (make sure mutex locks are here)
// bool release_res(int n_customer, int release[]){

// }