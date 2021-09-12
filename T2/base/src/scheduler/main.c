#include <stdio.h>
#include "../file_manager/manager.h"
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
//Process
struct process
{
    int id;
    struct process* next;
    struct process* prev;
    /*pid_t pid;
    char** name;
    int n_fábrica;
    int state; //0:running, 1:ready, 2:waiting, 3:finished
    int** bursts;*/
};
struct process* process_init(int id){
    struct process* new = malloc(sizeof(struct process));
    new->id=id;
    new->next=NULL;
    new->prev=NULL;
    return new;
}

//borra a todos los procesos que están después de el current en el queue y luego borra al current
//si no está en la queue solo borra al current
int destroy_process(struct process* current){
    if (current->next)
    {
        destroy_process(current->next);
    }
    free(current);
    return 0;
    
}

struct queue
{
    int processes_in_queue;
    struct process* first;
    struct process* last;
};

struct queue* queue_init(){
    struct queue* new = malloc(sizeof(struct queue));
    new->processes_in_queue = 0;
    new->first = NULL;
    new->last = NULL;
    return new;
}
int append_process(struct queue* queue, struct process* new){
    if (queue->first)
    {
        queue->last->next = new;
        new->prev = queue->last;
        queue->last = new;
    }else
    {
        queue->first = new;
        queue->last = new;
    }
    return 0;
    
    
}
//ordena destroy_process desde el primer proceso de la queue y luego destruye a la queue
int destroy_queue(struct queue* current){
    if (current->first)
    {
        destroy_process(current->first);
    }
    free(current);
    return 0;
}



//esta función calcula el quantum
double Quantum(int n, int Q, int f)
//n es la cantidad de procesos de la fábrica i que se encuentran actualmente enla cola.
//f es la cantidad de fábricas con al menos un proceso dentro de la cola
//Q es un input del programa
{
  int denom = n * f;
  double result = (double) Q/denom; 
  result = floor(result);
  return result;
}

int main(int argc, char **argv)
{
    struct process* p1 = process_init(1);
    struct process* p2 = process_init(2);
    struct process* p3 = process_init(3);
    struct process* p4 = process_init(4);

    struct queue* cola = queue_init();
    append_process(cola, p1);
    append_process(cola, p2);
    append_process(cola, p3);
    append_process(cola, p4);
    printf("first:%i, second:%i, third:%i, fourth:%i\n", cola->first->id, cola->first->next->id, cola->first->next->next->id, cola->last->id);

    destroy_queue(cola);
    /*printf("Hello T2!\n");

    int Q;
    if (argc == 3)
    {
        Q = 100;
    }
    else if (argc == 4)
    {
        Q = atoi(argv[3]);   
    }
    else
    {
        exit(1);
    }

    InputFile *file = read_file(argv[1]);

    printf("Reading file of length %i:\n", file->len);
    for (int i = 0; i < file->len; i++)
    {
        char **line = file->lines[i];
        printf(
                "\tProcess %s from factory %s has init time of %s and %s bursts.\n",
                line[0], line[2], line[1], line[3]);
    }


    input_file_destroy(file);*/
}
