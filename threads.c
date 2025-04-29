#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
    Main: 
        Forks 2 producer processes and 1 consumer process
        pipes the two producers to the consumer
        waits for all three
    
    @param int argc: number of arguments pointed to by argv[]
    @param char *argv[]: contains the name of the output file or nothing
    also contains necessary information for program to run
*/
int main(int argc, char *argv[]) {
    //check if user inputted a name for the output file
    char *nameOfFile;
    //if they provided more than one argument then stop
    if (argc >= 3) {
        puts("Invalid input");
        exit(1);
    }
    //if argc > 1, user entered an argument so store that in nameOfFile
    if (argc > 1) {
        nameOfFile = argv[1];
    }
    //if they didn't the default is output.txt
    else {
        nameOfFile = "output.txt";
    }
    //piping 
    int fd[2]; 
    pipe(fd);

    //fork for first producer
    int pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(1);
    }//end if (pid1 < 0)

    else if (pid1 == 0) {
        // Child process 1 (producer for product_1)
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);

        char *array[] = {"producer", "product_A", NULL};
        execv("./producer", array);
        perror("execv");
        exit(1);
    }//end else if (pid1 == 0)

    //fork for second producer
    int pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(1);
    }//end if (pid2 < 0)

    else if (pid2 == 0) {
        // Child process 2 (producer for product_2)
        close(fd[0]);
        dup2(fd[1], 1);
        close(fd[1]);

        char *array[] = {"producer", "product_B", NULL};
        execv("./producer", array);
        perror("execv");
        exit(1);
    }//end else if (pid2 == 0)

    //fork for consumer
    int pid3 = fork();
    if (pid3 < 0) {
        perror("fork");
        exit(1);
    }//end if (pid3 < 0)

    else if (pid3 == 0) {
        // Child process 3 (consumer)
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);

        char *array[] = {"consumer", nameOfFile, NULL};
        execv("./consumer", array);
        perror("execv");
        exit(1);
    }//end else if (pid3 == 0)

    // Parent process
    close(fd[0]);
    close(fd[1]);

    // Wait for all child processes to terminate
    wait(NULL);
    wait(NULL);
    wait(NULL);

    return 0;
}//end main
