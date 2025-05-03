#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3
#define MAX_TIME 2001

int availableResources[NUMBER_OF_RESOURCES];
int maximumDemandOfCustomer[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int currentAllocationOfCustomer[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int remainingCustomerNeed[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

pthread_mutex_t mutex;

void printSystemState() {
    printf("\n----- System Current State ----\n");
    printf("Available Resources: ");
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        printf("%d ", availableResources[i]);
    }
    printf("\n");

    printf("Client Allocation \t/\tCustomer Need:\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        printf("Customer %d: ", i);
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            printf("%d ", currentAllocationOfCustomer[i][j]);
        }
        printf("\t/\t");
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            printf("%d ", remainingCustomerNeed[i][j]);
        }
        printf("\n");
    }
    printf("--------------------------------\n");
}

bool systemInSafeState() {
    int work[NUMBER_OF_RESOURCES];
    int finish[NUMBER_OF_CUSTOMERS] = { false };

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        work[i] = availableResources[i];
    }

    int canAllocate;
    while (true) {
        canAllocate = false;
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            if (!finish[i]) {
                int canFulfill = true;
                for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                    if (remainingCustomerNeed[i][j] > work[j]) {
                        canFulfill = false;
                        break;
                    }
                }
                if (canFulfill) {
                    for (int j = 0; j < NUMBER_OF_RESOURCES; j++){
                        work[j] += currentAllocationOfCustomer[i][j];
                    }
                    finish[i] = true;
                    canAllocate = true;
                }
            }
        }
        if (!canAllocate) break;
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        if (!finish[i]) return false;
    }  

    return true;
}

int request_resources(int customer_num, int request[]) {
    pthread_mutex_lock(&mutex);

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > remainingCustomerNeed[customer_num][i] || request[i] > availableResources[i]) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        availableResources[i] -= request[i];
        currentAllocationOfCustomer[customer_num][i] += request[i];
        remainingCustomerNeed[customer_num][i] -= request[i];
    }

    if (!systemInSafeState()) {
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            availableResources[i] += request[i];
            currentAllocationOfCustomer[customer_num][i] -= request[i];
            remainingCustomerNeed[customer_num][i] += request[i];
        }
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    printSystemState();
    pthread_mutex_unlock(&mutex);
    return 0;
}

int release_resources(int customer_num, int release[NUMBER_OF_RESOURCES]) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (release[i] > currentAllocationOfCustomer[customer_num][i]) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
        availableResources[i] += release[i];
        currentAllocationOfCustomer[customer_num][i] -= release[i];
        remainingCustomerNeed[customer_num][i] += release[i];
    }
    printSystemState();
    pthread_mutex_unlock(&mutex);
    return 0;
}

void printRequestResult(bool success, int customerId, int customerDemand[NUMBER_OF_RESOURCES]) {
    printf("Cliente %d: ", customerId);
    printf("solicitação %s - ", success ? "atendida" : "negada");
    printf("(%d,%d,%d)\n", customerDemand[0], customerDemand[0], customerDemand[0]);
}

void printReleaseResult(int customerId, int customeRelease[NUMBER_OF_RESOURCES]) {
    printf("Cliente %d: recursos liberados - ", customerId);
    printf("(%d,%d,%d)\n", customeRelease[0], customeRelease[0], customeRelease[0]);
}

void* customerDemandFunction(void* arg) {
    int customer_num = *(int*)arg;
    int request[NUMBER_OF_RESOURCES];
    int release[NUMBER_OF_RESOURCES];

    while(true) {
        pthread_mutex_lock(&mutex);

        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            request[i] = rand() % (remainingCustomerNeed[customer_num][i] + 1);
        }
            
        pthread_mutex_unlock(&mutex);

        bool requestSuccess = request_resources(customer_num, request) == 0;
        printRequestResult(requestSuccess, customer_num, request);

        sleep(rand() % MAX_TIME);

        pthread_mutex_lock(&mutex);

        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            release[i] = rand() % (currentAllocationOfCustomer[customer_num][i] + 1);
        }
    
        pthread_mutex_unlock(&mutex);

        release_resources(customer_num, release);
    }

    return NULL;
}

void initAvailableResources(char* argv[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++){
        availableResources[i] = atoi(argv[i+1]);
    }
}

void defineDemandOfCustomers() {
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int ii = 0; ii < NUMBER_OF_RESOURCES; ii++) {
            maximumDemandOfCustomer[i][ii] = rand() % (availableResources[ii]);
            currentAllocationOfCustomer[i][ii] = 0;
            remainingCustomerNeed[i][ii] = maximumDemandOfCustomer[i][ii];
        }
    }
}

void setupThreads(pthread_t threads[NUMBER_OF_CUSTOMERS]) {
    int customer_ids[NUMBER_OF_CUSTOMERS];
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        customer_ids[i] = i;
        pthread_create(&threads[i], NULL, customerDemandFunction, &customer_ids[i]);
    }
}

int main(int argc, char* argv[]) {

    if (argc != NUMBER_OF_RESOURCES + 1) {
        fprintf(stderr, "Error passing arguments\n");
        fprintf(stderr, "Use: %s [resource1] [resource2] [resource3]\n", argv[0]);
        exit(1);
    }

    initAvailableResources(argv);

    pthread_mutex_init(&mutex, NULL);

    defineDemandOfCustomers();

    pthread_t threads[NUMBER_OF_CUSTOMERS];
    setupThreads(threads);

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}