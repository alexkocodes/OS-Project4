/* Group Parners: Alex Ko (fyk211) and Ritin Malhotra (rm5486)*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    // Declare variables for args (as specified by the assignment)
    char *operation;
    char *ad_file;
    char *input_file;

    // Get the comamnd-line args
    operation = argv[1];
    ad_file = argv[2];
    input_file = argv[3];

    printf("Operation: %s\n", operation);
    printf("Ad File: %s\n", ad_file);
    printf("Input File: %s\n", input_file);

    exit(0);
}
