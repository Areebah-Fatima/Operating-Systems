//By areebah fatima (AXF190025)

// Includes
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

// Global constants
#define MAX_CUSTOMERS 20
#define MAX_AGENTS 2
#define AGENT_Q_SIZE 4

// Function Pointers for thread functions (Customer, Information Desk, Announcer, Agent)
void * info_desk_cb(void * idp);
void * announcer_cb(void * idp);
void * customer_cb(void * idp);
void * agent_cb(void * idp);

// Semaphores
sem_t a_cust_enters_dmv;
sem_t cust_wait_for_srv_num[MAX_CUSTOMERS]; // Each specific customer
sem_t now_serving[MAX_CUSTOMERS]; // Each customer has a service number
sem_t cust_gets_lic[MAX_CUSTOMERS]; // Customer gets lic
sem_t a_customer_in_agent_line;

sem_t announcer_ready;
sem_t agent_ready;
sem_t cust_enters_wr;
sem_t agent_line;

sem_t mutex_info_desk_q;
sem_t mutex_agent_q;

// Infor Desk and Agent  Queue
struct Queue * info_desk_q;
struct Queue * agent_q;

// Counters
int num_assign_to_customer = 0;

int service_num = 0;
int now_serving_cust = 0;
int all_done = 0;

// Customer Structure (to keep track of customerId, assigned number, threadID)
typedef struct CustomerStruct {
  int customerId;
  int my_service_num;
  pthread_t threadId;
}
Customer;

// Agent Structure (to keep track of Id, threadID)
typedef struct AgentStruct {
  int agentId;
  pthread_t threadId;
}
Agent;

// function to start up all threads
void createSimulation(Customer customers[], Agent agent[]);

// To create Queues
struct Queue {
  int front, rear, size;
  unsigned capacity;
  Customer * array[100];
};

// Functions for queues
void enqueue(struct Queue * queue, Customer * cust);
Customer * dequeue(struct Queue * queue);
Customer * front(struct Queue * queue);

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue * createQueue(unsigned capacity);


// Call Back Function for agent threads
void * agent_cb(void * idp) {

  Agent * agent;
  agent = (Agent * ) idp;

  Customer * cust = NULL;

  // While all customers have not been served ie simulation is not over yet
  while (!all_done) {

    // Agent will wait to begin processing until a customer has signalled they have entered the line
    sem_wait( & a_customer_in_agent_line);
  
    // Semaphore wait to protect the agent queue (wont dequeue until all other alterations of the queue have completed) 
    sem_wait( & mutex_agent_q); // Wait if some other thread is altering the agent_q
    cust = dequeue(agent_q); // The next customer is the next waiting customer in the agent line
    sem_post( & mutex_agent_q); // signal we are done with the agent_q
    
    if (cust == NULL) {
      //printf("Agent %d  has no customer to serve\n", agent -> agentId);
      continue;
    }

    // Print agents actions
    printf("Agent %d is serving customer  %d\n", agent -> agentId, cust -> customerId);
    printf("Customer %d is being served by agent %d\n", cust -> customerId, agent -> agentId); 

    printf("Agent %d asks customer %d to take photo and eye exam\n", agent -> agentId, cust -> customerId);
    printf("Customer %d completes photo and eye exam for agent %d\n", cust -> customerId, agent -> agentId);
    printf("Agent %d gives license to customer %d\n", agent -> agentId, cust -> customerId);
    
    // To signal a license has been given to a specific customer
    sem_post( & cust_gets_lic[cust -> customerId]);
    // Signal an empty spot in the agent line
    sem_post( & agent_line);
  }
  return;
}


// Call Back Function for information desk threads
void * info_desk_cb(void * idp) {

  Customer * cust = NULL;

  //While there are customer to serve 
  while (service_num < MAX_CUSTOMERS) {
    
    // Info desk will not begin processing until a customer is waiting to be assigned a number (wait on customer to enter dmv)
    sem_wait( & a_cust_enters_dmv);

    // Protect the info_desk_q
    sem_wait( & mutex_info_desk_q);   //  Wait if some other thread is altering the info_desk_q
    cust = dequeue(info_desk_q);     // The next customer is the next waiting customer in the queue
    sem_post( & mutex_info_desk_q); // signal we are done with the info_desk_q

    if (cust == NULL) {
      printf("Info Desk has no customer to process\n");
      continue;
    }

    // Assign number
    cust -> my_service_num = service_num;
    
    // signal to a specific customer. To let the customer know their number has been given  (they are free to move to the waiting room)     
    sem_post( & cust_wait_for_srv_num[cust -> customerId]);
    
    // increment service number
    service_num++;

  }
  return;
}


