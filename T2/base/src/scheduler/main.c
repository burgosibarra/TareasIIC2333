#include <stdio.h>
#include "../file_manager/manager.h"
#include <unistd.h>
#include <math.h>

struct process
{
    pid_t pid;
    char* nombre[255];
    int fabrica;
    /* Estados:
        0: Running
        1: Ready
        2: Waiting
        3: Finished
    */
    int estado;
    int cantidad_cpu_burst;
    int* A;
};
typedef struct process Process;

/* Inicia y retorna un proceso */
Process processs_init(int pid, char* nombre, int fabrica, int estado, int CPU_burst)
{
    int A[2 * CPU_burst - 1];
    Process proceso;
    proceso.pid = pid;
    strcpy(proceso.nombre, nombre);
    proceso.fabrica = fabrica;
    proceso.estado = estado;
    proceso.cantidad_cpu_burst = CPU_burst;
    proceso.A = A;
    return proceso;
}

struct queue
{
    Process** procesos;
    /* Arreglo de 4 enteros:
        si el índice i está en 0 es que la fábrica i no tiene procesos en la cola
        si el índice i está en 1 es que la fábrica i    tiene procesos en la cola
    */
    int cantidad_de_procesos;
    int* presencia_fabrica;
};
typedef struct queue Queue;

/* Inicia y retorna una cola*/
Queue queue_init(int numero_de_procesos)
{
    Process** queue = calloc(numero_de_procesos, sizeof(Process*));
    int* presencia_fabrica = {0, 0, 0, 0};
    Queue new_queue;
    new_queue.procesos = queue;
    new_queue.cantidad_de_procesos = numero_de_procesos;
    new_queue.presencia_fabrica = presencia_fabrica;
    return new_queue;
}

/* Agrega un proceso en un espacio libre de la cola*/
Queue agregar_proceso(Queue cola, Process* proceso)
{
    for (int index = 0; index < cola.cantidad_de_procesos; index++)
    {
        if (cola.procesos[index] == NULL)
        {
            cola.procesos[index] = proceso;
            cola.presencia_fabrica[proceso->fabrica] = 1;
            break;
        }
    }
}

//esta función calcula el quantum
double Quantum(int n, int Q, int f)
//n es la cantidad de procesos de la fábrica i que se encuentran actualmente enla cola.
//f es la cantidad de fábricas con al menos un proceso dentro de la cola
//Q es un input del programa
{
  int denom = n * f;
  double result = (double) Q/denom; 
  floor(result);
  return result;
}

int main(int argc, char **argv)
{
    printf("Hello T2!\n");

    int Q;
    if (argc == 3)
    {
        Q = 100;
    }
    else if (argc == 4)
    {
        Q = argv[3];   
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
        /**/
    }


    input_file_destroy(file);
}
