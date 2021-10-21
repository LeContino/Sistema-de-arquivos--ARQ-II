#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define pattern 256 //padrao representa o 2^8 que aparece diversas vezes no enunciado do problema -> 0 a 255 =256
#define clusterSize 32768
#define indiceStart 8 // 1 byte tam do indice, 2 bytes tam do cluster, 2 bytes inicio do indice, 2 bytes inicio root  -> prox byte � posicao do indice
#define rootPos 263   // 7 bytes posteriores +256 bytes de indice

//Nome    Valor       Significado
//SEEK_SET       0        Início do arquivo
//SEEK_CUR       1        Ponto corrente no arquivo
//SEEK_END       2        Fim do arquivo

// int fseek (FILE *fp,long numbytes,int origem);

typedef struct file_folder_item_pattern
{
    bool ler_bit : 1;      //indica se ha algo para ler
    bool eh_pasta_bit : 1; // indica se e pasta ou arquivo
    short ponteiro;
    char nome[16];

} ITEM;

typedef struct metadados
{

    short tam_indice;           // char e implicitamente transformado em um numero  --> 16 bits
    unsigned short tam_cluster; // 1kB= 1024B
    short indiceStartice;       //inicio do indice --> 16 bits
    short inicio_root;          //posicao do primeiro cluster

} METADADOS;

typedef struct
{
    ITEM *array;
    size_t used;
    size_t size;
} Array;

void initArray(Array *a)
{
    a->array = malloc(sizeof(ITEM));
    a->used = 0;
    a->size = sizeof(ITEM);
}

void insertArray(Array *a, ITEM element)
{
    // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
    // Therefore a->used can go up to a->size
    if (a->used == a->size)
    {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(ITEM));
    }
    a->array[a->used++] = element;
}

