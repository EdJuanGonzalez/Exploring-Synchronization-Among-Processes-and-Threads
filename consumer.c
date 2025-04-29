#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdbool.h>
#include "product.h"

//size of buffers
#define BUFFER_SIZE_A 20
#define BUFFER_SIZE_B 15

/*
    struct buffer: circular buffer for putting and getting products

    @member Product*array: an array of products
    @member int input: index in array to put the next product
    @member int output: index in array to get the current product
    @member int count: the amount of variables in the array
    @member int arraySize: size of the Product array
    @member int consumerCount: keeps track of the total amount of times a product is retreived
    @member int killswitch: a killswitched used to end the second thread of each type
    @member pthread_cond_t full: conditional used to signal when buffer is full or not
    @member pthread_cond_t empty: conditional used to signal when buffer is empty or not
    @member pthread_mutex_t mutex: used to locking critial sections in get and put functions
    @member  pthread_mutex_t consumerMutex: mutex specifically for the consumerCount variable
    @member FILE *file: file pointer to output file
*/
typedef struct {
    Product *array;
    int input;
    int output;
    int count;
    int arraySize;
    int consumerCount;
    int killswitch;
    pthread_cond_t full;
    pthread_cond_t empty;
    pthread_mutex_t mutex;
    pthread_mutex_t consumerMutex;
    FILE *file;
} Buffer;

/*
    struct OuputStruct: used to hold variables required for output file

    @member pid_t tid: the pid of the thread
    @member productType: the type of prodcut
    @member poduct_sequence: the order of product creations (doubles as product ID)
    @member consumerCount: the total amount of products consumed
*/
typedef struct {
    char productType[11];
    pthread_t tid;
    int product_sequence;
    int consumerCount;
} OutputStruct;


/*
    struct Thread_args: used to send parameters to threads

    @member Buffer *bufferA: the buffer for products of type A
    @member Buffer *bufferB: the buffer for products of type B
*/
typedef struct {
    Buffer *bufferA;
    Buffer *bufferB;
} Thread_args;

/*
    put_in_buffer:
        The function that inputs products into their respective buffers
        increases the count in the buffer by one to account for new entry
        increments the input variable to mimic circular buffer

    @param Product product: a specific product sent from producer
    @param Buffer *buffer: a pointer to the correct buffer
*/
void put_in_buffer(Product product, Buffer *buffer) {
    buffer->array[buffer->input] = product;
    buffer->input = (buffer->input + 1) % buffer->arraySize;
    buffer->count = buffer->count + 1;
}//end put_in_buffer

/*
    get_productType:
        The function used to get a product from a specific buffer
        decrease the count in the buffer by one to account for the "removal" of an entry
        increments the output variable to mimic circular buffer
    
    @param Buffer *buffer: a pointer to the correct buffer

    @return: returns the first product available in the buffer
*/
Product get_product(Buffer *buffer) {
    int temp = buffer->output;
    buffer->output = (buffer->output + 1) % buffer->arraySize;
    buffer->count = buffer->count - 1;
    pthread_mutex_lock(&buffer->consumerMutex);
    buffer->consumerCount++;
    pthread_mutex_unlock(&buffer->consumerMutex);
    return buffer->array[temp];
}//end get_product

/*
    initialize_buffer:
        The function that initializes a buffer
    
    @param Buffer *buffer: a pointer to the buffer being initialized
    @param int buffer_size: the size of the array in the buffer
    @param FILE *file: a file pointer to the output file
*/
void initialize_buffer(Buffer *buffer, int buffer_size, FILE *file) {
    buffer->array = (Product *) calloc(buffer_size, sizeof(Product));
    buffer->input = 0;
    buffer->output = 0;
    buffer->count = 0;
    buffer->arraySize = buffer_size;
    buffer->consumerCount = 0;
    buffer->killswitch = 0;
    pthread_mutex_init(&buffer->mutex, NULL);  
    pthread_cond_init(&buffer->full, NULL); 
    pthread_cond_init(&buffer->empty, NULL);
    buffer->file = file;
}//end initialize_buffer

/*
    get_product_type:
        A function used to determine which product type the product is.
        This also determines the buffer to insert the product into and which boolean needs to be check to stop distributor
    
    @param Product product: the current product due for insertion into a buffer
    @param Thread_agrs thread_agrs: the struct containing both buffers
    @param int* type: an int pointer used in distributor to signal when to stop
    
    @return type: an integer value used to choose which buffer to insert object into
*/
Buffer* get_product_type(Product product, Thread_args thread_agrs, int* type) {
    if (strcmp(product.productType, "product_A") == 0) {
        *type = 1;
        return thread_agrs.bufferA;
    }//end if (strcmp(product.productType, "product_A") == 0)
    else {
        *type = 2;
        return thread_agrs.bufferB;
    }//end else
}

