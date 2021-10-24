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

typedef struct clmeta
{
    int ponto;
    int pontoponto;
} clmeta;

readFat(int cluster){

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
    fseek(arq, sizeof(clmeta), SEEK_CUR);
    // fread (void *buffer, int numero_de_bytes, int count, FILE *fp);

    static ITEM arr[256];
    ITEM i;
    int x = 0;
    bool t = 1;

    //printf("posição chegada: %d \n", ftell(arq));
    while (t)
    {
        //printf("inicio da leitura: %d\n", ftell(arq));
        fread(&i, sizeof(ITEM), 1, arq);
        //printf("eh pasta bit: %i \n", i.eh_pasta_bit);

        //printf("nome: %s \n", i.nome);
        //printf("ponteiro: %d \n", i.ponteiro);
        //printf('%s\n', i.nome);

        //printf("ler_bit: %d \n", i->ler_bit);
        //printf("posição apos leitura: %d \n", ftell(arq));

        /*|| (i > clusterSize / sizeof(ITEM))*/

        if (!i.ler_bit)
        {
            //printf("acabou os itens\n");
            t = 0;
            fseek(arq, -(sizeof(ITEM)), SEEK_CUR);
        }

        else
        {
            arr[x].eh_pasta_bit = i.eh_pasta_bit;
            arr[x].ler_bit = i.ler_bit;
            strcpy(arr[x].nome, i.nome);
            arr[x].ponteiro = i.ponteiro;
            x += 1;
        }
    }

    return arr;
}

void goToCluster(FILE *arq, int n)
{
    int metadataDesloc = sizeof(METADADOS) - 1;
    int fatTableDesloc = pattern;
    int clusterDesloc = (n * clusterSize) / 8;
    int desloc = metadataDesloc + fatTableDesloc + clusterDesloc;
    //printf("desloc: %d \n", desloc);
    fseek(arq, desloc, SEEK_SET); //deslocamento medido em bytes
    //printf("going to cluster %d \n", n);
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
    fseek(arq, sizeof(clmeta), SEEK_CUR);
    ITEM *i;
    bool t = 1;
    int clusteratual = (ftell(arq) - 263) / clusterSize;
    //printf("cluster atual : %i \n", clusteratual);

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

    //printf("item nome: %s \n", item->nome);
    //printf("posição recuo: %d \n", ftell(arq));
    //(*item).ponteiro = 11; // navegar a fat até achar local para criar a pasta/item
    //printf("escrita comecou na posicao: %d \n", ftell(arq));

    fwrite(item, sizeof(ITEM), 1, arq);

    if (item->eh_pasta_bit)
    {
        IniciaCluster(arq, clusteratual, i->ponteiro);
    }

    //printf("escrita terminou na posicao: %d \n", ftell(arq));
}

//printf("posição pos escrita: %d \n", ftell(arq));

/*}
void CD(FILE *arq, char *comando)
    /* ITEM *folderItems = readFolderContent(arq);/
ra                                          //    eadfFldercCntent\()// }

*/

void IniciaCluster(FILE *arq, int anterior, int novo)
{
    goToCluster(arq, novo);
    //. ..
    fwrite(&novo, sizeof(int), 1, arq);
    fwrite(&anterior, sizeof(int), 1, arq);

    return;
}

void DIR(FILE *arq)
{
    ITEM *folderItens = readFolderContent(arq);

    int x = folderItens[0].ler_bit;
    int i = 0;

    printf(">>>>>>>>> ");

    while (x)
    {
        printf("%s ", folderItens[i].nome);
        i += 1;
        x = folderItens[i].ler_bit;
    }

    printf("\n");
}

/*
void RM(FILE *arq, char *name)
{
    // Limpa um arquivo ou diretorio
    // Limpa a tabela FAT
    // Limpa a diretorio parente

    ITEM *parentFolderItems = readFolderContent(arq); // Pega os itens do parente
    ITEM lixo;
    
    i = 0 // Acha a posição do arquivo que deve ser removido
    while(strcmp(parentFolderItems[i].nome, name))
    {
        i++;
    }
    
    int freePlace = i*sizeof(ITEM) + sizeof(clmeta); // Calcula posição do item que será removido
    fseek(arq, desloc, SEEK_CUR); // Vai até o item que será removido
    
    // while(parentFolderItems[i].ler_bit) // Procura pelo próximo item que possa vir para a posição livre
    // {
    //    i++;
    // }
    // i--;
    
    fread(son, sizeof(ITEM), 1, arq);
    
    
    
}
*/

void MKDIR(FILE *arq, char *name)
{
    ITEM temp;
    temp.ler_bit = 1;
    temp.eh_pasta_bit = 1;
    strcpy(temp.nome, name);

    writeItemInCurrFolder(arq, &temp);
}

// void MKFILE(FILE *arq, )
// {
//     ITEM *item;
//     item = malloc(sizeof(ITEM));
//     item->ler_bit = true;
//     item->eh_pasta_bit = false;
//     item->ponteiro = 2;
//     strcpy(item->nome, "TESTE");
//     writeItemInCurrFolder(arq, item);
// }
void EDIT()
{
}

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

// i += 1;
// // }
// FILE *arq, //
//char *namet//   / /,void MOextVE() {}

// void interfaceLoop()
// {
//     while (1)
//     {
//         printf(">>>>>>>>> ");
//         scanf("%s");
//         //string stroke
//     }
// }

// retorna de 1 a 256
int analisa_fat(FILE *arq)
{
    char preenchimento;
    int indice = 1;

    fseek(arq, (8 * sizeof(char)), SEEK_SET); // va para byte 8 -> inicio do indice

    fread(&preenchimento, sizeof(char), 1, arq);

    while (preenchimento != 0)
    {
        fread(&preenchimento, sizeof(char), 1, arq);
        indice++;
    }

    return indice;
}

int main()
{
    FILE *arq;
    METADADOS dados = {pattern, clusterSize, indiceStart, rootPos};

    cria_arquivo(dados, arq);

    arq = fopen("arquivo.bin", "r+");

    goToCluster(arq, 0);
    printf("\n");

    ITEM *item;
    item = malloc(sizeof(ITEM));
    item->ler_bit = true;
    item->eh_pasta_bit = true;
    item->ponteiro = 2;
    strcpy(item->nome, "TESTE");
    writeItemInCurrFolder(arq, item);

    item->ler_bit = true;
    item->eh_pasta_bit = true;
    item->ponteiro = 33;
    strcpy(item->nome, "Item123232");

    goToCluster(arq, 0);
    printf("\n");

    writeItemInCurrFolder(arq, item);

    goToCluster(arq, 0);
    printf("\n");

    DIR(arq);

    printf("#4\n");

    // ITEM *folderItens = readFolderContent(arq);
    //printf("nome: %s \n", folderItens[0].nome);
    printf("Fat: %d", analisa_fat(arq));

    fclose(arq);
}
