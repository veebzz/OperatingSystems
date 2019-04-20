

#ifndef OPERATINGSYSTEMSP4_OSS_H
#define OPERATINGSYSTEMSP4_OSS_H

/*Majority of Queue files here are from GeeksForGeeks
https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/ */

typedef struct qStruct
{
    int front, rear, size;
    unsigned capacity;
    int* array;
}qStruct;

// function to create a queue of given capacity.  
// It initializes size of queue as 0 
qStruct* createQueue(unsigned capacity)
{
    int i;
    qStruct* queue = (qStruct*) malloc(sizeof(qStruct));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (int*) malloc(queue->capacity * sizeof(int)); // array of pids
    //start all spots on queue as -1 to indicate empty
    for(i = 0; i < capacity; i++){
        queue->array[i] = -1;
    }
    return queue;
}

// Queue is full when size becomes equal to the capacity  
int isFull(qStruct* queue)
{  return (queue->size == queue->capacity);  }

// Queue is empty when size is 0 
int isEmpty(qStruct* queue)
{  return (queue->size == 0); }

// Function to add an item to the queue.   
// It changes rear and size 
void enqueue(qStruct* queue, int item)
{
    if (isFull(queue)) {
        printf("QUEUE IS FULL\n");
        return;
    }
    queue->rear = (queue->rear + 1)%queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    printf("%d enqueued to queue\n", item);
}

// Function to remove an item from queue.  
// It changes front and size 
int dequeue(qStruct* queue)
{
    if (isEmpty(queue)) {
        perror("oss.h: dequeue error: Cannot dequeue an empty queue.\n");
        return -1;
    }
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)%queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue 
int front(qStruct* queue)
{
    if (isEmpty(queue)) {
        perror("oss.h: front error: Nothing is in front of queue\n");
        return -1;
    }
    return queue->array[queue->front];
}

// Function to get rear of queue
int rear(qStruct* queue)
{
    if (isEmpty(queue)) {
        perror("oss.h: rear error: Nothing is in back of queue\n");
        return -1;
    }
    return queue->array[queue->rear];
}

#endif //OPERATINGSYSTEMSP4_OSS_H
