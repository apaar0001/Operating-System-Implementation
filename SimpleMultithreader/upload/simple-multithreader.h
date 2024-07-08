#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <pthread.h>
#include <time.h>

using namespace std;

int user_main(int argc, char **argv);
int counter = 1;

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

// Initialising global variables

double total_execution_time = 0;

// Function to define structures after the program starts
    // Defining structs for thread arguments
  typedef struct {
      int low_1;
      int high_1;
      int low_2;
      int high_2;
      function<void(int, int)> lambda;
  } thread_args_double_for;

  typedef struct {
      int low;
      int high;
      function<void(int)> lambda;
  } thread_args_for;


/*our parallel_for implementation*/
/*low_1 and high_1 are the range of the outer loop*/
/*low_2 and high_2 are the range of the inner loop*/
/*lambda is the function to be executed in parallel*/
/*numThreads is the number of threads to be used*/


void parallel_for(int low, int high, std::function<void(int)>&& lambda, int numThreads);
void parallel_for(int low_1, int high_1, int low_2, int high_2, function<void(int, int)>&& lambda, int numThreads);

// Defining the functions for the parallel_for implementation
void for_loop(int low, int high, function<void(int)>&& lambda) {
    for (int i = low; i < high; i++) {
        lambda(i);
    }
}

// Defining the functions for the parallel_for implementation
void double_for_loop(int low_1, int high_1, int low_2, int high_2, function<void(int, int)>&& lambda) {
    for (int i = low_1; i < high_1; i++) {
        for (int j = low_2; j < high_2; j++) {
            lambda(i, j);
        }
    }
}


// Defining the thread functions
void* thread_func_for(void* ptr) {
    thread_args_for* args = (thread_args_for*)(ptr);
    for_loop(args->low, args->high, std::move(args->lambda));
    pthread_exit(NULL);
}

// Defining the thread functions
void* thread_func_double_for(void* ptr) {
    thread_args_double_for* args = (thread_args_double_for*)(ptr);
    double_for_loop(args->low_1, args->high_1, args->low_2, args->high_2, std::move(args->lambda));
    pthread_exit(NULL);
}

int makeThread(int default_value,int i,int chunk){
  return default_value + i * chunk;
}

int makeThreadO(int default_value,int i,int chunk,int overflow){
  return default_value + i*chunk + overflow;
}
// Implementing the parallel_for functions
void parallel_for(int low, int high, std::function<void(int)>&& lambda, int numThreads) {
   
    pthread_t tid[numThreads];   // Making an Array to store thread IDs
    int default_value = low;
    thread_args_for args[numThreads];  // Making an Array to store thread arguments

    // Calculating the chunk size and overflow for iterations among threads
    int chunk = (high - low) / numThreads;
    int overflow = (high - low) % numThreads;

    // Recording start time for performance measurement
    clock_t start_time = clock();

    // Creating threads and assign loop ranges to each thread
    for (int i = 0; i < numThreads; i++) {
        args[i].low = makeThread(default_value,i,chunk);
        if (i == numThreads - 1) {
          args[i].high = makeThreadO(default_value,i+1,chunk,overflow);
        } else {
            args[i].high = makeThread(default_value,i+1,chunk);
        }
        args[i].lambda = lambda;
        if(pthread_create(&tid[i], NULL, thread_func_for, (void*)(&args[i])) != 0){
          perror("pthread_create");
        };
    }

    // Waiting for all threads to finish
    for (int i = 0; i < numThreads; i++) {
        if(pthread_join(tid[i], NULL) != 0){
          perror("pthread_join");
        };
    }

    // Recording end time and calculating the total execution time
    clock_t end_time = clock();
    double total_time = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    // Print total execution time for the parallel_for call
    printf("\nTotal Execution Time for parallel_for call %d: %f milliseconds\n", counter, total_time);
    counter++;
    total_execution_time += total_time; 
}

// Implementation of parallel_for for nested loops
void parallel_for(int low_1, int high_1, int low_2, int high_2, function<void(int, int)>&& lambda, int numThreads) {
    
    pthread_t tid[numThreads]; // Making the Array to store thread IDs
    int default_value = low_1;
    
    thread_args_double_for args[numThreads]; // Making the Array to store thread arguments

    // Calculating the chunk size and overflow for loop iterations among threads
    int chunk = (high_1 - low_1) / numThreads;
    int overflow = (high_1 - low_1) % numThreads;

    // Recording start time for performance measurement
    clock_t start_time = clock();

    // Creating threads and assign loop ranges to each thread
    for (int i = 0; i < numThreads; i++) {
        args[i].low_1 = makeThread(default_value,i,chunk);
        if (i == numThreads - 1) {
          args[i].high_1 = makeThreadO(default_value,i+1,chunk,overflow);
        } else {
            args[i].high_1 = makeThread(default_value,i+1,chunk);
        }
        args[i].low_2 = low_2;
        args[i].high_2 = high_2;
        args[i].lambda = lambda;

        // Creating thread
        if (pthread_create(&tid[i], NULL, thread_func_double_for, (void*)&args[i]) != 0) {
          perror("pthread_create");
        }
    }

    // Waiting for all threads to finish
    for (int i = 0; i < numThreads; i++) {
        if(pthread_join(tid[i], NULL) != 0){
          perror("pthread_join");
        };
    }

    // Recording end time and calculate total execution time
    clock_t end_time = clock();
    double total_time = ((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC;

    // Print total execution time for the parallel_for call
    printf("\nTotal Execution Time for parallel_for call %d: %f milliseconds\n", counter, total_time);
    counter++;
    total_execution_time += total_time;
}

// Defining the main function
int main(int argc, char **argv) {
  // defineStructures();
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);

  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"\nTotal Execution Time for all parallel_for calls: "<<total_execution_time<<" milliseconds\n";
    std::cout<<"\n====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main


