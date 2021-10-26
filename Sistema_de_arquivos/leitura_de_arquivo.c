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
char currentPath[1024000] = "/";

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
    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    fseek(arq, ((cluster + 9) * sizeof(char)), SEEK_SET); // va para byte 9 (inicio do indice +1 (pula o zero))+posicao
    fread(&ponteiro, sizeof(short), 1, arq);

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
    return ponteiro;
};

void writeFat(FILE *arq, int cluster, char content)
{
    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    fseek(arq, ((cluster + 9) * sizeof(char)), SEEK_SET); // va para byte 9 (inicio do indice +1 (pula o zero))+posicao
    fwrite(&content, sizeof(char), 1, arq);

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
};

short *busca_arquivo_pasta(FILE *arq, char *name)
{
    int i = 0;
    short *pont_arq1;
    ITEM item;
    int val = sizeof(ITEM);

    while (i == 0)
    {
        fread(&item, sizeof(ITEM), 1, arq);

        if (!(item.ler_bit) || (i > clusterSize / sizeof(ITEM)))
            break;
        else
        {
            if (!(strcmp(item.nome, name)))
            {
                fseek(arq, -(val), SEEK_CUR);
                *pont_arq1 = (short)ftell(arq);
            }
            i++;
        }
    }

    return pont_arq1;
}

void apagaCluster(FILE *arq) // Apaga o cluster atual
{
    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    int i, val = 0;
    for (i = 0; i < clusterSize / 8; i++)
    {
        fwrite(&val, sizeof(char), 1, arq);
    }

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
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
        int numberOfClusters = pattern;

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
    int trashIndex = clusterDesloc + rootPos;
    //printf("%i \n", trashIndex);
    //printf("trashIndex: %d \n", trashIndex);
    fseek(arq, trashIndex, SEEK_SET); //deslocamento medido em bytes
    printf("\0");
    printf("going to cluster %d \n", n);
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

    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    fseek(arq, (9 * sizeof(char)), SEEK_SET); // va para byte 9 -> inicio do indice +1 (pula o zero)

    fread(&preenchimento, sizeof(char), 1, arq);

    printf("\n charrr %s \n", preenchimento);

    while (!(preenchimento == '\0') && indice <= 254) //não vai para 255 pois este indica arquivo corrompido
    {
        fread(&preenchimento, sizeof(char), 1, arq);
        indice++;
    }

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
    return indice;
}

ITEM *readFolderContent(FILE *arq)
{
    ITEM vazio;
    vazio.eh_pasta_bit = 0;
    vazio.ler_bit = 0;
    vazio.ponteiro = 0;
    strcpy(vazio.nome, "");

    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo
    int volta = ftell(arq);
    //printf("chegou na read com posicao %i\n", volta);
    fseek(arq, sizeof(clmeta), SEEK_CUR);
    // fread (void *buffer, int numero_de_bytes, int count, FILE *fp);
    int texx;
    static ITEM arr[256];

    for (texx = 0; texx < 256; texx++)
    {
        arr[texx] = vazio;
    }

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

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
    fseek(arq, volta, SEEK_SET);

    return arr;
}

void writeItemInFolderCluster(FILE *arq, int cluster, ITEM item)
{
    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    goToCluster(arq, cluster);
    bool *i;

    while (i)
    {
        fread(i, sizeof(bool), 1, arq);
        fseek(arq, (7 * sizeof(bool)), SEEK_CUR);
    }

    fwrite(&item, sizeof(ITEM), 1, arq);

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
}

