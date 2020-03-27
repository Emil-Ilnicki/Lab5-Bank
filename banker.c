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

pthread_mutex_t mux;

// Customer functions
bool request_res(int n_customer, int request[]);
bool release_res(int n_customer, int release[]);
void fill_matrix();



void print_table() {
    printf("Allocation       Max       Need\n");
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
            printf("%d %d %d            %d %d %d     %d %d %d\n", 
            allocation[i][0],allocation[i][1],allocation[i][2],
            max[i][0],max[i][1],max[1][2],
            need[i][0],need[i][1],need[i][2]);
    }
}

void fill_matrix(){

    for (int i = 0; i < NUM_CUSTOMERS; i++){
        for (int j = 0; j < NUM_RESOURCES; j++){
            allocation[i][j] = 0;
            max[i][j] = (rand() % available[j]);
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }
   
}

int sum_need(int n_customer) {
    int sum = 0;
    pthread_mutex_lock(&mux);
    for (int i = 0; i < NUM_RESOURCES; i++) {
        sum+= need[n_customer][i];
    }
    pthread_mutex_unlock(&mux);

    return sum;

}

void* customer(void* customer_id_ptr) {

    int customer_id = *(int*)(customer_id_ptr);

    printf("Customer Id: %d\n", customer_id);

    while(true) {

        int request[NUM_RESOURCES];
        int sum_request = 0;
        while (sum_request == 0) {
           for (int i = 0; i < NUM_RESOURCES; i++) {
               if (need[customer_id][i] == 0) { 
                    continue;
                }
                request[i] = rand() % need[customer_id][i];
                sum_request += request[i];
            } 
        }
        
        bool request_outcome = request_res(customer_id, request);

        if (sum_need(customer_id) > 0) {
            bool release_outcome = release_res(customer_id, allocation[customer_id]);
            // TODO print
            break;
        }

        //TODO print
    }

    free(customer_id_ptr);
    pthread_exit(0);
}



// // Does the acutal bankers algorithm
// bool request_res(int n_customeyor, int request[]){

//     for (int i = 0; i < NUM_RESOURCES; i++){
//         printf("%d", request[i]);
//     }
// }

// // Changes the value of available (make sure mutex locks are here)
// bool release_res(int n_customer, int release[]){

// }


int main(int argc, char *argv[]){

    pthread_t threads[NUM_CUSTOMERS];

    if (argc != NUM_RESOURCES + 1) {
        fprintf(stderr, "Invalid input. Needs the available resources inputted");
        exit(1);
    }
    
    
    for (int i = 1; i < argc; i++){
        available[i-1] = atoi(argv[i]);
    }
    
    fill_matrix();

    for (int i = 0; i < NUM_CUSTOMERS; i++){

        int* customer_id = malloc(sizeof(int));
        *customer_id = i;

        int rc = pthread_create(&threads[i], NULL, customer, customer_id);
        if (rc > 0){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            exit(1);
        }
    }

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL);
    }

}