#include <stdio.h>
#include "../file_manager/manager.h"
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int tiempo = 1;

//Process
struct process
{
    int id;
    struct process* next;
    struct process* prev;
    char nombre[255];
    int n_fabrica;
    int state; //0:running, 1:ready, 2:waiting, 3:finished
    int cantidad_bursts;
    int* bursts;
    int veces_elegido_para_cpu;
    int veces_interrupciones;
    int tiempo_ingreso;
    int tiempo_salida;
    int response_time;
    int tiempo_burst;
};


int tiempo_total_burst(int* array, int cantidad){
    int tiempo_burst = 0;
    for (int i = 0; i < cantidad; i++)
    {
        if (i%2 == 0)
        {
            tiempo_burst= tiempo_burst + array[i];
        }  
    }
    return tiempo_burst;
}

struct process* process_init(int id, char* nombre, int n_fabrica, int state, int cantidad_bursts, int* burst, int tiempo_ingreso){
    struct process* new = malloc(sizeof(struct process));
    new->id = id;
    strcpy(new->nombre, nombre);
    new->n_fabrica = n_fabrica;
    new->state = state;
    new->cantidad_bursts = cantidad_bursts;
    new->bursts = burst;
    new->veces_elegido_para_cpu = 0;
    new->tiempo_ingreso = tiempo_ingreso;
    new->tiempo_salida = 0;
    new->response_time = -1;
    new->tiempo_burst = tiempo_total_burst(burst, cantidad_bursts);
    new->next=NULL;
    new->prev=NULL;
    return new;
}

//borra a todos los procesos que están después de el current en el queue y luego borra al current
//si no está en la queue solo borra al current
int destroy_process(struct process* current){
    //if (current->next)
    //{
    //   destroy_process(current->next);
    //}
    free(current);
    return 0;
    
}

struct queue
{
    int procesos_en_cola;
    int procesos_por_fabrica[4];
    struct process* first;
    struct process* last;
};

struct queue* queue_init(){
    struct queue* new = malloc(sizeof(struct queue));
    new->procesos_en_cola = 0;
    new->procesos_por_fabrica[0] = 0;
    new->procesos_por_fabrica[1] = 0;
    new->procesos_por_fabrica[2] = 0;
    new->procesos_por_fabrica[3] = 0;
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

    queue->procesos_en_cola = queue->procesos_en_cola + 1;
    queue->procesos_por_fabrica[new->n_fabrica - 1] = queue->procesos_por_fabrica[new->n_fabrica - 1] + 1;
    
    return 0;  
}

struct process* pop_process(struct queue* queue){
    struct process* poped= queue->first;
    queue->first = queue->first->next;
    queue->first->prev = NULL;
    poped->next = NULL;
    poped->prev = NULL;

    queue->procesos_en_cola = queue->procesos_en_cola - 1;
    queue->procesos_por_fabrica[poped->n_fabrica - 1] = queue->procesos_por_fabrica[poped->n_fabrica - 1] - 1;

    return poped;
}
//ordena destroy_process desde el primer proceso de la queue y luego destruye a la queue
int destroy_queue(struct queue* current){
    //if (current->first)
    //{
    //   destroy_process(current->first);
    //}
    free(current);
    return 0;
}

int avanzar_un_segundo(struct process* current){
    int iter = current-> cantidad_bursts;
    for (int i = 0; i < iter; i++)
    {
        if (current->bursts[i]>0)
        {
            current->bursts[i] = current->bursts[i] - 1;
            return current->bursts[i];
        } 
    }
    printf("NUNCA DEBERIA LLEGAR AQUI");
    return -1;   
}

int ultimo_burst(struct process* current){
    if (current->bursts[current->cantidad_bursts - 1] == 0)
    {
        return 1;
    }
    return 0;
}
int finalizar(struct process** lista_de_procesos, int cantidad){
    printf("ENTRE A FINALIZAR\n");
    for (int i = 0; i < cantidad; i++)
    {
        if (lista_de_procesos[i]->state != 3)
        {
            return 0;
        }
    }
    return 1;
    
}