void writeItemInCurrFolder(FILE *arq, ITEM *item)
{
    ITEM *i;
    i = malloc(sizeof(ITEM));

    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo
    long volta = ftell(arq);

    bool t = 1;
    int clusteratual = (ftell(arq) - rootPos) / clusterSize;
    fseek(arq, sizeof(clmeta), SEEK_CUR); // pula metadados
    int fatpos = 0;
    long posant = 0;
    //printf("cluster atual : %i \n", clusteratual);

    fread(i, sizeof(ITEM), 1, arq);

    t = i->ler_bit;

    //printf("posição chegada: %d \n", ftell(arq));
    while (t) // Acha a primeira posição onde não tem um item escrito
    {
        t += 1;
        fread(i, sizeof(ITEM), 1, arq);
        t = i->ler_bit;
    }

    //printf("%i\n", sizeof(ITEM));
    //printf("ler_bit: %d \n", i->ler_bit);
    //printf("posição apos leitura: %d \n", ftell(arq));
    // if (!(i->ler_bit) /*|| (i > clusterSize / sizeof(ITEM))*/)
    // {
    //     t = 0;

    fseek(arq, -(sizeof(ITEM)), SEEK_CUR);

    //printf("item nome: %s \n", item->nome);
    //printf("posição recuo: %d \n", ftell(arq));
    //(*item).ponteiro = 11; // navegar a fat até achar local para criar a pasta/item
    //printf("escrita comecou na posicao: %d \n", ftell(arq));

    // guarda posicao atual
    posant = ftell(arq);

    // acha cluster fat
    fatpos = analisa_fat(arq);

    printf("%i", fatpos);

    item->ponteiro = fatpos;

    // se eh pasta
    if (item->eh_pasta_bit)
    {
        IniciaCluster(arq, clusteratual, fatpos);
        writeFat(arq, fatpos, fatpos);
        fseek(arq, posant, SEEK_SET);
        fwrite(item, sizeof(ITEM), 1, arq);
    }

    // se eh arquivo
    else
    {
        //fseek(arq, (9 * sizeof(char) + (fatpos * sizeof(char))), SEEK_SET);
        //fwrite(&item->ponteiro, sizeof(short), 1, arq);
        writeFat(arq, fatpos, fatpos);
        fseek(arq, posant, SEEK_SET);
        fwrite(item, sizeof(ITEM), 1, arq);
    }

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
    fseek(arq, volta, SEEK_SET);

    //printf("final da write");
    //goToCluster(arq, clusteratual);

    //printf("escrita terminou na posicao: %d \n", ftell(arq));
}

//printf("posição pos escrita: %d \n", ftell(arq));