/*
    dispatcher:
        The function that the dispatcher thread executes
        puts stuff in buffer until ending signal is reached
    
    @param void *args: a void pointer to Thread_args, a struct containing both buffers

    @return pthread_exit(NULL)
*/
void *distributor(void *args) {

    //typecasts the arguments from thread
    Thread_args *thread_args = (Thread_args*) args;

    //pointer to the correct buffer
    Buffer* buffer;
    
    //create the product
    Product product;

    //flags to know when each producer is finished
    bool prodA_is_active = true;
    bool prodB_is_active = true;

    //int and int* used to stop distributor
    int type = 0;
    int *ptrType = &type;

    //loops 
    while (prodA_is_active || prodB_is_active) {
        read(0, &product, sizeof(product)); //read from pipe

        //checks which buffer needs to be used
        buffer = get_product_type(product, *thread_args, ptrType);

        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count == buffer->arraySize) {
            pthread_cond_wait(&buffer->full, &buffer->mutex);
        }//while (buffer->count == buffer->arraySize)
            
        put_in_buffer(product, buffer);

        pthread_cond_signal(&buffer->empty);
        pthread_mutex_unlock(&buffer->mutex);

        if (product.count == -1) {
            if (type == 1) prodA_is_active = false;
            else prodB_is_active = false;
        }//end if (product.count == -1)
    }//end while (prod1_is_active || prod2_is_active)
    pthread_exit(NULL); 
}//end dispatcher

/*
    assignToStruct:
        A function that initializes the struct that contains all the informations needed in the output file

    @param OutputStruct output: the struct being initialized
    @param char *productType: a char pointer that holds the product type
    @param int product_count: the unique product count for each product
    @param int sequence_count: the total amount of products currently consumed

*/
void assignToStruct(OutputStruct *ouput, char *productType, int product_count, int sequence_count) {
    strcpy(ouput->productType, productType);
    ouput->tid = pthread_self();
    ouput->product_sequence = product_count;
    ouput->consumerCount = sequence_count;
}// void assignToStruct

/*
    consume:
        The function that the consumer threads execute
        gets product from specific buffer specified by parameter
    
    @param void *agrs: a void pointer to the buffer used by this specific thread

    @return pthread_exit(NULL)
*/
void *consume(void *args) {
    // Typecasts the arguments from thread
    Buffer *buffer = (Buffer *)args;

    OutputStruct output;

    // Create the product
    Product product;
    
    while (buffer->killswitch != 1) {
        pthread_mutex_lock(&buffer->mutex);
        while (buffer->count <= 0 && buffer->killswitch != 1) {
            pthread_cond_wait(&buffer->empty, &buffer->mutex);
        }//end while (buffer->count <= 0 && buffer->killswitch != 1)
        if (buffer->killswitch != 1) {
            product = get_product(buffer);

            assignToStruct(&output, product.productType, product.count, buffer->consumerCount);

            fprintf(buffer->file, "\n%15s | %20lu | %10d | %10d |", output.productType, (unsigned long) output.tid, output.product_sequence, output.consumerCount);
        }//end if (buffer->killswitch != 1)
        pthread_cond_signal(&buffer->full);
        pthread_mutex_unlock(&buffer->mutex); 

        if (product.count == -1) {
            pthread_mutex_lock(&buffer->mutex);
            buffer->killswitch = 1;
            pthread_cond_signal(&buffer->empty);
            pthread_mutex_unlock(&buffer->mutex);
            break;
        }//end if (product.count == -1)
    }//end while (buffer->killswitch != 1)
    pthread_exit(NULL);
}//end void *consume

/*
    Main:
    * creates the output file
    * writes the information columns into output file
    * creates and initializes the buffers
    * creates and initializes the Thread_args struct
    * creates the threads
    * waits for threads to finish
    * frees the buffers
    * 
    @param int argc: number of arguments pointed to by argv[]
    @param char *argv[]: contains a string that is the name of the output file.
    Also contains the necessary information for program to run
*/
int main(int argc, char *argv[]) {
    FILE *file;
    file = fopen(argv[1], "w");

    if (file == NULL) {
        perror("Error Opening file");
        exit(1);
    }//end if (file == NULL)

    fprintf(file, "%15s | %20s | %10s | %10s |", "Product ID", "Thread ID", "Prod Seq #", "Consumer Seq #");
    Buffer bufferA;
    Buffer bufferB;

    initialize_buffer(&bufferA, BUFFER_SIZE_A, file);
    initialize_buffer(&bufferB, BUFFER_SIZE_B, file);

    Thread_args thread_args;
    thread_args.bufferA = &bufferA;
    thread_args.bufferB = &bufferB;

    pthread_t distributor_thread, consumerA_thread1, consumerA_thread2, consumerB_thread1, consumerB_thread2; 
    pthread_create(&distributor_thread, NULL, distributor, (void *) &thread_args);
    pthread_create(&consumerA_thread1, NULL, consume, (void *) &bufferA);
    pthread_create(&consumerA_thread2, NULL, consume, (void *) &bufferA);
    pthread_create(&consumerB_thread1, NULL, consume, (void *) &bufferB);
    pthread_create(&consumerB_thread2, NULL, consume, (void *) &bufferB);
    
    pthread_join(distributor_thread, NULL);
    pthread_join(consumerA_thread1, NULL);
    pthread_join(consumerA_thread2, NULL);
    pthread_join(consumerB_thread1, NULL);
    pthread_join(consumerB_thread2, NULL);

    free(bufferA.array);
    free(bufferB.array);
    fclose(file);
    return 0;
}//end int main()