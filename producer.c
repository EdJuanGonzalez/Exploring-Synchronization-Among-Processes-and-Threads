#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "product.h"

/*
    Main:
        Function that handles the production of products
        uses srand to have a random delay afer creating and sending product to pipe
        Uses the product struct found in product.h

    @param int argc: number of arguments pointed to by argv[]
    @param char *argv[]: arguments to the producer function, 
        argv[] consists of just a string containing the product type (either A or B)

*/
int main(int argc, char *argv[]) {
    srand(time(NULL));
    unsigned int num_of_iterations = (rand() % 51) + 151; //random number between 151 and 200 inclusive
    int count = 0; //number of the specific product
    char *product_type = argv[1];//the product type of these products
    /*
        loops:
        * creats the products
        * writes to pipe
        * waits a random time between .1 and .2 seconds
        * ensures that last item has a product count of -1 (used as a ending indicator)
    */
    for (int i = 0; i < num_of_iterations; i++) {
        Product product;
        if (i == num_of_iterations - 1) {
            product.count = -1; 
        }//end if (i == num_of_iterations - 1)
        else {
            product.count = count + 1;
        }//end else

        strcpy(product.productType, product_type); //assign product type
        
        write(1, &product, sizeof(product)); //write to pipe

        unsigned int delay = (rand() % (200000 - 10000 + 1)) + 10000; //random delay length
        usleep(delay); //delay
        count++; //increment count
    }//end for (int i = 0; i < num_of_iterations; i++)
    return 0;
}//end main()