char *goToFolderAndReturnLastNameOfPath(FILE *arq, char directoryPath[maxCharsInFileSystem])
{
    // pastaatual/texto.txt -> texto.txt
    // pastaatual/pasta1/texto.txt -> (vai ir para pasta1) + return texto.txt
    const char breakChar[1] = "/";
    if (directoryPath[0] == breakChar)
        goToCluster(arq, 0); // absolutePath
    char *currentDir;
    char *lastNameOfPath;
    currentDir = strtok(directoryPath, breakChar);
    while (currentDir != NULL)
    {
        char *nextDir = strtok(NULL, breakChar);
        if (nextDir == NULL)
        {
            return currentDir;
        }

        ITEM *folderItems = readFolderContent(arq);
        int x = folderItems[0].ler_bit;
        int i = 0;

        while (x)
        {
            // printf("%s ", folderItems[i].nome);
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
    return currentDir;
}

void CD(FILE *arq, char directoryPath[maxCharsInFileSystem])
{
    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    printf("CD to %s\n", directoryPath);
    char *lastFolderNameOfPath = goToFolderAndReturnLastNameOfPath(arq, directoryPath);
    ITEM *folderItems = readFolderContent(arq);
    int x = folderItems[0].ler_bit;
    int i = 0;

    while (x)
    {
        if (!strcmp(folderItems[i].nome, lastFolderNameOfPath))
        {
            goToCluster(arq, folderItems[i].ponteiro);
            printf("%s \n", folderItems[i].nome);
            printf("%i \n", folderItems[i].ponteiro);
            break;
        }
        i += 1;
        x = folderItems[i].ler_bit;
    }

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
}

void DIR(FILE *arq)
{
    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    ITEM *folderItens = readFolderContent(arq);

    int x = folderItens[0].ler_bit;
    int i = 0;

    printf("\n--- DIR ---\n");

    while (x)
    {
        printf("%s \n", folderItens[i].nome);
        i += 1;
        x = folderItens[i].ler_bit;
    }

    printf("-----------\n");

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
}

/*
void RM(FILE *arq, char *name) // recebe um ponteiro para o inicio do folder que contem o item que será removido e o nome deste item
{
    //int startPoint = ftell(arq);
    //fseek(arq, startPoint, SEEK_SET);
      
    int i;
    int trashIndex; // indice na lista de itens na pasta parente do lixo
    int trashPointer; // ponteiro dentro do arquivo para o lixo

    short pAtual, pProx; // ponteiros usados para navegar pelos clusters usados nos .txt

    ITEM *parentFolderItems = readFolderContent(arq); // Pega os itens do parente
    ITEM trash; // Item do lixo
    ITEM content; // Primeiro item lido da pasta
    ITEM lastOne; // Ultimo item da pasta

    ITEM vazio; // Cria um item vazio

    vazio.eh_pasta_bit = 0;
    vazio.ler_bit = 0;
    vazio.ponteiro = 0;
    strcpy(vazio.nome, ""); 

    
    i = 0 // Acha o indice do item do arquivo que deve ser removido
    while(!strcmp(parentFolderItems[i].nome, name))
    {
        i++;
    }
    
    trashIndex = i // Guarda o indice do lixo
    fseek(arq, i*sizeof(ITEM) + sizeof(clmeta), SEEK_CUR); // Vai até o lixo
    trashPointer = ftell(arq); // Guarda a posicao do lixo para uso futuro
    
    fread(&trash, sizeof(ITEM), 1, arq); // Le o item do arquivo que deve ser removido
    goToCluster(arq, trash.ponteiro); // Vai até o cluster que deve ser limpo 
    
    if(trash.eh_pasta_bit) // Testa se o item a ser removido é uma pasta
    {
        fseek(arq, sizeof(clmeta), SEEK_CUR) // Vai até o primeiro item da pasta
        fread(&content, sizeof(ITEM), 1, arq) // Le o primeiro item
        
        if(content.ler_bit) // Testa se a pasta está vazia
        {
            // Se a pasta está vazia, não faz nada
            return;
        }
        else
        {
            fseek(arq, -sizeof(clmeta) -sizeof(ITEM), SEEK_CUR); // Volta para o inicio da pasta
            
            apagaCluster(arq);  // Limpa o cluster atual
            writeFat(arq, trash.ponteiro, 0); // Retira o clustel atual da FAT
        }
    }
    else // Caso seja um arquivo de texto
    {
        pProx = trash.ponteiro;

        do {
            
            pAtual = pProx; // Atualiza o ponteiro atual
            pProx = readFat(arq, pAtual); // Acha qual é o próximo ponteiro
            
            apagaCluster(arq); // Limpa o cluster atual

            writeFat(arq, pAtual, 0); // Apaga o cluster atual da FAT
            
            gotoCluster(arq, pProx); // Vai para o próximo cluster

        } while (pAnt != pProx) // Enquanto não chegar no end-of-file (quando o ponteiro apontar para o próprio indice)
    }
    
    fseek(arq, trashPointer, SEEK_SET); // Volta para o item que deve ser apagado
    
    while(parentFolderItems[i+1].ler_bit) // Procura pelo ultimo item da pasta
    {
        i++;
    }

    if (i != trashIndex) // Testa se foi achado um item além do lixo
    {
        fseek(arq, (i-trashIndex-1)*sizeof(ITEM), SEEK_CUR) // Vai ate o ultimo item 
        fread(&lastOne, sizeof(ITEM), 1, arq); // Le o ultimo item do arquivo
        fwrite(vazio, sizeof(ITEM), 1, arq-sizeof(ITEM)); // Apaga o ultimo item da pasta

        fseek(arq, trashPointer, SEEK_SET) // Volta até o lixo
        fwrite(lastOne, sizeof(ITEM), 1, arq); // Subscreve o lixo
    }
    else
    {
        fwrite(vazio, sizeof(ITEM), 1, arq); // Apaga o item do arquivo removido
    }
    
}
*/

void MKDIR(FILE *arq, char *name)
{
    ITEM *temp = malloc(sizeof(ITEM));
    temp->ler_bit = true;
    temp->eh_pasta_bit = true;
    strcpy(temp->nome, name);
    //printf("%s", temp->nome);
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

    /*
    int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo
    char *fileName = goToFolderAndReturnLastNameOfPath(fileDir);
    int fileFolderStartPoint = ftell(arq); // Guarda a posicao inicial no arquivo
    ITEM *folderItems = readFolderContent(arq);
    int x = folderItems[0].ler_bit;
    int i = 0;

    while (x)
    {
        if (!strcmp(folderItems[i].nome, fileName))
        {
            goToCluster(arq, folderItems[i].ponteiro);
            printf("%s \n", folderItems[i].nome);
            printf("%i \n", folderItems[i].ponteiro);
            break;
        }
        i += 1;
        x = folderItems[i].ler_bit;
    }

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
    */

    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo
    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
}

void RENAME(FILE *arq, char *name, char *newname)
{
    //int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

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

    //fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
}
/*
void MOVE(FILE *arq, char *name , char *name_destino){
// copiar arquivo
// mover para novo diretorio
// excluir do diretorio antigo


    char *caminho= "";
    char *path;
    int *pos1,*pos2;
    char ch;
    ITEM item;
    int zero=0;

    path = malloc(sizeof(caminho));

    strcpy(path,caminho);

    strcat(caminho,name);
    strcat(caminho,path);
    strcat(caminho,name_destino);

    *pos1=busca_arquivo_pasta(arq,name);
    *pos2=busca_arquivo_pasta(arq,name_destino);

    while ((fread(&ch, sizeof(char), 1, arq)) != EOF)
       {
           fseek(arq,pos1,SEEK_SET);
           fread(&ch, sizeof(char), 1, arq);
           *pos1=*pos1+ sizeof(char);
           fwrite(&val, sizeof(char), 1, arq); //escreve zero
           fseek(arq,pos2,SEEK_SET);
           fwrite(&ch, sizeof(char), 1, arq);


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

    // short fat = analisa_fat(arq);

    // printf("fatpos antes: %d\n", fat);

    // writeFat(arq, fat, 'c');

    // fat = analisa_fat(arq);

    // printf("fatpos dps: %d", fat);

    // printf("teste \n");

    fclose(arq);
    return 8;
    goToCluster(arq, 0);

    // printf("teste2");
    // printf("teste2");

    // printf("teste2");
    // printf("AI");

    ITEM *item;
    item = malloc(sizeof(ITEM));
    item->ler_bit = true;
    item->eh_pasta_bit = true;
    strcpy(item->nome, "Item123");

    MKDIR(arq, item->nome);

    goToCluster(arq, 0);

    strcpy(item->nome, "Item13222");
    MKDIR(arq, item->nome);

    printf("Agora kkkk\n");
    MKDIR(arq, "kkkk");

    // goToCluster(arq, 0);

    DIR(arq);

    CD(arq, "Item123");
    DIR(arq);
    // printf("!\n");
    strcpy(item->nome, "subp");
    //goToCluster(arq, 1);
    //writeItemInCurrFolder(arq, item);
    MKDIR(arq, "teste1");

    printf("Agora teste2:\n");
    MKDIR(arq, "teste2");
    printf("Agora teste3:\n");
    MKDIR(arq, "teste3");
    CD(arq, "teste1");
    goToCluster(arq, 0);
    CD(arq, "kkkk");
    // printf("!!\n");
    DIR(arq);
    // CD(arq, "Item13222");

    // ITEM *folderItens = readFolderContent(arq);
    //printf("nome: %s \n", folderItens[0].nome);
    //printf("Fat: %d", analisa_fat(arq));

    fclose(arq);
}

// corrompido = 255
//