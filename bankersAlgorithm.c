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

// ====== UTILS ======
void print_system_state() {
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

void print_request_result(bool success, int customerId, int customerDemand[NUMBER_OF_RESOURCES]) {
    printf("Client %d: ", customerId);
    printf("Request %s - ", success ? "allowed" : "denied");
    printf("(%d,%d,%d)\n", customerDemand[0], customerDemand[1], customerDemand[2]);
}

void print_release_result(int customerId, int customeRelease[NUMBER_OF_RESOURCES]) {
    printf("Client %d: resources released - ", customerId);
    printf("(%d,%d,%d)\n", customeRelease[0], customeRelease[1], customeRelease[2]);
}

// ====== SECURITY ALGORITHM ======

bool can_fulfill_need(int customer, int work[]) {
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        if (remainingCustomerNeed[customer][j] > work[j])
            return false;
    }
    return true;
}

void release_allocation(int customer, int work[]) {
    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
        work[j] += currentAllocationOfCustomer[customer][j];
    }
}

bool system_in_safe_state() {
    int work[NUMBER_OF_RESOURCES];
    bool finish[NUMBER_OF_CUSTOMERS] = { false };

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
        work[i] = availableResources[i];

    bool canAllocate;
    while (true) {
        canAllocate = false;
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            if (!finish[i] && can_fulfill_need(i, work)) {
                release_allocation(i, work);
                finish[i] = true;
                canAllocate = true;
            }
        }
        if (!canAllocate) break;
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
        if (!finish[i]) return false;

    return true;
}

// ====== REQUEST RESOURCE ======

bool is_request_valid(int customer, int request[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > remainingCustomerNeed[customer][i] || request[i] > availableResources[i])
            return false;
    }
    return true;
}

void allocate_resources(int customer, int request[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        availableResources[i] -= request[i];
        currentAllocationOfCustomer[customer][i] += request[i];
        remainingCustomerNeed[customer][i] -= request[i];
    }
}

void rollback_allocation(int customer, int request[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        availableResources[i] += request[i];
        currentAllocationOfCustomer[customer][i] -= request[i];
        remainingCustomerNeed[customer][i] += request[i];
    }
}

int request_resources(int customer_num, int request[]) {
    pthread_mutex_lock(&mutex);

    if (!is_request_valid(customer_num, request)) {
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    allocate_resources(customer_num, request);

    if (!system_in_safe_state()) {
        rollback_allocation(customer_num, request);
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    print_system_state();
    pthread_mutex_unlock(&mutex);
    return 0;
}

// ====== RELEASE RESOURCE ======

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
    print_system_state();
    pthread_mutex_unlock(&mutex);
    return 0;
}

// ====== CUSTOMER THREAD ======

void generate_random_request(int customer, int request[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        request[i] = rand() % (remainingCustomerNeed[customer][i] + 1);
    }
}

void generate_random_release(int customer, int release[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        release[i] = rand() % (currentAllocationOfCustomer[customer][i] + 1);
    }
}

void* customerDemandFunction(void* arg) {
    int customer_num = *(int*)arg;
    int request[NUMBER_OF_RESOURCES];
    int release[NUMBER_OF_RESOURCES];

    while(true) {
        pthread_mutex_lock(&mutex);
        generate_random_request(customer_num, request);
        pthread_mutex_unlock(&mutex);

        bool requestSuccess = request_resources(customer_num, request) == 0;
        print_request_result(requestSuccess, customer_num, request);

        sleep(rand() % MAX_TIME);

        pthread_mutex_lock(&mutex);
        generate_random_release(customer_num, release);
        pthread_mutex_unlock(&mutex);

        release_resources(customer_num, release);
        print_release_result(customer_num, release);
    }

    return NULL;
}

// ====== Initialize Properties ======

void init_available_resources(char* argv[]) {
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++){
        availableResources[i] = atoi(argv[i+1]);
    }
}

void define_demand_of_customers() {
    printf("\n--- Customers maximum demands ---\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        printf("Customer %d:", i);
        for (int ii = 0; ii < NUMBER_OF_RESOURCES; ii++) {
            maximumDemandOfCustomer[i][ii] = rand() % (availableResources[ii]);
            currentAllocationOfCustomer[i][ii] = 0;
            remainingCustomerNeed[i][ii] = maximumDemandOfCustomer[i][ii];
            printf(" %d", maximumDemandOfCustomer[i][ii]);
        }
        printf("\n");
    }
    printf("\n");
}

void init_threads(pthread_t threads[NUMBER_OF_CUSTOMERS]) {
    int customer_ids[NUMBER_OF_CUSTOMERS];
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        customer_ids[i] = i;
        pthread_create(&threads[i], NULL, customerDemandFunction, &customer_ids[i]);
    }
}

// ====== MAIN FUNCTION ======

int main(int argc, char* argv[]) {

    if (argc != NUMBER_OF_RESOURCES + 1) {
        fprintf(stderr, "Error passing arguments\n");
        fprintf(stderr, "Use: %s [resource1] [resource2] [resource3]\n", argv[0]);
        exit(1);
    }

    init_available_resources(argv);

    pthread_mutex_init(&mutex, NULL);

    define_demand_of_customers();

    pthread_t threads[NUMBER_OF_CUSTOMERS];
    init_threads(threads);

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}