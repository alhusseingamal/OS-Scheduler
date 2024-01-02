/*
=================================Buddy Memory allocation algorithm=================================

Problem Defintion:
We have a memory size of 2^m bytes
A process requests k byts of memory
you allocate the smallest number that is both a power of two and larger than k, let it be M, so M = 2 ^ ceil(log2(k))

We can think of a binary tree that keeps splitting on allocating and re-merging on deallocating

It is required to work with a best fit approach; that is allocate the smallest possible block of memory.
In case of multiple blocks with the same size, you can take any of them.

Note: Although best fit minimizes the wastage space, it consumes a lot of processor time for searching the best-fit (smallest) block size.
Like other placement algorithms, it doesn't avoid internal fragmentation.


Pseudocode:
If: 2 ^ ceil(log2(k - 1)) < k < 2 ^ ceil(log2(k)), k is allocated the whole block
Else: Recursively divide the block equally and test the condition at each time, when it satisfies, allocate the block and get out the loop.

We assume the root node exists from the start, but the code is easily extensible to account to the case where it isn't

How to allocate the smallest block?

We traverse the whole tree, searching for the smallest block and saving a pointer pointing to it.
After the traversal is done, the allocation is performed at the smallest-size block and it is then marked as used

Notice that this smallest block size could still be divided into smaller blocks depending on the required k bytes.
This further splitting is done accordingly during the allocation and not during our search for the best block

Note: Either the left and right are both allocated (not null) or are both null. For this approach to work, no other case should happen\

Each node can be in one of three states: 
1) it is used ==> cannot be considered for allocation
2) it is split ==> one of the children is used, the other isn't, and hence this unused child can be considered for allocation
3) it is neither used, nor split ==> can be considered for allocation, note that this is NOT a NULL node
*/


# define MAX_LEVEL 10
#define TOTAL_MEMORY_SIZE pow(2,10) // 1024 bytes
#define MAX_PROCESS_SIZE pow(2,8) // 256 bytes
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

typedef struct Data
{
    int process_id; // the id of the process it holds, if any.
    int memorysize; // the memorysize -in bytes- that the process takes.
    int memorystart; // the starting byte of the process memory
    int memoryend; // the ending byte of the process memory
    int process_node_level; // the level of the process-storing node within the memory tree
    // process_node_level may be removed later
} Data;

typedef struct BTnode
{
    // node information
    int level; // the node level in the tree; We start from MAX_LEVEL at root down to level 0
    /*
    A node can be in one of three states
            isSplit isUsed
        1.     0       1   ------ Both children are null
        2.     1       0   ------ Both children are not null
        3.     0       0   ------ Both children are null
    A node cannot be both used and split; this is a forbidden state.
    */ 
    int isUsed; // a node is either used (1) to store a process or not used
    int isSplit; // a node is either split or not
    struct Data data; // the data stored in the node
	struct BTnode *left; // pointer to the node's left child
	struct BTnode *right; // pointer to the node's right child
} BTnode;

void assignNode(struct BTnode *node, struct Data *new_process_data)
{
    // copy required data from the new process to the node
    node->data.process_id = new_process_data->process_id;
    node->data.memorysize = new_process_data->memorysize;
    
    // copy required data from the node to the new process
    new_process_data->memorystart = node->data.memorystart;
    new_process_data->memoryend = node->data.memoryend;
    new_process_data->process_node_level = node->level;

    // update node information
    node->left = NULL;
    node->right = NULL;
    node->isSplit = 0;
    node->isUsed = 1; // the node is now storing data
}


// This function takes a node and splits it into two allocatable children
void splitNode(struct BTnode *node)
{
    node->isUsed = 0; // this is probably useless, but just to make sure
    node->isSplit = 1;
    
    
    // create the left child
    struct BTnode *newnode = (struct BTnode *)malloc(sizeof(struct BTnode));
    newnode->level = (node->level) - 1;
    newnode->left = newnode->right = NULL;
    newnode->isUsed = 0;
    newnode->isSplit = 0;
    newnode->data.process_id = -1;
    newnode->data.memorystart = node->data.memorystart;
    newnode->data.memoryend = (node->data.memorystart + node->data.memoryend) / 2;

    node->left = newnode;


    // create the right child
    newnode = (struct BTnode *)malloc(sizeof(struct BTnode));
    newnode->level = (node->level) - 1;
    newnode->left = newnode->right = NULL;
    newnode->isUsed = 0;
    newnode->isSplit = 0;
    newnode->data.process_id = -1;
    newnode->data.memorystart = (node->data.memorystart + node->data.memoryend) / 2;
    newnode->data.memoryend = pow(2, ceil(log2(node->data.memoryend)));

    node->right = newnode;

    return;
}

/*
=================================
            Tree Methods
=================================
*/

