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
#define maxCharsInFileSystem 1024000

// int fseek (FILE *fp,long numbytes,int origem);
int currentFolderCluster = 0;

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

short readFat(FILE *arq, int cluster)
{
    short ponteiro;

    fseek(arq, ((cluster + 9) * sizeof(char)), SEEK_SET); // va para byte 9 (inicio do indice +1 (pula o zero))+posicao
    fread(&ponteiro, sizeof(short), 1, arq);

    return ponteiro;
};

void writeFat(FILE *arq, int cluster, char content)
{

    fseek(arq, ((cluster + 9) * sizeof(char)), SEEK_SET); // va para byte 9 (inicio do indice +1 (pula o zero))+posicao
    fwrite(&content, sizeof(char), 1, arq);
};

void apagaCluster(FILE *arq) // Apaga o cluster atual
{
    int i, val = 0;
    for (i = 0; i < clusterSize / 8; i++)
    {
        fwrite(&val, sizeof(char), 1, arq);
    }
};

void cria_arquivo(METADADOS dados, FILE *arq)
{
    int i;

    //criacao do arquivo

    if (!(arq = fopen("arquivo.bin", "wb"))) //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
    {
        fwrite(&dados, sizeof(METADADOS), 1, arq); // printa os valores de metadado que sao: 256 (de 0 a 255) , 4,0, 32768
        // int numberOfClusters = pattern;
        int numberOfClusters = 1;
        for (i = 0; i < numberOfClusters; i++)
        { //qntidade de clusters
            apagaCluster(arq);
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

void goToCluster(FILE *arq, int n)
{
    int x = n + 1;
    int clusterDesloc = ((x * clusterSize) / 8);
    clusterDesloc -= clusterSize / 8;
    int desloc = clusterDesloc + rootPos;
    //printf("%i \n", desloc);
    //printf("desloc: %d \n", desloc);
    fseek(arq, desloc, SEEK_SET); //deslocamento medido em bytes
    printf("\0");
    //printf("going to cluster %d \n", n);
}

// p = 0
// a = 1

// Perguntar:
// - Cluster de continuação de pastas
// - Cluster de continuação de arquivo
void IniciaCluster(FILE *arq, int anterior, int novo)
{
    // marcar como ocupado na fat
    goToCluster(arq, novo);

    //. ..
    fwrite(&novo, sizeof(int), 1, arq);
    fwrite(&anterior, sizeof(int), 1, arq);
    fseek(arq, (9 * sizeof(char) + (novo * sizeof(char))), SEEK_SET);

    return;
}

int analisa_fat(FILE *arq)
{
    char preenchimento;
    int indice = 1;

    fseek(arq, (9 * sizeof(char)), SEEK_SET); // va para byte 9 -> inicio do indice +1 (pula o zero)

    fread(&preenchimento, sizeof(char), 1, arq);

    while (preenchimento != 0 && indice <= 254) //não vai para 255 pois este indica arquivo corrompido
    {
        fread(&preenchimento, sizeof(char), 1, arq);
        indice++;
    }

    return indice;
}

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
    int clusteratual = (ftell(arq) - rootPos) / clusterSize;
    fseek(arq, sizeof(clmeta), SEEK_CUR); // pula metadados
    int fatpos = 0;
    int posant = 0;
    //printf("cluster atual : %i \n", clusteratual);

    //printf("posição chegada: %d \n", ftell(arq));
    while (t) // Acha a primeira posição onde não tem um item escrito
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

    // guarda posicao atual
    posant = ftell(arq);

    // acha cluster fat
    fatpos = analisa_fat(arq);

    item->ponteiro = fatpos;

    // se eh pasta
    if (item->eh_pasta_bit)
    {
        IniciaCluster(arq, clusteratual, fatpos);
        fwrite(&item->ponteiro, sizeof(short), 1, arq); // escreve na fat que ela termina em si
        fseek(arq, posant, SEEK_SET);
        fwrite(item, sizeof(ITEM), 1, arq);
    }

    // se eh arquivo
    else
    {
        fseek(arq, (9 * sizeof(char) + (fatpos * sizeof(char))), SEEK_SET);
        fwrite(&item->ponteiro, sizeof(short), 1, arq);
        fseek(arq, posant, SEEK_SET);
        fwrite(item, sizeof(ITEM), 1, arq);
    }

    goToCluster(arq, clusteratual);

    //printf("escrita terminou na posicao: %d \n", ftell(arq));
}

//printf("posição pos escrita: %d \n", ftell(arq));
/*
void CD(FILE *arq, char directoryPath[maxCharsInFileSystem])
{
    char *lastFolderNameOfPath = goToFolderAndReturnLastNameOfPath(arq, directoryPath);
    ITEM *folderItems = readFolderContent(arq);
    int x = folderItems[0].ler_bit;
    int i = 0;

    while (x)
    {
        printf("%s ", folderItems[i].nome);
        if (!strcmp(folderItems[i].nome, lastFolderNameOfPath))
        {
            goToCluster(arq, folderItems[i].ponteiro);
            break;
        }
        i += 1;
        x = folderItems[i].ler_bit;
    }
}

char *goToFolderAndReturnLastNameOfPath(FILE *arq, char directoryPath[maxCharsInFileSystem])
{
    const char breakChar[1] = "/";

    if (directoryPath[0] == breakChar)
        goToCluster(arq, 0); // absolutePath

    char *currentDir;
    currentDir = strtok(directoryPath, breakChar);
    while (currentDir != NULL)
    {
        char *nextDir = strtok(directoryPath, breakChar);

        ITEM *folderItems = readFolderContent(arq);
        int x = folderItems[0].ler_bit;
        int i = 0;

        while (x)
        {
            printf("%s ", folderItems[i].nome);
            if (!strcmp(folderItems[i].nome, currentDir))
            {
                goToCluster(arq, folderItems[i].ponteiro);
                break;
            }
            i += 1;
            x = folderItems[i].ler_bit;
        }

        currentDir = nextDir;
    }
    return "oi";
}

*/

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

    int i;
    short pAtual, pProx;

    ITEM *parentFolderItems = readFolderContent(arq); // Pega os itens do parente
    ITEM lixo;
    ITEM conteudo;

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
    
    fread(&lixo, sizeof(ITEM), 1, arq); // Le o item do arquivo que deve ser removido
    goToCluster(arq, lixo.ponteiro); // Vai até o cluster que deve ser limpo 
    
    if(lixo.eh_pasta_bit) // Testa se o item a ser removido é uma pasta
    {
        fseek(arq, sizeof(clmeta), SEEK_CUR) // Vai até o primeiro item da pasta
        fread(&conteudo, sizeof(ITEM), 1, arq) // Le o primeiro item
        
        if(conteudo.ler_bit) // Testa se a pasta está vazia
        {
            // ### Avisa que a pasta não pode ser apagada porque ela ainda tem conteudo ###
            return;
        }
        else
        {
            fseek(arq, -sizeof(clmeta) -sizeof(ITEM), SEEK_CUR); // Volta para o inicio da pasta
            
            apagaCluster(arq);
        }
    }
    else // Caso seja um arquivo de texto
    {
        pAtual = lixo.ponteiro;
        pProx = 0 // Valor inicial que nunca será igual ao pAtual (pois o 0 é o root);
        while (pAnt != pProx) // Enquanto não chegar no end-of-file (ponteiro aponta para a posição atual)
        {
            pProx = readFat(arq, pAtual); // Acha qual é o próximo ponteiro
            writeFat(arq,)
            apagaCluster(arq); // Limpa o cluster atual
            
            gotoCluster(arq, pProx); // Vai para o próximo cluster
        }
    }
    

    

    
}
*/

void MKDIR(FILE *arq, char *name)
{
    ITEM *temp;
    temp = malloc(sizeof(ITEM));
    temp->ler_bit = true;
    temp->eh_pasta_bit = true;
    strcpy(temp->nome, name);
    printf("%s", temp->nome);
    writeItemInCurrFolder(arq, temp);
}

void MKFILE(FILE *arq, char fileName[16])
{
    ITEM *item;
    item = malloc(sizeof(ITEM));
    item->ler_bit = true;       //true
    item->eh_pasta_bit = false; //false
    item->ponteiro = 0;
    strcpy(item->nome, fileName);
    writeItemInCurrFolder(arq, item);
}

void EDIT(FILE *arq, char fileDir[maxCharsInFileSystem], char fileContent[maxCharsInFileSystem])
{
    //encontrar cluster de acordo com diretorio
    //-goToFolderAndReturnLastNameOfPath
    //limpar cluster
    //-apagaCluster
    //gravar no diretorio
    //-goToCluster(arq, 0);
    //-while para quebrar arquivo em n clusters
    //--procurar cluster disponivel fat
    //--write
    //--gravar sequenciamento fat
}

void RENAME(FILE *arq, char *name, char *newname)
{
    int i = 0;
    ITEM item;

    while (1)
    {
        fread(&item, sizeof(ITEM), 1, arq);
        if (!(item.ler_bit) || (i > clusterSize / sizeof(ITEM)))
            break;

        else
        {
            if (!(strcmp(item.nome, name)))
            {
                strcpy(item.nome, newname);
                fseek(arq, -(sizeof(ITEM)), SEEK_CUR);
                fwrite(&item, sizeof(ITEM), 1, arq);
            }

            i += 1;
        }
    }
}
/*
void MOVE(FILE *arq, char *name , char *name_destino, int clusteratual, int new_cluster ){
// copiar arquivo
// mover para novo diretorio
// excluir do diretorio antigo 
//atualizar fat

    int i = 0;
    ITEM item;

    while (1)
    {
        fread(&item, sizeof(ITEM), 1, arq);
        if (!(item.ler_bit) || (i > clusterSize / sizeof(ITEM)))
            break;

        else
        {

            i += 1;
        }
    }
    
}
*/
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

// retorna de 1 a 255
void VivaAoC()
{
    printf("\0");
}

int main()
{
    FILE *arq;
    METADADOS dados = {pattern, clusterSize, indiceStart, rootPos};

    cria_arquivo(dados, arq);

    arq = fopen("arquivo.bin", "r+");

    // printf("teste \n");

    goToCluster(arq, 0);

    // printf("teste2");
    // printf("teste2");

    // printf("teste2");
    // printf("AI");

    ITEM *item;
    item = malloc(sizeof(ITEM));
    item->ler_bit = true;
    item->eh_pasta_bit = true;
    item->ponteiro = 33;
    strcpy(item->nome, "Item123");
    MKDIR(arq, item->nome);

    goToCluster(arq, 0);

    strcpy(item->nome, "Item13222");
    MKDIR(arq, item->nome);

    goToCluster(arq, 0);

    DIR(arq);

    // ITEM *folderItens = readFolderContent(arq);
    //printf("nome: %s \n", folderItens[0].nome);
    //printf("Fat: %d", analisa_fat(arq));

    fclose(arq);
}

// corrompido = 255
//