void freeArray(Array *a)
{
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

char readFat(int cluster){

};
void writeFat(int cluster, char content){

};

void cria_arquivo(METADADOS dados, FILE *arq)
{
    int i, j;
    char val = 0;

    //criacao do arquivo

    if (!(arq = fopen("arquivo.bin", "wb"))) //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
    {
        fwrite(&dados, sizeof(METADADOS), 1, arq); // printa os valores de metadado que sao: 256 (de 0 a 255) , 4,0, 32768
        // int numberOfClusters = pattern;
        int numberOfClusters = 1;
        for (i = 0; i < numberOfClusters; i++)
        {                                     //qntidade de clusters
            for (j = 0; j < clusterSize; j++) // cada cluster
            {
                fwrite(&val, sizeof(char), 1, arq); //printa o valor zero de todos os clusters para determinar o tamanho m�x do arquivo
            }
        }
    }
    fclose(arq); // fecha arquivo
}

void le_metadados(FILE *arq)
{
    int i, j;
    char val = 0;
    METADADOS leitura;

    //criacao do arquivo

    if (!(arq = fopen("arquivo.bin", "rb"))) //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
        fread(&leitura, sizeof(METADADOS), 1, arq);

    printf("%d %d %d %d \n", leitura.tam_indice, leitura.tam_cluster, leitura.indiceStartice, leitura.inicio_root); // le tam_indice como -1 pois est� em complemento de 2 (=255)

    fclose(arq); // fecha arquivo
}

// p = 0
// a = 1

// Perguntar:
// - Cluster de continuação de pastas
// - Cluster de continuação de arquivo

ITEM *readFolderContent(FILE *arq)
{
    //0 - 1101010100101101001
    //hF    p/a    end1  end2        ( Nome )
    // fread (void *buffer, int numero_de_bytes, int count, FILE *fp);

    ITEM arr[256];
    ITEM *i;
    bool t = 1;
    int tt = 0;

    //printf("posição chegada: %d \n", ftell(arq));
    while (t)
    {
        fseek(arq, 1, SEEK_CUR);
        printf("%i\n", tt);
        tt += 1;
        printf("%d\n", ftell(arq));
        fread(i, sizeof(ITEM), 1, arq);
        printf('%s\n', i->nome);
        //printf("ler_bit: %d \n", i->ler_bit);
        //printf("posição apos leitura: %d \n", ftell(arq));
        if (!(i->ler_bit) /*|| (i > clusterSize / sizeof(ITEM))*/)
        {
            printf("acabou os itens\n");
            t = 0;
            fseek(arq, -(sizeof(ITEM)), SEEK_CUR);
        }

        else
        {
            printf('%s\n', i->nome);
            arr[0].eh_pasta_bit = i->eh_pasta_bit;
            arr[0].ler_bit = i->ler_bit;
            strcpy(arr[0].nome, i->nome);
            arr[0].ponteiro = i->ponteiro;
        }
    }

    return &arr;
}

void goToCluster(FILE *arq, int n)
{
    int metadataDesloc = sizeof(METADADOS) - 1;
    int fatTableDesloc = pattern;
    int clusterDesloc = (n * clusterSize) / 8;
    int desloc = metadataDesloc + fatTableDesloc + clusterDesloc;
    printf("desloc: %d \n", desloc);
    fseek(arq, desloc, SEEK_SET); //deslocamento medido em bytes
    printf("going to cluster %d \n", n);
}

void writeItemInFolderCluster(FILE *arq, int cluster, ITEM item)
{
    goToCluster(arq, cluster);
    bool *i;

    while (i)
    {
        fread(i, sizeof(bool), 1, arq);
        fseek(arq, (7 * sizeof(bool)), SEEK_CUR);
    }

    fwrite(&item, sizeof(ITEM), 1, arq);
}

void writeItemInCurrFolder(FILE *arq, ITEM *item)
{
    ITEM *i;
    bool t = 1;

    //printf("posição chegada: %d \n", ftell(arq));
    while (t)
    {
        fread(i, sizeof(ITEM), 1, arq);
        //printf("ler_bit: %d \n", i->ler_bit);
        //printf("posição apos leitura: %d \n", ftell(arq));
        if (!(i->ler_bit) /*|| (i > clusterSize / sizeof(ITEM))*/)
        {
            t = 0;
            fseek(arq, -(sizeof(ITEM)), SEEK_CUR);
        }
    }

    //printf("posição recuo: %d \n", ftell(arq));
    //(*item).ponteiro = 11; // navegar a fat até achar local para criar a pasta/item

    fwrite(&item, sizeof(ITEM), 1, arq);

    //printf("posição pos escrita: %d \n", ftell(arq));
}

void CD(FILE *arq, char *comando)
{
}

void DIR(FILE *arq)
{
}

void RM() {}

// void MKDIR(FILE *arq, char *name)
// {
//     ITEM *temp;

//     (*temp).ler_bit = 1;
//     (*temp).eh_pasta_bit = 1;
//     strcpy(temp.nome, name);

//     writeItemInCurrFolder(arq, &temp);
// }

void MKFILE() {}

// void RENAME(FILE *arq, char *name, char *newname)
// {
//     int i = 0;
//     ITEM item;

//     while (1)
//     {
//         fread(&item, sizeof(ITEM), 1, arq);
//         if (item.ler_bit || (i > clusterSize / sizeof(ITEM)))
//             break;

//         else
//         {
//             if (strcmp(&item.nome, name))
//             {
//                 strcpy(&item.nome, newname);
//                 fseek(arq, -(sizeof(ITEM)), SEEK_CUR);
//                 fwrite(&item, sizeof(ITEM), 1, arq);
//             }
//         }
//         i += 1;
//     }
// }

void MOVE() {}

void EDIT() {}

void interfaceLoop()
{
    while (1)
    {
        printf(">>>>>>>>> ");
        scanf("%s");
        //string stroke
    }
}

int main()
{

    FILE *arq;
    METADADOS dados = {pattern, clusterSize, indiceStart, rootPos};

    cria_arquivo(dados, arq);

    arq = fopen("arquivo.bin", "r+");

    printf("#1\n");
    goToCluster(arq, 0);

    printf("#2\n");
    ITEM *item;
    item = malloc(sizeof(ITEM));
    item->ler_bit = true;
    item->eh_pasta_bit = true;
    item->ponteiro = 255;
    strcpy(item->nome, "Item1");
    writeItemInCurrFolder(arq, item);

    printf("#3\n");
    goToCluster(arq, 0);
    printf("#4\n");
    ITEM *folderItems = readFolderContent(arq);
    printf("#5\n");
    printf("nome: %s \n", (folderItems[0]).nome);

    fclose(arq);

    // [ 1010000000000000000000000010 ]
    // se for pasta na hora de criar item no diretorio atual -> entrar no cluster da pasta e inicializar pasta (., ..)

    //interfaceLoop();

    //le_metadados(arq);
}