int allocate(struct BTnode *node, struct Data *new_process_data) // traverse
{
    /*
    When might an allocation fail?
    We can think of two cases:
    1) node is null
    2) the required memory block size is larger than any available block
    */
    if(node == NULL || new_process_data->memorysize > pow(2, node->level))
    {
        return -1;
    }

    // we check if the level lower than ours can accomodate our required memory
    // if so, we split the current node, else we allocate at the current node

    if(new_process_data->memorysize <= pow(2, node->level - 1)) // check if the required memory can be fit into the lower level
    {   
        splitNode(node);
        return allocate(node->left, new_process_data); 
        /*
        We choose to always allocate to the leftmost child, if possible
        It is important to see that it doesn't matter which child we choose to allocate to
        Both have the same size and are at the same level, so the choice is ours
        Probabilistically, our choice to assign to the left child first makes increases the
        probability of a right child being found by the search function as the best block in future requests
        */
    }
    
    assignNode(node, new_process_data);

    // printf("Allocation successful at level = %d with id = %d\n", node->level, node->data.process_id);
    return 0;
}


void search(struct BTnode *node, int memsize, struct BTnode **best){
    
    // the case of an invalid (NULL) node
    if(node == NULL){
        return;
    }

    // the case of a node that is already allocated to some process

    if(node->isUsed == 1){ // if the node is used, it cannot be considered for allocation
        return;
    }

    // the case where the required memory is larger than the available space
    if(memsize > pow(2, node->level)){
        return;
    }

    // Note: From now on, any other node under consideration is neither used, nor smaller than the required memory

    // the case of a node that is nor used, nor split yet ==> and so we can consider it as a whole for allocation
    if((node->left) == NULL && (node->right) == NULL)
    {   
        if(*best == NULL || (node->level < (*best)->level && memsize <= pow(2, node->level)))
        {
            *best = node; // note that best will always be a node that doesn't have children
        }
        // return; // we don't even have to put this as its children are NULL and therefore searching them yields nothing
    }

    // the case of node that is not used, but is split, so we can consider its children for allocation
    // If it is not split, the following two function calls have no effect as left and right are null

    search(node->left, memsize, best); // search left
    search(node->right, memsize, best); // search right
    
    return;
}

void clearNode(struct BTnode *node){
    node->isUsed = 0;
    node->isSplit = 0;
    node->data.process_id = -1;
    return;
}

// returns 1 upon successfully deallocating and 0 otherwise
int deallocate(struct BTnode* node, struct Data *new_process_data)
{
    if(node == NULL)
    {
        return 0;
    }
    
    if(node->isUsed == 1) // check if the node is allocated to a process
    { 
        if(node->data.process_id == new_process_data->process_id) // if so, check that it matches the process we want to remove
        { 
            new_process_data->memorystart = node->data.memorystart;
            new_process_data->memoryend = node->data.memoryend;
            new_process_data->memorysize = node->data.memorysize;
            new_process_data->process_node_level = node->level;
            // printf("node id = %d cleared\n", node->data.process_id);
            clearNode(node);
            // JUST TO MAKE SURE
            node->left = NULL;
            node->right = NULL;
            return 1;
        }
    }
    else
    {   
        int flag = 0; // a counter of the number of children deallocated
        if(deallocate(node->left, new_process_data) || deallocate(node->right, new_process_data))
        {
            ++flag;
        }
        /*
        The next few lines of code subtly constitute the re-merging funtioonality
        If both the left and right children blocks are deallocated, the parent block no longer has
        children and now constiutes a single allocatable memory block on its own
        */
        if(node->left != NULL && node->right != NULL &&
        node->left->isSplit == 0 && node->right->isSplit == 0 && node->left->isUsed == 0 && node->right->isUsed == 0)
        {
            node->data.memorystart = node->left->data.memorystart;
            node->data.memoryend = node->right->data.memoryend;
            free(node->left);
            free(node->right);
            node->left = NULL;
            node->right = NULL;
            node->isSplit = 0;
        }
        if(flag)
        {
            return 1; // to indicate successful deallocation
        }
    }
    return 0;
}

struct Tree
{
    struct BTnode *head;
};

void initializeMemory(struct Tree *t1){

    t1->head = (struct BTnode *) malloc(sizeof(struct BTnode));
    t1->head->level = MAX_LEVEL;
    t1->head->isUsed = 0;
    t1->head->isSplit = 0;
    // 
    // we may lump those in a separate function
    t1->head->data.memorysize = TOTAL_MEMORY_SIZE;
    t1->head->data.process_id = -1;
    t1->head->data.memorystart = 0;
    t1->head->data.memoryend = TOTAL_MEMORY_SIZE;
    t1->head->data.process_node_level = MAX_LEVEL;
    // 
    t1->head->left = NULL;
    t1->head->right = NULL;
    return;
}

void clearMemory(struct Tree *t1){
    free(t1->head);
    return;
}
