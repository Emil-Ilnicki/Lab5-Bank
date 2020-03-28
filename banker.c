#define _POSIX_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

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
bool request_res(int n_customer, int request[NUM_RESOURCES]);
bool release_res(int n_customer, int release[NUM_RESOURCES]);
bool check_state();
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
    for (int i = 0; i < NUM_RESOURCES; i++) {
        sum+= need[n_customer][i];
    }

    return sum;

}

void* customer(void* customer_id_ptr) {

    int customer_id = *(int*)(customer_id_ptr);

    while (true) {
        int request[NUM_RESOURCES] = {0};
        int release[NUM_RESOURCES] = {0};
        int sum_request = 0;
        pthread_mutex_lock(&mux);
        do {
           for (int i = 0; i < NUM_RESOURCES; i++) {
               if (need[customer_id][i] == 0) { 
                    continue;
                }

                request[i] = rand() % need[customer_id][i] + (rand() % 2);
                // request[i] = rand() % (
                //     need[customer_id][i] == 1 ? 2 : need[customer_id][i]
                // );

                sum_request += request[i];
            } 
        } while (sum_request == 0);
        
        bool request_outcome = request_res(customer_id, request);

        printf("Customer %d:\tRequest: ", customer_id);
        for (int i = 0; i < NUM_RESOURCES; i++) {
            printf("%d ", request[i]);
        }
        printf("%s\n", request_outcome ? "Safe, granted" : "Unsafe, not granted");

        // exit the thread
        if (sum_need(customer_id) == 0) {
            pthread_mutex_unlock(&mux);
            break;
        }

        int sum_release = 0;
        for (int i = 0; i < NUM_RESOURCES; i++) {
            if (allocation[customer_id][i] == 0) { 
                continue;
            }
            release[i] = rand() % allocation[customer_id][i] + rand() % 2;
            sum_release += release[i];
        }

        if (sum_release > 0) {
            bool release_outcome = release_res(customer_id, release);
            printf("Customer %d:\tRelease: ", customer_id);
            for (int i = 0; i < NUM_RESOURCES; i++) {
                printf("%d ", release[i]);
            }
            printf("%s\n", release_outcome ? "Safe, granted" : "Unsafe, not granted");
        }
        pthread_mutex_unlock(&mux);
    }

    free(customer_id_ptr);
    pthread_exit(0);
}

/*
// Does the acutal bankers algorithm
bool* request_res(void* customer){
    struct customer *cur_customer = (struct customer*)customer;
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < NUM_RESOURCES; i++){
        printf("Thread %d is requesting %d\n", (*cur_customer).id_ptr, (*cur_customer).request[i]);
    }
    pthread_mutex_unlock(&mutex);

    return true;
} */

bool check_state() {

    bool finish[NUM_CUSTOMERS] = {false};
    int work[NUM_RESOURCES];

    for (int i = 0; i < NUM_RESOURCES; i++) {
        work[i] = available[i];
    }

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        for (int j = 0; j < NUM_CUSTOMERS; j++) {

            if (finish[j]) {
                continue;
            }

            bool valid = true;
            for (int k = 0; k < NUM_RESOURCES; k++) {
                
                if (need[j][k] > work[k]) {
                    valid = false;
                    break;
                }
            }

            if (valid) {
                for (int k = 0; k < NUM_RESOURCES; k++) {
                    work[k] += allocation[j][k];
                }

                finish[j] = true;
            }
        }

        bool result = true;
        for (int j = 0; j < NUM_CUSTOMERS; j++) {
            result &= finish[j];
        }

        // if a valid sequence was found
        if (result) {
            return true;
        }
    }

    return false;

}

// Does the acutal bankers algorithm
bool request_res(int n_customer, int request[NUM_RESOURCES]){

    for (int i = 0; i < NUM_RESOURCES; i++){

        if (request[i] > need[n_customer][i]) {
            fprintf(stderr, "Customer %d has requested more than it needs\n", n_customer);
            return false;
        }

        if (request[i] > available[i]) {
            return false;
        }
    }

    // allocate resources for the next state
    for (int i = 0; i < NUM_RESOURCES; i++) {
        allocation[n_customer][i] += request[i];
        available[i] -= request[i];
        need[n_customer][i] -= request[i];
    }

    bool safe_state = check_state();

    // if not safe, deallocate the resources to return to the previous state
    if (!safe_state) {
        for (int i = 0; i < NUM_RESOURCES; i++) {
            allocation[n_customer][i] -= request[i];
            available[i] += request[i];
            need[n_customer][i] += request[i];
        }
    }

    return safe_state;
}

// Changes the value of available (make sure mutex locks are here)
bool release_res(int n_customer, int release[]) {

    for (int i = 0; i < NUM_RESOURCES; i++){
        if (release[i] > allocation[n_customer][i]) {
            fprintf(stderr, "Customer %d is trying to release more than it has allocated\n", n_customer);
            return false;
        }
    }

     // release resources for the next state
    for (int i = 0; i < NUM_RESOURCES; i++) {
        allocation[n_customer][i] -= release[i];
        available[i] += release[i];
        need[n_customer][i] += release[i];
    }

    bool safe_state = check_state();

    if (!safe_state) {
    
        for (int i = 0; i < NUM_RESOURCES; i++) {
            allocation[n_customer][i] += release[i];
            available[i] -= release[i];
            need[n_customer][i] -= release[i];
        }
    }
    return safe_state;
}


int main(int argc, char *argv[]){

    srand(time(NULL));
    pthread_t threads[NUM_CUSTOMERS];
    pthread_mutex_init(&mux, NULL);

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        for (int j = 0; j < NUM_RESOURCES; j++) {
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }

    if (argc != NUM_RESOURCES + 1) {
        fprintf(stderr, "Invalid input. Needs the available resources inputted\n");
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