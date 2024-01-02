#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define null 0

struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
    int memorysize;
};

int main(int argc, char * argv[])
{
    FILE * pFile;
    pFile = fopen("processes.txt", "w");
    int no;
    struct processData pData;
    printf("Please enter the number of processes you want to generate: ");
    scanf("%d", &no);
    srand(time(null));
    fprintf(pFile, "#id arrival runtime priority memory\n");
    pData.arrivaltime = 1;
    int i = 1;
    for ( ; i <= no ; i++)
    {
        pData.id = i;
        pData.arrivaltime += rand() % (11); //processes arrives in order
        pData.runningtime = rand() % (30);
        pData.priority = rand() % (11);
        pData.memorysize = rand() % (256);
        fprintf(pFile, "%d\t%d\t%d\t%d\t%d\n", pData.id, pData.arrivaltime, pData.runningtime, pData.priority, pData.memorysize);
    }
    fclose(pFile);
}