// Call Back Function for announcer threads
void * announcer_cb(void * idp) {
  Customer * customer;

  //While there are customer to serve 
  while (now_serving_cust < MAX_CUSTOMERS) {
    
    // the announcer will run only when there are customers with numbers assigned to them (wait for customers to (signal) enter the waiting room)
    sem_wait( & cust_enters_wr);

    //Announcer will wait on atleast one of the 4 seats to open up in agnet line before announcing another customer 
    sem_wait( & agent_line);

    // Print Announcer actions
    printf("Announcer calls number %d\n", now_serving_cust);

    // signal to a specific customer. To let the customer know they have been allowed to enter the agent line. To let the customer know their number has been called
    sem_post( & now_serving[now_serving_cust]);
    
    //  increment service number
    now_serving_cust++;
  }
  return;
}


// Call Back Function for customer threads
void * customer_cb(void * idp) {

  int custnr;

  // Customer has entered DMV
  Customer * customer;
  customer = (Customer * ) idp;
  
  // Print the customers that has entered the dmv
  printf("Customer %d created, enters DMV\n", customer -> customerId);

  //to protect queue
  sem_wait( & mutex_info_desk_q); // wait if some other process is altering the queue
  enqueue(info_desk_q, customer);
  sem_post( & mutex_info_desk_q); // signal when done altering queue

  // Post a customer is ready (let info desk know a customer is waiting)
  sem_post( & a_cust_enters_dmv);

  // Now wait for the Semaphore for the thread (wait for a number to be given before moving to the waiting room)
  sem_wait( & cust_wait_for_srv_num[customer -> customerId]);

  // Move to wait room with assigned number
  printf("Customer %d gets number %d, enters waiting room\n", customer -> customerId, customer -> my_service_num);

  // to let announcer know there are customers to serve
  sem_post( & cust_enters_wr);

  // Wait for the number to be called 
  sem_wait( & now_serving[customer -> my_service_num]);


  // Enqueue customer to the agent line
  sem_wait( & mutex_agent_q);   // wait if another thread is altering the agent q
  enqueue(agent_q, customer);
  sem_post( & mutex_agent_q);   // signal to other waiting threads we have finished our aalterations

  // Print customer actions
  printf("Customer %d moves to agent line\n", customer -> customerId);

  // To let agent know there is a customer to serve
  sem_post( & a_customer_in_agent_line);

  // wait for a license  to be assigned before departing from the dmv
  sem_wait( & cust_gets_lic[customer -> customerId]);

  // Print customers final actions
  printf("Customer %d gets license and departs\n", customer -> customerId);
  printf("Customer %d was joined\n", customer -> customerId);
  return;
}

int main() {

  // Set up sturcture to keep track of threads with mutiple instances
  Customer customerArray[MAX_CUSTOMERS];
  Agent agentArray[MAX_AGENTS];

  // Queues for the info desk line and agent line
  info_desk_q = createQueue(100);
  agent_q = createQueue(100);

  // Call helper function to 1)	Creates and join (etc) all threads
  createSimulation(customerArray, agentArray);
  return 0;
}

