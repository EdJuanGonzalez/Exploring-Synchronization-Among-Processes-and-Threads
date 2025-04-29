# Exploring-Synchronization-Among-Processes-and-Threads
You are given three (3) processes in one program that work together to solve a producer  consumer problem. Two Processes are “producers”, each producing its own type of product in a  continuous loop. That is, one product type produced by one producer and a different product  type by the other producer.

# Project-2-S24
## Project 2: Exploring Synchronization Among Processes and Threads

## Deliverable 1
-threads.c
    --Forking 2 producer processes and 1 consumer process
    --pipe both producers to consumer
-producer.c
    --producer file has only one function (main) that does everything specified by the requirments
    --iterates and produces products that get sent to consumer through pipe
    --sleeps for a random amount of time between specified values
-consumer.c
    --creation and implementation of the consumer file
        ---put_in_buffer: puts products in respective buffer
        ---get_product: gets product from specified buffer
        ---initialize_buffer: initializes a buffer
        ---distributor: function that distributor thread executes
            ----loops through and uses put_in_buffer to put products in buffers and exits once -1 (exit signal) is read
        ---main: creates and initializes everything before creating thread
        
## Deliverable 2
-updates to consumer.c
    ---get_product_type function: returns a pointer to the correct buffer
        ----also assigns a type to an int pointer used in distributor
        ----previously in the distributor function, this function determines which buffer to use
-Syncronization in distributor and consume functions
    --Uses mutex, thread conditionals, and signals to ensure syncronization
-product.h
    --header file containing the product struct definition

## Deliverable 3
--updates to consumer.c
    ---output file creation and implementation
        ----Instead of printing to screen, results are in a file
    ---update to buffer struct
        ----FILE *file: a pointer to the output file
        ----consumerMutex: a mutex for the consumer count
    ---OutputStruct struct: the struct that holds all information required for output file
    ---assignToStruct: a functon that assigns all necessary values to OuputStruct
-allows for user to enter name of output file if they desire

## Disclaimers
Make file will generate 3 executables that are used for program to function

## Known Issues
If the executables already exist and were compiled natively on ones machine, they will not work on server 
and make file cannot override them if nothing has changed
If these executables already exist type make clean

## Compilation
If there are already executables or the file permissions are denying anything
Step 1: Type "make clean" if statement above is true
Step 2: Type make
Step 3: Type ./threads [name of output file]
NOTE: the name of output file argument is not necessary

## Testing

### Deliverable 1
Print statements that show if the thread terminated successfully
Print statements that show the product type and count
    -For product_A there should be 20
    -For product_B there should be 15
The sizes should match the amounts printed 
The counts should both be 151 for deliverable A

### Deliverable 2
Print statements in consume function that show what is happening
So long as A_i+1 doesn't come before A_i (same for b) and program terminates, the program is successful
The total count should be 1 greater than the last number printed to the screen from the product because the -1 product is not printed

### Deliverable 3
Check the output.txt file to see if all the information was successfully added to output file.
