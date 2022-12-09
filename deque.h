//
// hexeditor.c library file
// deque.c
//
// Provides logic for double-ended queue (deque) data structure.
//
// Demonstration:
//
//   +-----------+-----------+-----------+-----------+-----------+
//   |           |           |           |           |           |
//   |  Segment  |  Segment  |  Segment  |  Segment  |  Segment  |
//   |     0     |     1     |     2     |     3     |     4     |
//   |           |           |           |           |           |
//   +-----------+-----------+-----------+-----------+-----------+
//
// Values can be inserted at the front of the deque (first non-filled segment) or inserted at the back of the
// deque (first segment) which pushes all of the other segments up 1 segment, if the deque has space.
// 

// Avoid redefinition errors during compilation
#ifndef FILE_DEQUE_SEEN
#define FILE_DEQUE_SEEN

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    int front; // Distance from beginnning of allocated memory.
    int arrayLength; // Length of each element of the deque.
    int length; // Length of the deque.
    char **d; // Underlying data.
} deque;

// Checks deque object is valid. Only available in scope of deque.c
// deque: pointer to the deque.
//
// Throws if deque is null.
static void checkDequeIsValid(deque *deque)
{
    // Throw if deque is null.
    if (deque == NULL)
    {
        fprintf(stderr, "Deque is null.\n");
        exit(1);
    }
}

// Checks deque arguments supplied are valid. Only available in scope of deque.c
// deque: pointer to the deque.
// array: the array.
//
// Throws if deque is null, array is null or array and deque element length are not equal.
static void checkDequeArguments(deque *deque, char *array)
{
    checkDequeIsValid(deque);

    // Throw if array is null.
    if (array == NULL)
    {
        fprintf(stderr, "Array is null.\n");
        exit(1);
    }
}

// Generate a deque.
// length: the length of the deque.
// arrayLength: the length of each element of the deque.
//
// Returns: the generated deque.
deque *buildDeque(int length, int arrayLength)
{   
    // Allocate memory for deque.
    deque *obj = (deque *)malloc(length * arrayLength + sizeof(int) * 4);

    // Initialise deque variables.
    obj->front = -1;
    obj->length = length;
    obj->arrayLength = arrayLength;
    obj->d = (char **)malloc(length * arrayLength);

    return obj;
}

// Delete the backmost array from the queue.
// deque: pointer to the deque that will have the element removed.
//
// Throws if deque is empty or deque is null
void deleteDequeBack(deque *deque)
{
    checkDequeIsValid(deque);
    
    // Check if deque is empty
    if (deque->front == -1)
    {
        fprintf(stderr, "Deque empty\n");
    }

    // Iterate through each segment of the deque
    for (int i = 1; i < deque->front + 1; i++) // i refers to the segment in the deque
    {
        // Push all segments down one to fill the null segment 0.
        free(deque->d[i - 1]);
        deque->d[i - 1] = (char *)malloc(deque->arrayLength);
        memcpy(deque->d[i - 1], deque->d[i], deque->arrayLength);
    }
    
    deque->front--;
}

// Enqueue an array to the front. For pushing overflowing values out of queue, use pushDequeFront()
// deque: pointer to the deque that the array will be inserted into.
// array: the array to insert into the deque.
//
// Throws if deque is full, deque is null, array is null or array and deque element length are not equal.
void insertDequeFront(deque *deque, char *array)
{
    checkDequeArguments(deque, array);

    // Check deque has space.
    if (deque->front >= deque->length - 1)
    {
        fprintf(stderr, "Deque full\n");
        exit(1);
    }

    deque->front++;
    // Insert the value onto the front of the array.
    deque->d[deque->front] = (char *)malloc(deque->arrayLength);
    memcpy(deque->d[deque->front], array, deque->arrayLength);
}

// Enqueue an array to the back. For pushing overflowing values out of queue, use pushDequeBack()
// deque: pointer to the deque that the array will be inserted into.
// array: the array to insert into the deque.
//
// Throws if deque is full, deque is null, array is null or array and deque element length are not equal.
void insertDequeBack(deque *deque, char *array)
{
    checkDequeArguments(deque, array);

    // Check deque has space.
    if (deque->front >= deque->length - 1)
    {
        fprintf(stderr, "Deque full\n");
        exit(1);
    }

    // Push all existing segments forward one.
    for(int i = deque->front; i > -1; i--)
    {
        deque->d[i + 1] = deque->d[i];
    }

    // Replace segment 0 with value.
    deque->d[0] = array;

    deque->front++;

}

