struct crmsfile
{
    int process_id;
    int validation;
    int name;
    int size;
    int virtual_address;
};

typedef struct crmsfile CrmsFile;
CrmsFile* cr_open(int process_id, char* file_name, char mode);