void cambio_de_estado(struct process* proceso, int estado)
{
    if (proceso->state == 0)
    {
        printf("[t = %i] El proceso %s cambio a estado running\n", tiempo, proceso->nombre);
    }
    else if (proceso->state == 1)
    {
        printf("[t = %i] El proceso %s cambio a estado ready\n", tiempo, proceso->nombre);
    }
    else if (proceso->state == 2)
    {
        printf("[t = %i] El proceso %s cambio a estado waiting\n", tiempo, proceso->nombre);
    }
    else if (proceso->state == 3)
    {
        printf("[t = %i] El proceso %s cambio a estado finished\n", tiempo, proceso->nombre);
    }   
    proceso->state = estado;
}


void actualizar_waiting(struct queue* cola){
    struct process* current = cola->first;
    while (current)
    {
        if (current->state == 2)
        {
            int waiting_time = avanzar_un_segundo(current);
            printf("[t = %i] El proceso %s aun debe esperar %i segundos\n", tiempo, current->nombre, waiting_time);
            if (waiting_time == 0)
            {
                printf("cambiando estado a ready\n");
                cambio_de_estado(current, 1);
                //current->state = 1;
            }
        }
        current = current->next;
    }
}
//esta función calcula el quantum
int calcular_quantum(int n, int Q, int f)
//n es la cantidad de procesos de la fábrica i que se encuentran actualmente en la cola.
//f es la cantidad de fábricas con al menos un proceso dentro de la cola
//Q es un input del programa
{
  int denom = n * f;
  double result = (double) Q/denom; 
  result = (int) floor(result);
  return result;
}

int fabricas_en_cola(struct queue* cola){
    int contador = 0;
    for (int i = 0; i < 4; i++)
    {
        if (cola->procesos_por_fabrica[i] > 0)
        {
            contador++;
        }
        
    }
    return contador;
}

struct process* cola_a_cpu(struct queue* cola)
{
   struct process* current = cola->first;
    while (current)
    {
        if (current->state == 1)
        {
            printf("[t = %i] El proceso %s ingresará a la CPU\n", tiempo, current->nombre);
            return current;
        }
        current = current->next;
    }
    return NULL;
}


void remover_proceso_en_cola(struct queue* cola, struct process* current){
    if (current == cola->first)
    {
        if (cola->first==cola->last)
        {
            cola->first=NULL;
            cola->last=NULL;
        }else
        {
            current->next->prev = NULL;
            cola->first = current->next;
            current->next = NULL;
        }
    }else if (current == cola->last)
    {
        current->prev->next = NULL;
        cola->last = current->prev; 
        current->prev = NULL;
    }else
    {
        current->next->prev = current->prev;
        current->prev->next = current->next;
        current->next = NULL;
        current->prev = NULL;
    } 
    
}
void ingresar_procesos_a_cola(struct queue* cola, struct process** lista, int tiempo, int cantidad){
    struct process* desordenada[cantidad];
    int cantidad_de_procesos_a_ingresar = 0;
    for (int i = 0; i < cantidad; i++)
    {
        if (lista[i]->tiempo_ingreso == tiempo)
        {
            desordenada[i] = lista[i];
            cantidad_de_procesos_a_ingresar++;
        }else
        {
            desordenada[i] = NULL;
        }    
    }
    struct process* desordenada_2[cantidad_de_procesos_a_ingresar];
    int count = 0;
    for (int i = 0; i < cantidad; i++)
    {
        if (desordenada[i])
        {
            desordenada_2[count] = desordenada[i];
            count++;
        }
        
    }
    
    for (int i = 0; i < cantidad_de_procesos_a_ingresar; i++)
    {
        int smallest = i;
        for (int j = i + 1; j < cantidad_de_procesos_a_ingresar; j++)
        {
            if (desordenada_2[j]->n_fabrica < desordenada_2[smallest]->n_fabrica)
            {
                smallest = j;
            }
            else if (desordenada_2[j]->n_fabrica == desordenada_2[smallest]->n_fabrica)
            {
                if (strcmp(desordenada_2[j]->nombre, desordenada_2[smallest]->nombre) < 0)
                {
                    smallest = j;
                }
            }
            struct process* temp = desordenada_2[i];
            desordenada_2[i] = desordenada_2[smallest];
            desordenada_2[smallest] = temp;
        }
    }

    for (int i = 0; i < cantidad_de_procesos_a_ingresar; i++)
    {
        append_process(cola, desordenada_2[i]);
    }
}