// Delete the frontmost array from the queue.
// deque: pointer to the deque that will have the element removed.
//
// Throws if deque is empty or deque is null
void deleteDequeFront(deque *deque)
{
    checkDequeIsValid(deque);

    // Check deque has space.
    if (deque->front == -1)
    {
        fprintf(stderr, "Deque empty\n");
    }

    // Remove frontmost segment.
    free(deque->d[deque->front]);
    deque->front--;
}

// Enqueue an array to the front. If the queue is full, push out the backmost element.
// deque: pointer to the deque that the array will be inserted into.
// array: the array to push into the deque.
//
// Throws if deque is null, array is null or array and deque element length are not equal.
void pushDequeFront(deque *deque, char *array)
{
    checkDequeArguments(deque, array);

    // If the deque doesn't have enough space, remove back segment and insert, otherwise insert normally.
    if (deque->front >= deque->length - 1)
    {
        deleteDequeBack(deque);
        insertDequeFront(deque, array);
    }
    else
    {
        insertDequeFront(deque, array);
    }
}

// Enqueue an array to the back. If the queue is full, push out the frontmost element.
// deque: pointer to the deque that the array will be inserted into.
// array: the array to push into the deque.
//
// Throws if deque is null, array is null or array and deque element length are not equal.
void pushDequeBack(deque *deque, char *array)
{
    checkDequeArguments(deque, array);

    // If the deque doesn't have enough space, remove front segment and insert, otherwise insert normally.
    if (deque->front >= deque->length - 1)
    {
        deleteDequeFront(deque);
        insertDequeBack(deque, array);
    }
    else
    {
        insertDequeBack(deque, array);
    }
}

// Reads the backmost elemnt of the deque.
// deque: pointer to the deque to read from.
// nullInjector: if set, automatically injects the null terminator character to the output.
//
// Throws if deque is null
char *readDequeBack(deque *deque, int nullInjector)
{
    checkDequeIsValid(deque);

    // Check deque has entries.
    if (deque->front == -1)
    {
        return NULL;
    }

    char *val = deque->d[0]; // The value of first segment.
    char *output; // The output.
    if (nullInjector)
    {
        // Allocate memory for output, and copy the val to output, inserting the terminator at end.
        output = (char *)malloc(deque->arrayLength + 1);
        memcpy(output, val, deque->arrayLength + 1);
        output[deque->arrayLength] = '\0';
    }
    else
    {
        // Allocate memory for output, and copy the val to output
        output = (char *)malloc(deque->arrayLength);
        memcpy(output, val, deque->arrayLength);
    }
    return output;
}

// Reads the frontmost element of the deque.
// deque: pointer to the deque to read from.
// nullInjector: if set, automatically injects the null terminator character to the output.
//
// Throws if deque is null
char *readDequeFront(deque *deque, int nullInjector)
{
    checkDequeIsValid(deque);

    // Check deque has entries.
    if (deque->front == -1)
    {
        return NULL;
    }

    char *val = deque->d[deque->front]; // The value of last segment.
    char *output; // The output.
    if (nullInjector)
    {
        // Allocate memory for output, and copy the val to output, inserting the terminator at end.
        output = (char *)malloc(deque->arrayLength + 1);
        memcpy(output, val, deque->arrayLength + 1);
        output[deque->arrayLength] = '\0';
    }
    else
    {
        // Allocate memory for output, and copy the val to output
        output = (char *)malloc(deque->arrayLength);
        memcpy(output, val, deque->arrayLength);
    }
    return output;
}

// Reads a byte from a specified location in the deque. 
// index: the value of the index at which the array of the value is stored.
// elementIndex: the value of the index at which the specific character exists at.
//
// Returns: the value of the byte at the specified location.
char readDequeByte(deque *deque, int index, int elementIndex)
{
    return deque->d[index][elementIndex];
}

// Replaces a byte at a specified location in the deque. 
// index: the value of the index at which the array of the value is stored.
// elementIndex: the value of the index at which the specific character exists at.
// byte: the new value of the specified location.
char writeDequeByte(deque *deque, int index, int elementIndex, char byte)
{
    deque->d[index][elementIndex] = byte;
}

// Free all lines of the deque
void freeDequeLines(deque *deque)
{
    for (int i = 0; i < deque->front + 1; i++)
    {
        free(deque->d[i]);
    }
    deque->front = -1;
}

#endif
