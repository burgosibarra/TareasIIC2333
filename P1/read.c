#include <stdlib.h>
#include <stdio.h>

struct crmsfile
{
    int process_id;
    int validation;
    int name;
    int size;
    int virtual_address;
};

typedef struct crmsfile CrmsFile;

char* path = "memformat.bin";
int PCB_SIZE = 4096; // tamaño del espacio de tablas PCB 
int PCB_POINTER_SIZE = 256; // tamaño de una tabla PCB 
int POINTERS_AMOUNT = 16; // Cantidad de tablas PCB
int PCB_SUBPOINTER_SIZE = 21; // Cantidad de entradas de archivos
int PAGE_TABLE_SIZE = 32; // tamaño de tabla de paginas
int PROCESS_NAME = 12; // tamaño del nombre de un proceso
int SUBPOINTERS_AMOUNT = 10; // tamaño de las entradas de un archivo 
int PROCESS_FILE_NAME = 12; // tamaño del nombre de un proceso
int PROCESS_FILE_SIZE = 4;
int PROCESS_FILE_VIRTUAL_MEMORY = 4;


int cr_read(CrmsFile* file_desc, void* buffer, int n_bytes)
{
    FILE* memory;
    memory = fopen(path, "rb");
    // en file_desc debría estar la dirección virtual del archivo 
    fclose(memory);

    char status;
    char process_id;
    int VPN;
    int PFN;
    int offset;
    int physicalAddress;
    int page_table;
    int bytes_read;
    for (int pointer = 0; pointer < POINTERS_AMOUNT; pointer++)
    {
        fread(&status, 1, 1, memory); // se lee el status del proceso
        fread(&process_id, 1, 1, memory); // se lee id de el proceso
        if (process_id == file_desc->process_id) // si el proceso es el mismo del archivo se busca la tabla de páginas
        {

            fseek(memory,210, SEEK_CUR); // pasamos las entradas de archivos
            page_table = ftell(memory); // guardamos la posición de la tabla de páginas del proceso
            // separamos la VPN de el offset en la dirección virtual
            int virtual_dir = file_desc->virtual_address;
            int SEG_MASK = 0b00001111100000000000000000000000;
            int OFFSET_MASK = 0b00000000011111111111111111111111;
            int SEG_SHIFT = 23;
            VPN = (virtual_dir & SEG_MASK) >> SEG_SHIFT;
            offset = virtual_dir & OFFSET_MASK;

            int page;
            fseek(memory ,VPN, SEEK_CUR); // nos movemos hasta la entrada número segment
            fread(&page, 1, 1, memory); // leemos la entrada 1bit de validez y 7 de PFN
            // se paramos el bit de validez del PFN
            int valid_mask = 128;
            int pfn_mask = 127;
            int valid_shift = 7;
            int valid = (page & valid_mask) >> valid_shift;
            int PFN = virtual_dir & OFFSET_MASK;
            int physicalAddress = PFN + offset; // obtenemos la dirección fisica como la dirección de la pagina + el offset
            // continuamos moviendonos hasta llegar al final de las tablas
        }
        else
        {
            fseek(memory, PCB_POINTER_SIZE - 2, SEEK_CUR);
            
        }
    }
    fseek(memory, 128, SEEK_CUR); // nos saltamos el frame bit map
    fseek(memory, PFN*8388608, SEEK_CUR); // llegamos a la página designada por el pfn*8MB
    if (n_bytes+offset<=8388608) // si solo tenemos que leer una página
    {
        /* leer los n_bytes */
        fread(buffer, n_bytes, 1, memory); 
        
    }else
    {
        /*hay que leer hasta donde se pueda y seguir a la siguiente página*/
        bytes_read = 0;
        while (bytes_read<n_bytes) // mientras no terminemos de leer 
        {
            /*leemos lo que podamos*/
            fseek(memory, page_table, SEEK_SET); //volvemos a la tabla de páginas
            /*buscamos la siguiente página*/
        }
    }
    
    
    
    
}