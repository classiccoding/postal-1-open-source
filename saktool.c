#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// linux only for now
#include <sys/stat.h>

#define VER_MAJ     0
#define VER_MIN     1


#define SAK_COOKIE 0x204b4153		// Looks like "SAK " in the file
#define SAK_CURRENT_VERSION 1		// Current version of SAK file format

typedef struct  {
    char*       name;
    uint32_t     offset;
    uint32_t     size;
} sak_record;

typedef struct {
    uint32_t         sig;
    uint32_t         ver;
    uint32_t         size;
    uint32_t         cap;
    sak_record       *records;
} sak_file;

void read_cstring(FILE*f, char* buff) {
    int i = 0;
    do {
        fread(buff+i, 1, 1, f);
    } while (buff[i++]);
}

// return 1 if name is a match for mask, return 0 if not (mask can include ? and *)
int match(const char* name, const char* mask)
{
    int i = 0, j = 0;
    if(!mask) return 1;     // empty mask is always a match
    if(!mask[0]) return 1;
    
    char c, c1, cn;
    c1 = mask[j++]; cn = mask[j++];
    do {
        c = name[i++];
        if (c!=c1) {
            if (c1=='?') {
                c1 = cn;
                cn = mask[j++];
            } else if (c1!='*') {
                return 0;
            } else {
                if (c == cn) {
                    c1 = mask[j++];
                    cn = mask[j++];
                }
            }
        } else {
            c1 = cn;
            cn = mask[j++];
        }
    } while (c);

    return 1;
}

// very rudimentary folder check
int check_folder(const char* folder)
{
    if(!folder) 
        return 0;
    if(!folder[0])
        return 0;
    if(strchr(folder,'*')!=NULL)
        return 0;
    if(strchr(folder,'?')!=NULL)
        return 0;
    
    return 1;
}

int create_folder(const char* folder)
{
    // recursively create folders
    struct stat st = {0};
    char* buff = strdup(folder);
    char* name = strrchr(buff, '/');
    if(name) {
        *name='\0';
        name++;
        create_folder(buff);
    } else 
        name = buff;

    if(stat(folder, &st) == -1) {
        mkdir(folder, 0777);    //TODO, put some better permissions
        if(stat(folder, &st) == -1) {
            free(buff);
            return 0;
        }
    } 
    free(buff);
    return 1;
}

void add_offset(uint32_t *offsets, uint32_t off, int size)
{
    int i = 0;
    do {
        if(!offsets[i]) {
            offsets[i] = off;
            return;
        }
        if(offsets[i]>=off) {
            for (int j=size-1; j>i; j--) offsets[j]=offsets[j-1];
            offsets[i] = off;
            return;
        }
        i++;
    } while(i<size);
}

uint32_t size_from_offset(uint32_t *offsets, uint32_t off, int size)
{
    int i = 0;
    do {
        if(offsets[i]==off) {
            return offsets[i+1] - offsets[i];
        }
        i++;
    } while (i<size);
    return 0;
}

sak_file* read_sak(const char* sakname)
{
    sak_file * sak = (sak_file*)malloc(sizeof(sak_file));
    memset(sak, 0, sizeof(sak_file));
    sak->sig = SAK_COOKIE;
    sak->ver = SAK_CURRENT_VERSION;
    // open sak file
    FILE *sakfile = fopen(sakname, "rb");
    if(sakfile==NULL) {
        // nothing to read...
        printf("New file\n");
        return sak;
    }
    // read & check signature
    uint32_t ul;
    fread(&ul, sizeof(ul), 1, sakfile);
    if(ul != SAK_COOKIE) {
        printf("Error: not a SAK file\n");
        free(sak);
        return NULL;
    }
    // read & check version
    fread(&ul, sizeof(ul), 1, sakfile);
    if(ul != SAK_CURRENT_VERSION) {
        printf("Error: SAK version (%u) unsuported\n", ul);
        free(sak);
        return NULL;
    }
    uint16_t us;
    fread(&us, sizeof(us), 1, sakfile);
    sak->cap = us;
    sak->records = (sak_record*)malloc(us*sizeof(sak_record));
    uint32_t n = 0, offset = 0;
    char buff[256];
    while (n<us) {
        read_cstring(sakfile, buff);
        sak->records[n].name = strdup(buff);
        fread(&ul, sizeof(ul), 1, sakfile);
        sak->records[n].offset = ul;
        n++;
    }
    fseek(sakfile, 0, SEEK_END);
    ul = ftell(sakfile);
    // create an ordered maps of offset
    uint32_t *offsets = (uint32_t*)malloc((n+1)*sizeof(uint32_t));
    memset(offsets, 0, sizeof(uint32_t)*(n+1));
    for (int j=0; j<n; j++) add_offset(offsets, sak->records[j].offset, n+1);
    add_offset(offsets, ul, n+1);
    for (int j=0; j<n; j++) sak->records[j].size = size_from_offset(offsets, sak->records[j].offset, n+1);

    sak->size = n;

    fclose(sakfile);

    return sak;
}