int main(int argc, char **argv)
{
    /*struct process* p1 = process_init(1);
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
    */
    
    printf("Hello T2!\n");

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
    int cantidad_procesos = file->len;
    struct process* lista_de_procesos[cantidad_procesos];

    for (int i = 0; i < file->len; i++)
    {
        char **line = file->lines[i];

        printf(
                "Creando procesos (%i)%s de la fábrica %s con tiempo de inicio %s y %s bursts.\n",
                i, line[0], line[2], line[1], line[3]);

        int cantidad_bursts = atoi(line[3]) * 2 - 1;
        int burst_array[cantidad_bursts];
        printf("Con tiempos de cpu_burst-io_burst: ");
        for (int iter = 0; iter < cantidad_bursts; iter++)
        {
            burst_array[iter] = atoi(line[iter + 4]);
            printf("%i ", atoi(line[iter + 4]));
        }
        printf("\n");
        lista_de_procesos[i] = process_init(i, line[0], atoi(line[2]), 1, cantidad_bursts, burst_array, atoi(line[1]));
        printf("[t = %i] El proceso %s ha sido creado\n", tiempo, lista_de_procesos[i]->nombre);

    }
    struct queue* cola = queue_init();
    struct process* cpu = NULL; 
    int activo = 1;
    int quantum;
    while (activo)
    {
        if (cpu)
        {
            quantum--;
            int burst_restante = avanzar_un_segundo(cpu);

            if (burst_restante == 0)
            {
                if (ultimo_burst(cpu))
                {
                    cambio_de_estado(cpu, 3);
                    //cpu->state = 3;
                    cpu->tiempo_salida = tiempo;
                    if (finalizar(lista_de_procesos, cantidad_procesos))
                    {
                        activo = 0;
                    }
                }
                else
                {
                    cambio_de_estado(cpu, 2);
                    append_process(cola, cpu);
                }
                cpu = NULL;
            }

            else if (quantum == 0)
            {
                printf("HOLA se acabo el quantum\n");
                cambio_de_estado(cpu, 1);
                //cpu->state = 1;
                cpu->veces_interrupciones++;
                append_process(cola, cpu);
                cpu = NULL;
            }

        }
        
        actualizar_waiting(cola);
        ingresar_procesos_a_cola(cola, lista_de_procesos, tiempo, cantidad_procesos);

        if (!cpu && cola->procesos_en_cola)
        {
            struct process* temp = cola_a_cpu(cola);
            if (temp)
            {
                cpu = temp;
                cpu->veces_elegido_para_cpu++;
                quantum = calcular_quantum(cola->procesos_por_fabrica[cpu->n_fabrica - 1], Q, fabricas_en_cola(cola));
                if (cpu->response_time == -1)
                {
                    cpu->response_time = tiempo;
                }
                remover_proceso_en_cola(cola, cpu);
                printf("se pasa el proceso a running\n");
                cambio_de_estado(cpu, 0);
                //cpu->state = 0;
            }

        }
        if (!cpu)
        {
            printf("[t = %i] La CPU no está ejecutando procesos\n", tiempo);
        }
        
        tiempo++;
        //printf("%i\n", tiempo);
    }
    

    input_file_destroy(file);

    FILE *output_file = fopen(argv[2], "w");
    for (int index=0; index < cantidad_procesos; index++)
    {
        struct process* current = lista_de_procesos[index];
        int turnaround_time = current->tiempo_salida - current->tiempo_ingreso;
        int waiting_time = turnaround_time - current->tiempo_burst;
        fprintf(output_file, "%s,%i,%i,%i,%i,%i\n", current->nombre, current->veces_elegido_para_cpu, current->veces_interrupciones, turnaround_time, current->response_time, waiting_time);
        destroy_process(current);
    }
    fclose(output_file);
    destroy_queue(cola);

    return 0;
}
