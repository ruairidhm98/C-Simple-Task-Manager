#include <time.h>
#include <stdio.h>

/* Tester function */
void test() { printf("Hello world\n"); }

int main(int argc, char **argv) {

    double t1, t2;
    int i, numProcesses;

    /* Check if the correct nuber of command line arguments have been passed */
    if (argc != 2) {
        fprintf(stderr, "Error: wrong number of command line arguments entered\n");
        fprintf(stderr, "Expected: 2\n");
        fprintf(stderr, "Got: %d\n", argc);
        fprintf(stderr, "Exiting with 1 (invalid command line arguments)\n");
        return 1;
    }
    numProcesses = -1;
    sscanf(argv[1], "%d", &numProcesses);
    if (numProcesses < -1){
        fprintf(stderr, "Error: argv[1] was invalid\n");
        fprintf(stderr, "Exiting with 1 (invalid command line arguments)\n");
        return 1;
    }
    t1 = clock();
    for (i = 0; i < numProcesses; i++) test();
    t2 = clock();

    fprintf(stderr, "Time taken %.6fs\n", (t2-t1)/CLOCKS_PER_SEC);

    return 0;
}