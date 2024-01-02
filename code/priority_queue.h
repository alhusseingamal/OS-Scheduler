#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

/*******************************************************************************
 *                                   Node                                      *
 * *****************************************************************************/
typedef struct node
{
    struct PCB *pcb;
    struct node *next;
    int priority;
} Node;


/*******************************************************************************
 *                                   Priority Queue Functions                  *
 * *****************************************************************************/
Node *createNode(struct PCB *pcb, int priority)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->pcb = pcb;
    node->priority = priority;
    node->next = NULL;
    return node;
}

struct PCB *peek(Node **head)
{
    return (*head)->pcb;
}
struct PCB *pop(Node **head)
{
    Node *temp = *head;
    (*head) = (*head)->next;
    struct PCB *pcb = temp->pcb;
    free(temp);
    return pcb;
}

void push(Node **head, struct PCB *pcb, int priorityno)
{
    Node* start = (*head);
    if (start == NULL)
    {
        (*head) = createNode(pcb, priorityno);
        return;
    }
    Node *temp = createNode(pcb, priorityno);
    if ((*head)->priority > priorityno)
    {
        temp->next = *head;
        (*head) = temp;
    }
    else
    {
        while (start->next != NULL && start->next->priority <= priorityno) /*find the right position to insert*/
        {
            start = start->next;
        }
        temp->next = start->next;
        start->next = temp;
    }
}


int isEmptyPQ(Node **head)
{
    return (*head) == NULL;
}

int getPQSize(Node **head)
{
    Node *temp = *head;
    int size = 0;
    while (temp != NULL)
    {
        size++;
        temp = temp->next;
    }
    return size;
}

#endif