void list_sak_content(sak_file *sak, const char* mask)
{
    if(!sak) return;
    printf("SAK version %u\n\n", sak->ver);
    for (int i=0; i<sak->size; i++)
    {
        if(match(sak->records[i].name, mask))
            printf(" %s\tsize=%u\n", sak->records[i].name, sak->records[i].size);
    }
}

void extract_sak(const char* sakfile, sak_file *sak, const char* mask, const char* folder)
{
    if(!sak) return;
    char* name[4096];
    void* buff = NULL;
    uint32_t size = 0;
    FILE *f = fopen(sakfile, "rb");
    for (int i=0; i<sak->size; i++) {
        if(match(sak->records[i].name, mask)) {
            strcpy((char*)name, folder);
            strcat((char*)name, "/");
            strcat((char*)name, sak->records[i].name);
            printf("Extracting %s...", name);
            // read data
            if(sak->records[i].size > size) {
                free(buff);
                size = sak->records[i].size;
                buff = malloc(size);
            }
            fseek(f, sak->records[i].offset, SEEK_SET);
            fread(buff, sak->records[i].size, 1, f);
            // try to create file
            FILE *o = fopen((char*)name, "wb");
            if (o==NULL) {
                // faillure, try to locate any folder and create them
                char* tmp = strdup((char*)name);
                char* sep = strrchr(tmp, '/');
                if(sep) {
                    *sep='\0';
                    create_folder(tmp);
                    o = fopen((char*)name, "wb");
                }
            }
            if (o==NULL) {
                printf("\tFailled\n");
            } else {
                fwrite(buff, sak->records[i].size, 1, o);
                fclose(o);
                printf("\tok\n");
            }
        }
    }
    fclose(f);
    free(buff);
}

void free_sak(sak_file *sak)
{
    if(!sak) return;
    for (int i=0; i<sak->size; i++)
    {
        free(sak->records[i].name);
    }
    free(sak->records);
    sak->size = sak->cap = 0;
    sak->records = NULL;
}

int main(int argc, char** argv)
{
    int cmd = -1;
    char* sakfile = NULL;
    char* folder = NULL;
    char* mask = NULL;

    printf("saktools v%d.%d\n", VER_MAJ, VER_MIN);

    if (argc<3 || (argc>1 && argv[1][0]=='h'))  {
        printf("\n\nUsage:\n\nsaktools CMD sakfile (folder) [mask]\n");
        printf("\twhere CMD is\n");
        printf("\t l : list content (no folder argument)\n");
        printf("\t e : extract content (folder argument mandatory)\n");
        return -1;
    }
    if(strcmp(argv[1],"l")==0) {
        cmd = 0; 
    }
    if(strcmp(argv[1],"e")==0) {
        cmd = 1; 
    }
    if(strcmp(argv[1],"x")==0) {
        cmd = 1; 
    }
    if(cmd<0) {
        printf("Unknown command '%s'\n", argv[1]);
        return -2;
    }
    sakfile = argv[2];
    if (argc>3) {
        if(cmd == 0)
            mask = argv[3];
        else
            folder = argv[3];
    }
    if (argc>4)
        mask = argv[4];
    
    switch(cmd) {
        case 0 : {
            sak_file* sak = read_sak(sakfile);

            list_sak_content(sak, mask);
            for (int j=4; j<argc; j++)
                list_sak_content(sak, argv[j]);

            free_sak(sak);
            free(sak);
        }
        case 1 : {
            if(!check_folder(folder)) {
                printf("ERROR: Bad folder name\n");
                return -5;
            }
            sak_file* sak = read_sak(sakfile);
            if(create_folder(folder)) {
                extract_sak(sakfile, sak, mask, folder);
                for (int j=5; j<argc; j++)
                    extract_sak(sakfile, sak, argv[j], folder);
            } else {
                printf("Error creating folder \"%s\"\n", folder);
            }

            free_sak(sak);
            free(sak);
        }
        break;
    }

    return 0;
}