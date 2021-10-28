/*
=========================================================================
                    Sistema de Arquivos Light-FS 
    Arquitetura e Organização de Computadores II - INF01112 - 2021/1

                        Integrantes do grupo:
                    Gabriel Madeira (00322863)
                   Henrique Borges Manzke (00326970)
                   Leticia Cattai Contino (00316207)
                    Lucas Tres de Lima (00326901)

=========================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define pattern 256 //padrao representa o 2^8 que aparece diversas vezes no enunciado do problema -> 0 a 255 =256
#define clusterSize 32768
#define indiceStart 8 // 1 byte tam do indice, 2 bytes tam do cluster, 2 bytes inicio do indice, 2 bytes inicio root  -> prox byte � posicao do indice
#define rootPos 263   // 7 bytes posteriores +256 bytes de indice
#define maxCharsInFileSystem 1024000

int currentFolderCluster = 0;
char currentPath[maxCharsInFileSystem] = "/";

typedef struct file_folder_item_pattern
{
    bool ler_bit : 1;      //indica se ha algo para ler
    bool eh_pasta_bit : 1; // indica se e pasta ou arquivo
    unsigned char ponteiro;
    char nome[16];

} ITEM;

typedef struct metadados
{
    short tam_indice;           // char e implicitamente transformado em um numero  --> 16 bits
    unsigned short tam_cluster; // 1kB= 1024B
    short indiceStartice;       //inicio do indice --> 16 bits
    short inicio_root;          //posicao do primeiro cluster

} METADADOS;

typedef struct clmeta //metadados do cluster
{
    int ponto;      // Ponteiro para o cluster atual na FAT
    int pontoponto; // Ponteiro para o cluster parente na FAT
} clmeta;

unsigned char readFat(FILE *arq, int cluster) // Recebe o nº de um cluster e devolve o ponteiro para sua continuacao
{
    unsigned char ponteiro;
    int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    fseek(arq, ((cluster + 8) * sizeof(char)), SEEK_SET); // va para byte 9 (inicio do indice +1 (pula o zero))+posicao
    fread(&ponteiro, sizeof(unsigned char), 1, arq);

    fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
    return ponteiro;
};

void writeFat(FILE *arq, int cluster, unsigned char content) // Escreve na FAT
{
    int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    fseek(arq, ((cluster + 8) * sizeof(unsigned char)), SEEK_SET); // va para byte 8 +posicao

    fwrite(&content, sizeof(unsigned char), 1, arq); // Altera o valor do ponteiro na FAT

    fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
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
    int startPoint = ftell(arq); // Guarda a posicao inicial no arquivo

    int i, val = 0;
    for (i = 0; i < clusterSize / 8; i++)
    {
        fwrite(&val, sizeof(char), 1, arq);
    }

    //fwrite("endoffolder", sizeof("endoffolder"), 1, arq);

    fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo
};

void cria_arquivo(METADADOS dados, FILE *arq)
{
    int i;

    //criacao do arquivo

    if (!(arq = fopen("fileSystem.bin", "wb"))) //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
    {
        fwrite(&dados, sizeof(METADADOS), 1, arq); // printa os valores de metadado que sao: 256 (de 0 a 255) , 4,0, 32768
        int numberOfClusters = pattern;

        for (i = 0; i < numberOfClusters; i++) // apaga os clusters
        {
            apagaCluster(arq);
        }
    }
    fclose(arq); // fecha arquivo
}

void le_metadados(FILE *arq)
{
    METADADOS leitura;

    if (!(arq = fopen("fileSystem.bin", "rb"))) //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
        fread(&leitura, sizeof(METADADOS), 1, arq);

    printf("%d %d %d %d \n", leitura.tam_indice, leitura.tam_cluster, leitura.indiceStartice, leitura.inicio_root); // le tam_indice como -1 pois est� em complemento de 2 (=255)

    fclose(arq);
}

void goToCluster(FILE *arq, int n) // Recebe o ponteiro para um cluster e
{
    int x = n + 1;
    int clusterDesloc = ((x * clusterSize) / 8);
    clusterDesloc -= clusterSize / 8;
    int desloc = clusterDesloc + rootPos;
    fseek(arq, desloc, SEEK_SET); //deslocamento medido em bytes
    printf("\0");
}

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

unsigned char analisa_fat(FILE *arq) // função que dado um arquivo calcula o indice do cluster em que o ponteiro se encontra
{
    unsigned char preenchimento;
    short indice = 1;

    int startPoint = ftell(arq); // Guarda a posicao inicial do arquivo

    fseek(arq, (9 * sizeof(unsigned char)), SEEK_SET); // va para byte 9 -> inicio do indice +1 (pula o zero)

    fread(&preenchimento, sizeof(unsigned char), 1, arq);

    while (((int)preenchimento != 0) && indice <= 254)
    {
        fread(&preenchimento, sizeof(unsigned char), 1, arq);
        indice++;
    }

    fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial do arquivo
    return indice;
}

ITEM *readFolderContent(FILE *arq)
{
    goToCluster(arq, currentFolderCluster);
    ITEM vazio;
    vazio.eh_pasta_bit = 0;
    vazio.ler_bit = 0;
    vazio.ponteiro = 0;
    strcpy(vazio.nome, "");

    int startPoint = ftell(arq); // Guarda a posicao inicial do arquivo

    fseek(arq, sizeof(clmeta), SEEK_CUR); // Pula os metadados

    int texx;
    static ITEM arr[256];

    for (texx = 0; texx < 256; texx++)
    {
        arr[texx] = vazio;
    }

    ITEM i;
    int x = 0;
    bool t = 1;

    while (t)
    {

        fread(&i, sizeof(ITEM), 1, arq);

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

    fseek(arq, startPoint, SEEK_SET); // Retorna a posicao inicial no arquivo

    return arr;
}

void writeItemInCurrFolder(FILE *arq, ITEM *item)
{
    goToCluster(arq, currentFolderCluster);

    fseek(arq, sizeof(clmeta), SEEK_CUR); // pula metadados
    ITEM *i;
    i = malloc(sizeof(ITEM));
    int desloc = 0;
    int anterior = ftell(arq);
    bool t = 1;

    int fatpos = 0;
    long posant = 0;

    fread(i, sizeof(ITEM), 1, arq);
    if (!strcmp(i->nome, item->nome))
    {
        printf("Nao sao permitidos nomes repetidos\n");
        goToCluster(arq, currentFolderCluster);
        return;
    }

    t = i->ler_bit;
    while (t) // Acha a primeira posição onde não tem um item escrito
    {
        if (!strcmp(i->nome, item->nome))
        {
            goToCluster(arq, currentFolderCluster);
            printf("Nao sao permitidos nomes repetidos\n");
            return;
        }

        desloc += 1;
        fread(i, sizeof(ITEM), 1, arq);
        t = i->ler_bit;
    }

    fseek(arq, anterior, SEEK_SET);
    fseek(arq, desloc * sizeof(ITEM), SEEK_CUR);

    // guarda posicao atual
    posant = ftell(arq);

    // acha cluster fat
    fatpos = analisa_fat(arq);

    item->ponteiro = fatpos;

    // se eh pasta
    if (item->eh_pasta_bit)
    {
        IniciaCluster(arq, currentFolderCluster, fatpos);
        writeFat(arq, fatpos, fatpos);
        fseek(arq, posant, SEEK_SET);
        fwrite(item, sizeof(ITEM), 1, arq);
    }

    // se eh arquivo
    else
    {
        IniciaCluster(arq, currentFolderCluster, fatpos);
        writeFat(arq, fatpos, fatpos);
        fseek(arq, posant, SEEK_SET);
        fwrite(item, sizeof(ITEM), 1, arq);
    }

    goToCluster(arq, currentFolderCluster);
}

void writeInCurrentPath(char *name)
{
    int i;
    if (!strcmp(name, ".."))
    {
        if (currentFolderCluster == 0)
        {
            strcpy(currentPath, "/");
            return;
        }

        strcat(currentPath, name);
        for (i = strlen(currentPath) - 1; i >= 0; i--)
        {
            if (currentPath[i] == '/')
            {
                currentPath[i] = '\0';
                break;
            }
        }
    }
    else if (strcmp(name, "."))
    {
        char temp[maxCharsInFileSystem] = "";
        if (strcmp(currentPath, "/"))
            strcpy(temp, "/");
        strcat(temp, name);
        strcat(currentPath, temp);
    }
}

char *goToFolderAndReturnLastNameOfPath(FILE *arq, char directoryPath[maxCharsInFileSystem], int cd)
{
    // pastaatual/texto.txt -> texto.txt
    // pastaatual/pasta1/texto.txt -> (vai ir para pasta1) + return texto.txt
    const char breakChar[1] = "/";

    if (directoryPath[0] == '/')
    {
        if (directoryPath[1] == '\0')
            return NULL;
        // directoryPath++;
        currentFolderCluster = 0;
        goToCluster(arq, 0); // absolutePath
        strcpy(currentPath, "/");
    }

    char *currentDir;
    currentDir = strtok(directoryPath, breakChar);

    while (currentDir != NULL)
    {
        char *nextDir = strtok(NULL, breakChar);
        if (nextDir == NULL)
        {
            printf("\0", currentDir);
            return currentDir;
        }

        int ponto;
        if (!strcmp(currentDir, "."))
        {
            printf("É .\n");
            if (cd)
                writeInCurrentPath(".");
            goToCluster(arq, currentFolderCluster);
            break;
        }
        if (!strcmp(currentDir, ".."))
        {
            printf("É ..\n");
            goToCluster(arq, currentFolderCluster);
            fseek(arq, sizeof(int), SEEK_CUR);
            fread(&ponto, sizeof(int), 1, arq);

            currentFolderCluster = ponto;
            goToCluster(arq, ponto);
            if (cd)
                writeInCurrentPath("..");
            break;
        }

        ITEM *folderItems = readFolderContent(arq);
        int x = folderItems[0].ler_bit;
        int i = 0;

        while (x)
        {
            if (!strcmp(folderItems[i].nome, currentDir))
            {

                if (!folderItems[i].eh_pasta_bit)
                {
                    printf("%s is not a folder!\n", folderItems[i].nome);
                    return currentDir;
                }
                currentFolderCluster = folderItems[i].ponteiro;
                goToCluster(arq, folderItems[i].ponteiro);
                if (cd)
                    writeInCurrentPath(folderItems[i].nome);
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
    // printf("CD to %s\n", directoryPath);
    int ponto = 0;
    char *lastFolderNameOfPath = goToFolderAndReturnLastNameOfPath(arq, directoryPath, 1);

    if (lastFolderNameOfPath == NULL)
    {
        strcpy(currentPath, "/");
        currentFolderCluster = 0;
        return;
    }

    if (!strcmp(lastFolderNameOfPath, "."))
        return;

    if (!strcmp(lastFolderNameOfPath, ".."))
    {
        goToCluster(arq, currentFolderCluster);
        fseek(arq, sizeof(int), SEEK_CUR);
        fread(&ponto, sizeof(int), 1, arq);

        currentFolderCluster = ponto;
        goToCluster(arq, ponto);
        writeInCurrentPath("..");
        return;
    }

    ITEM *folderItems = readFolderContent(arq);
    int x = folderItems[0].ler_bit;
    int i = 0;

    while (x)
    {
        if (!strcmp(folderItems[i].nome, lastFolderNameOfPath))
        {
            if (!folderItems[i].eh_pasta_bit)
            {
                printf("%s is not a folder!\n", folderItems[i].nome);
                return;
            }

            currentFolderCluster = folderItems[i].ponteiro;
            goToCluster(arq, folderItems[i].ponteiro);
            writeInCurrentPath(folderItems[i].nome);
            break;
        }

        i += 1;
        x = folderItems[i].ler_bit;
    }
}

void DIR(FILE *arq) // Lista todos os itens que estao na pasta
{
    goToCluster(arq, currentFolderCluster);

    printf("\0");
    ITEM *folderItens = readFolderContent(arq); // Pega uma lista com todos os itens do folder

    int x = folderItens[0].ler_bit;
    int i = 0;

    printf("\n--- DIR ---\n");

    while (x)
    {
        printf("%s \n", folderItens[i].nome);
        i += 1;
        x = folderItens[i].ler_bit;
    }
    if (i == 0) // Se nenhum item foi lido
    {
        printf("empty folder\n");
    }

    printf("-----------\n");

    goToCluster(arq, currentFolderCluster);
}

void removeItemFromCurrentCluster(FILE *arq, char *name)
{
    goToCluster(arq, currentFolderCluster);
    fseek(arq, sizeof(clmeta), SEEK_CUR);
    ITEM i;
    int anterior = 0;
    int ultimo = 0;
    int lixo = 0;

    ITEM zero;
    zero.eh_pasta_bit = 0;
    zero.ler_bit = 0;
    zero.ponteiro = 0;
    strcpy(zero.nome, "");

    i.ler_bit = 1;

    while (i.ler_bit)
    {
        anterior = ftell(arq);
        fread(&i, sizeof(ITEM), 1, arq);
        if (!strcmp(i.nome, name))
        {
            lixo = anterior;
        }
    }

    ultimo = anterior - sizeof(ITEM);

    if (lixo == 0)
    {
        return;
    }

    fseek(arq, ultimo, SEEK_SET);
    fread(&i, sizeof(ITEM), 1, arq);
    fseek(arq, lixo, SEEK_SET);
    fwrite(&i, sizeof(ITEM), 1, arq);
    fseek(arq, ultimo, SEEK_SET);
    fwrite(&zero, sizeof(ITEM), 1, arq);

    goToCluster(arq, currentFolderCluster);
}

void RM(FILE *arq, char *name) // recebe um ponteiro para o inicio do folder que contem o item que será removido e o nome deste item
{

    char *currentPathAux = currentPath;
    int currentClusterTemp = currentFolderCluster;

    char *fileName = goToFolderAndReturnLastNameOfPath(arq, name, 0);
    char branco = '\0';
    ITEM *folderItems = readFolderContent(arq);
    ITEM *folderItensGO = malloc(sizeof(ITEM));
    int x = folderItems[0].ler_bit;
    int i = 0;

    while (x)
    {
        if (!strcmp(folderItems[i].nome, fileName))
        {
            if (folderItems[i].eh_pasta_bit)
            {
                goToCluster(arq, folderItems[i].ponteiro);
                fseek(arq, sizeof(clmeta), SEEK_CUR);
                fread(folderItensGO, sizeof(ITEM), 1, arq);

                if (folderItensGO->ler_bit)
                {
                    printf("Nao se pode excluir uma pasta com arquivo dentro");
                    return;
                }
            }

            writeFat(arq, folderItems[i].ponteiro, '\0');
            goToCluster(arq, folderItems[i].ponteiro);
            apagaCluster(arq);
            goToCluster(arq, currentClusterTemp);
            removeItemFromCurrentCluster(arq, folderItems[i].nome);

            break;
        }
        i += 1;
        x = folderItems[i].ler_bit;
    }

    strcpy(currentPath, currentPathAux);
    currentFolderCluster = currentClusterTemp;
    goToCluster(arq, currentClusterTemp);

    // TO-DO
    // - remover sequenciamento de acordo com sequenciamento da fat do arquivo divido em clusters

    return;
}

void MKDIR(FILE *arq, char *name)
{
    goToCluster(arq, currentFolderCluster);
    ITEM *temp = malloc(sizeof(ITEM));
    temp->ler_bit = true;
    temp->eh_pasta_bit = true;
    strcpy(temp->nome, name);
    writeItemInCurrFolder(arq, temp);
}

void MKFILE(FILE *arq, char *name)
{
    goToCluster(arq, currentFolderCluster);
    ITEM *temp = malloc(sizeof(ITEM));
    temp->ler_bit = true;
    temp->eh_pasta_bit = false;
    strcpy(temp->nome, name);
    writeItemInCurrFolder(arq, temp);
}

void EDIT(FILE *arq, char fileDir[maxCharsInFileSystem], char fileContent[maxCharsInFileSystem])
{

    char *currentPathAux = currentPath;
    int currentClusterTemp = currentFolderCluster;

    char *fileName = goToFolderAndReturnLastNameOfPath(arq, fileDir, 0);

    ITEM *folderItems = readFolderContent(arq);
    int x = folderItems[0].ler_bit;
    int i = 0;

    while (x)
    {
        if (!strcmp(folderItems[i].nome, fileName))
        {
            if (folderItems[i].eh_pasta_bit)
            {
                printf("cannot edit a folder!\n");
                return;
            }

            goToCluster(arq, folderItems[i].ponteiro);
            apagaCluster(arq); // Limpa o cluster antigo
            fwrite(fileContent, clusterSize / 8, 1, arq);
            break;
        }
        i += 1;
        x = folderItems[i].ler_bit;
    }

    strcpy(currentPath, currentPathAux);
    currentFolderCluster = currentClusterTemp;
    goToCluster(arq, currentClusterTemp);

    // RIP
    // TO-DO
    //-while para quebrar arquivo em n clusters
    //--procurar cluster disponivel fat
    //--write
    //--gravar sequenciamento fat
}

void RENAME(FILE *arq, char *fileDir, char *fileNewName)
{

    char *currentPathAux = currentPath;
    int currentClusterTemp = currentFolderCluster;

    char *fileName = goToFolderAndReturnLastNameOfPath(arq, fileDir, 0);
    int folderClusterOfFile = currentFolderCluster;
    printf("fileName: %s\n", fileName);
    ITEM *folderItems = readFolderContent(arq);
    int x = folderItems[0].ler_bit;
    int i = 0;

    while (x)
    {
        if (!strcmp(folderItems[i].nome, fileName))
        {
            strcpy(folderItems[i].nome, fileNewName);
            printf("fileName: %s\n", folderItems[i].nome);
            //goToCluster(arq, folderItems[i].ponteiro);
            goToCluster(arq, folderClusterOfFile);
            fseek(arq, sizeof(clmeta), SEEK_CUR);
            fseek(arq, sizeof(ITEM) * i, SEEK_CUR);
            fwrite(&folderItems[i], sizeof(ITEM), 1, arq);
            break;
        }
        i += 1;
        x = folderItems[i].ler_bit;
    }

    strcpy(currentPath, currentPathAux);
    currentFolderCluster = currentClusterTemp;
    goToCluster(arq, currentClusterTemp);
}

void MOVE(FILE *arq, char path_origem[maxCharsInFileSystem], char path_destino[maxCharsInFileSystem])
{
    ITEM *parentFolderItems = readFolderContent(arq); // Pega os itens do parente
    int i = 0;                                        // Acha o indice do item do arquivo que deve ser removido
    //int clusterAtual = currentFolderCluster;
    ITEM item;
    char *name;
    clmeta meta;

    name = goToFolderAndReturnLastNameOfPath(arq, path_origem, 0);

    // search na pasta e acha o item
    while (!strcmp(parentFolderItems[i].nome, name))
    {
        i++;
    }

    // read cluster que esse item aponta e o tipo
    //clusterAtual = (ftell(arq) - rootPos) / clusterSize;
    goToCluster(arq, currentFolderCluster);
    fread(&meta, sizeof(clmeta), 1, arq);
    fread(&item, sizeof(ITEM), 1, arq);
    removeItemFromCurrentCluster(arq, name);

    // strcpy?
    name = goToFolderAndReturnLastNameOfPath(arq, path_destino, 0);

    CD(arq, name);

    i = 0;

    while (!strcmp(parentFolderItems[i].nome, name))
    {
        i++;
    }

    //clusterAtual = (ftell(arq) - rootPos) / clusterSize;

    writeItemInCurrFolder(arq, &item);
}

int interfaceHandleCommand(FILE *arq, char *command)
{

    if (!strcmp(command, ""))
        return 1;

    const char breakChar = ' ';
    char *commandName = strtok(command, &breakChar);
    if (!strcmp(commandName, "CD"))
    {
        char *folderDir = strtok(NULL, &breakChar);
        if (folderDir == NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }
        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }
        CD(arq, folderDir);
        return 1;
    }

    if (!strcmp(commandName, "DIR"))
    {
        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("No parameters expected for this command\n");
            return 1;
        }

        DIR(arq);

        return 1;
    }

    if (!strcmp(commandName, "RM"))
    {
        char *dirName = strtok(NULL, &breakChar);
        if (dirName == NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }
        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }

        RM(arq, dirName);
        return 1;
    }

    if (!strcmp(commandName, "MKDIR"))
    {
        char *dirName = strtok(NULL, &breakChar);
        if (dirName == NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }
        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }
        MKDIR(arq, dirName);
        return 1;
    }

    if (!strcmp(commandName, "MKFILE"))
    {
        char *fileName = strtok(NULL, &breakChar);
        if (fileName == NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }
        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return 1;
        }
        MKFILE(arq, fileName);
        return 1;
    }

    if (!strcmp(commandName, "EDIT"))
    {

        char *fileName = strtok(NULL, &breakChar);
        if (fileName == NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }

        char *fileDir = strtok(NULL, &breakChar);
        if (fileDir == NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }

        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }
        EDIT(arq, fileName, fileDir);
        return 1;
    }

    if (!strcmp(commandName, "MOVE"))
    {
        char *fileDirSource = strtok(NULL, &breakChar);
        if (fileDirSource == NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }

        char *fileDirDestiny = strtok(NULL, &breakChar);
        if (fileDirDestiny == NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }

        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }
        MOVE(arq, fileDirSource, fileDirDestiny);
        return 1;
    }

    if (!strcmp(commandName, "RENAME"))
    {
        char *fileDir = strtok(NULL, &breakChar);
        if (fileDir == NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }

        char *fileNewName = strtok(NULL, &breakChar);
        if (fileNewName == NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }

        char *invalidParameter = strtok(NULL, &breakChar);
        if (invalidParameter != NULL)
        {
            printf("Two parameters expected for this command\n");
            return 1;
        }
        RENAME(arq, fileDir, fileNewName);
        return 1;
    }

    if (!strcmp(commandName, "EXIT"))
    {
        return 0;
    }
    // to pay respect
    if (!strcmp(commandName, "F"))
    {
        printf("...........................................................\n.....................-:+++++::-............................\n.............*@WWWWWWWWWWWWWWWWWWWW#-......................\n..........-@WWWWW@*-............-=@WW@=-...................\n.........-@WWW@:.......-:::::-......-#W@=..................\n.........@WWW#.........*WWWWW+.........=@#.................\n........+WWW@..........*WWWWW+..........*@#................\n........=WWW+..........*WWWWW+...........@W+...............\n........#WW@.....=WWWWWWWWWWWWWWWWW+.....=W@...............\n........#WW+.....=WWWWWWSUSWWWWWWWW+.....+WW+..............\n........=WW-.....=WWWWWWWWWWWWWWWWW+.....+WW#..............\n........=W#......------=WWWWW*-----......:WW=..............\n........=@*............*WWWWW+...........:WW*..............\n........*@+............*WWWWW+...........*WW:..............\n........*@+............*WWWWW+...........@W@...............\n........*@+............*WWWWW+..........+WW=...............\n........*@=............*WWWWW+..........#WW+...............\n........=W=............*WWWWW+.........-WWW:...............\n........#W#............................:WW@................\n........=W@............................:WW=................\n........+WW:...........................*WW:................\n........:WW+...........................*WW-................\n.........-:-...........................*WW-................\n...........................................................\n");
        return 1;
    }
    printf("Unrecognized command\n");
    return 1;
}

void interfaceLoop(FILE *arq)
{
    int t = 1;
    while (t)
    {
        printf("%s\n", currentPath);
        printf("> ");
        char command[maxCharsInFileSystem];
        fgets(command, maxCharsInFileSystem, stdin);
        command[strcspn(command, "\n")] = 0;
        t = interfaceHandleCommand(arq, command);
    }
}

int main(int argc, char *argv[])
{

    FILE *arq;
    METADADOS dados = {pattern, clusterSize, indiceStart, rootPos};

    if (argv[1])
    { // resetar arquivo
        cria_arquivo(dados, arq);
        // caso queria um arquivo grandao :)
        //arq = fopen("fileSystem.bin", "r+");
        //goToCluster(arq, 257);
        //fwrite("2", sizeof("2"), 1, arq);
        //fclose(arq);
    }

    arq = fopen("fileSystem.bin", "r+");

    goToCluster(arq, 0);

    interfaceLoop(arq); // teste com interface

    fclose(arq); // teste com interface

    return 0; // teste com interface
}