// Will set up all threads in simulation
void createSimulation(Customer customers[], Agent agents[]) {

  int i = 0;
  int rc = 0;

  // Threads
  pthread_t customer_thread;
  pthread_t info_desk_thread;
  pthread_t announcer_thread;
  pthread_t agent_thread;

  // Set Id for all customers (20)
  for (i = 0; i < MAX_CUSTOMERS; ++i) {
    Customer customer;
    customer.customerId = i;
    customers[i] = customer;
  }

  // Set ids for agent 0 and 1
  for (i = 0; i < MAX_AGENTS; ++i) {
    Agent agent;
    agent.agentId = i;
    agents[i] = agent;
  }

  int thread_count = 0;


  // Initialize semaphores 
  sem_init( & a_cust_enters_dmv, 0, 0);
  sem_init( & a_customer_in_agent_line, 0, 0);
  for (i = 0; i < MAX_CUSTOMERS; ++i) {
    sem_init( & cust_wait_for_srv_num[i], 0, 0);
  }

  for (i = 0; i < MAX_CUSTOMERS; ++i) {
    sem_init( & now_serving[i], 0, 0);
  }

  for (i = 0; i < MAX_CUSTOMERS; ++i) {
    sem_init( & cust_gets_lic[i], 0, 0);
  }

  sem_init( & mutex_info_desk_q, 0, 1);
  sem_init( & mutex_agent_q, 0, 1);

  sem_init( & cust_enters_wr, 0, 0);
  sem_init( & agent_line, 0, 4);

  //create info desk thread
  rc = pthread_create( & info_desk_thread, NULL, (void * ) info_desk_cb, NULL);
  if (rc) {
    printf("Failed to create announcer thread.");

  } else {

    printf("Information desk created\n");
  }

  //create announcer thread
  rc = pthread_create( & announcer_thread, NULL, (void * ) announcer_cb, NULL);
  if (rc) {
    printf("Failed to create announcer thread.");

  } else {

    printf("Announcer created\n");
  }

  //create agent threads 
  for (i = 0; i < MAX_AGENTS; ++i) {
    rc = pthread_create( & agents[i].threadId, NULL, (void * ) agent_cb, & agents[i]);
    if (rc) {
      printf("Failed to create agent thread.");

    } else {
      printf("Agent %d created\n", i);
    }
  }

  // create customer thread (20) 

  for (i = 0; i < MAX_CUSTOMERS; ++i) {

    //rc = pthread_create(&customer_thread, NULL, (void *)customer, NULL);
    if (pthread_create( & customers[i].threadId, NULL, (void * ) customer_cb, & customers[i])) {
      printf("Failed to create customer thread.");
    }
  }

  // wait for threads to join
  for (i = 0; i < MAX_CUSTOMERS; ++i) {
    if (pthread_join(customers[i].threadId, NULL)) {
      printf("Failed to join customer thread");
    } else {
      //printf("Customer %d was joined\n", customers[i].customerId);
    }
  }

  // If all customers are done
  all_done = 1;

  // post semaphores to unblock agents
  for (i = 0; i < MAX_AGENTS; ++i) {
    sem_post(&a_customer_in_agent_line);
    sem_post(&a_customer_in_agent_line);
  }
  //printf("All customers done!!! \n");

  for (i = 0; i < MAX_AGENTS; ++i) {
    sem_post( & a_customer_in_agent_line);
    if (pthread_join(agents[i].threadId, NULL)) {
      printf("Failed to join agent %d  thread", i);
    } else {
      //printf("Agent %d thread was joined\n", i);
    }
  }

  //printf("Info Thread Join \n");
  if (pthread_join(info_desk_thread, NULL)) {
    printf("Failed to join Info Desk thread");
  } else {
    //printf("Info Thread was Joined \n");
  }

  //printf("Annocuner Thread Join \n");
  if (pthread_join(announcer_thread, NULL)) {
    printf("Failed to join Anncoucer thread\n");
  } else {
    //printf("Announcer Thread was Joined \n");
  }

  
  // Print Done
  printf("Done\n");

  return;
}

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue * createQueue(unsigned capacity) {
  struct Queue * queue = (struct Queue * ) malloc(
    sizeof(struct Queue));
  queue -> capacity = capacity;
  queue -> front = queue -> size = 0;

  // This is important, see the enqueue
  queue -> rear = capacity - 1;
  return queue;
}

// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue * queue) {
  return (queue -> size == queue -> capacity);
}

// Queue is empty when size is 0
int isEmpty(struct Queue * queue) {
  return (queue -> size == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue * queue, Customer * cust) {
  
  // Dont add to a full queue
  if (isFull(queue))
    return;
 
 queue -> rear = (queue -> rear + 1) %
    queue -> capacity;
  queue -> array[queue -> rear] = cust;
  queue -> size = queue -> size + 1;
  //printf("QUEUE: 0%x (ID = %d) enqueued to queue\n", cust, cust->customerId);
}

// Function to remove an item from queue.
// It changes front and size
Customer * dequeue(struct Queue * queue) {
  
  // dont remove item from an empty list
  if (isEmpty(queue))
    return NULL;
  Customer * cust = queue -> array[queue -> front];
  
  // move front and update capacity of the queue 
  queue -> front = (queue -> front + 1) %
    queue -> capacity;
  
  // update size
  queue -> size = queue -> size - 1;
  return cust;
}

// Function to get front of queue
Customer * front(struct Queue * queue) {

 // No front if empty queue
  if (isEmpty(queue)) {
    printf("Front: Queue is Empty \n");
    return NULL;
  }
  Customer * cust = queue -> array[queue -> front];
  return cust;
}

// Function to get rear of queue
Customer * rear(struct Queue * queue) {
 
 // No rear if empty queue
 if (isEmpty(queue))
    return NULL;
  return queue -> array[queue -> rear];
}
