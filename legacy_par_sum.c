/*
 * sumsq.c
 *
 * CS 446.646 Project 1 (Pthreads)
 *
 * Compile with --std=c99
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define MAX_THREADS 4



static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// aggregate variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;

struct node{
    char process[1];
    int value;
    struct node* next;
};


volatile struct node* head = NULL;
volatile struct node* latest = NULL;
const struct node* current = NULL;
const char wait[1] = {'w'};

int insert_processes(char *action, long int *num){

  struct node* new_node = (struct node*) malloc(sizeof(struct node));

  new_node -> process[0] = *action;
  new_node -> value = *num;

  //if head hasn't been initialized/pointed to anything yet, proceed to do so.
  if(head == NULL){
    new_node -> next = head;
    head = new_node;
    latest = head;
    return 1;
  }
  latest -> next = new_node;
  latest = latest -> next;
  return 1;
}


// function prototypes
// void calculate_square(long number);
void *calculate_square(void* value);
void *master_thread(void* value);


void *master_thread(void* value){

  //current value is the files


  char **arguments = (char**)value;

  long int number_of_threads = strtol(arguments[2], NULL, 10);


  //Take the current file and read from it
  FILE* fin = fopen(arguments[1], "r");
  char action;
  long num;

  
  //check to see if the worker threads parameter is less than 0
  if(number_of_threads < 0){
    printf("Thread creation parameter is invalid! Please execute with a value > 0.\n");
    exit(0);
  }
  else{
    printf("Parameters Okay\n");
  }


  pthread_t *worker_threads = malloc(sizeof(pthread_t)*(number_of_threads)); 
  // worker_threads = realloc(worker_threads, sizeof(pthread_t)*(number_of_threads));


  volatile char current_process;
  int current_value = 0;
  int counter = 0;
  int wait_time = 0;


  fscanf(fin, "%c %ld\n", &action, &num);

  fflush(stdout);


  do{
    // fflush(stdin);
    insert_processes(&action, &num);
    printf("Current here: %c, %d\n", latest -> process[0], latest -> value);

    current = latest;
    current_process = strcmp(current -> process, wait);

    fflush(stdout);
    if(current_process == 0){
      printf("Waiting...\n");
      wait_time = current -> value;
      sleep(wait_time);
      printf("Finished waiting.\n");
      fflush(stdout);
    }
    else{
      current_value = current -> value;
      printf("creating child thread: %d with value %d\n", counter+1, current_value);
      fflush(stdout);
      pthread_create(&worker_threads[counter], NULL, calculate_square, (void *)current_value);
      counter++;
    }
    
  }while(fscanf(fin, "%c %ld\n", &action, &num) == 2);

  fclose(fin);

  printf("waiting for all threads to finish...\n");
  fflush(stdout);
  //wait for all threads to finish then return
  for(int i = 0; i < number_of_threads; i++){
    pthread_join(worker_threads[i], NULL);
  }


}


//   FILE* fin = fopen(file_arg, "r");

//}

void *calculate_square(void* value)
{
  
  pthread_mutex_lock(&mutex);

  long *number = (long*)value;

  printf("value casted, starting calucations\n");
  printf("the value is: %ld\n", number);
  fflush(stdout);
  // calculate the square
  long the_square = *number * *number;

  printf("%ld\n", the_square);
  fflush(stdout);


  // let's add this to our (global) sum
  sum += the_square;

  // now we also tabulate some (meaningless) statistics
  if (*number % 2 == 1) {
    // how many of our numbers were odd?
    odd++;
  }

  // what was the smallest one we had to deal with?
  if (*number < min) {
    min = *number;
  }

  // and what was the biggest one?
  if (*number > max) {
    max = *number;
  }
  printf("Thread done.\n");
  fflush(stdout);
  pthread_mutex_unlock(&mutex);
}

//master thread will be initialized in main
//initialize global mutex and execute the master thread by send the functions the arg values
int main(int argc, char* argv[])
{

  if (argc != 3 || argv[2] < 0) {
    printf("Usage: sumsq <infile>\n");
    exit(EXIT_FAILURE);
  }


  pthread_t master;

  pthread_create(&master, NULL, master_thread, (void *)argv);

  pthread_join(master, NULL);

  printf("%ld %ld %ld %ld\n", sum, odd, min, max);
  
  return (EXIT_SUCCESS);
}

