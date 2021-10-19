#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define padrao 255                                               //padrao representa o 2^8 que aparece diversas vezes no enunciado do problema -> 0 a 255 =256
#define tamanho_cluster 32768
#define inicio_ind 8                                             // 1 byte tam do indice, 2 bytes tam do cluster, 2 bytes inicio do indice, 2 bytes inicio root  -> prox byte é posicao do indice
#define pos_root 263                                             // 7 bytes posteriores +256 bytes de indice


typedef struct folder_pattern{

    bool ler_bit:1; //indica se há algo para ler
    bool eh_pasta_bit:1; // indica se é pasta ou arquivo
    short ponteiro;
    char nome[16];

}FOLDER;

typedef struct metadados{

    char tam_indice;                                            // char é implicitamente transformado em um numero  --> 8 bits
    unsigned short tam_cluster;                                 // 1kB= 1024B
    short inicio_indice;                                        //inicio do indice --> 16 bits
    short inicio_root;                                          //posição do primeiro cluster

}METADADOS;


void cria_arquivo(METADADOS dados, FILE* arq)
{
    int i,j;
    char val=0;

    //criação do arquivo

    if(!(arq =fopen ("arquivo.bin","wb")))                     //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
    {
        fwrite(&dados,sizeof(METADADOS),1,arq);                    // printa os valores de metadado que são: 256 (de 0 a 255) , 4,0, 32768


        for (i=0;i<padrao;i++){                                 //qntidade de clusters
            for (j=0;j<tamanho_cluster;j++)                         // cada cluster
            {
                fwrite(&val,sizeof(char),1,arq);                 //printa o valor zero de todos os clusters para determinar o tamanho máx do arquivo
            }
        }
    }
    fclose(arq); // fecha arquivo
}


void le_metadados(FILE* arq){
    int i,j;
    char val=0;
    METADADOS leitura;

    //criação do arquivo

    if(!(arq =fopen ("arquivo.bin","rb")))                     //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
        fread(&leitura,sizeof(METADADOS),1,arq);

    printf("%d %d %d %d \n",leitura.tam_indice,leitura.tam_cluster, leitura.inicio_indice,leitura.inicio_root); // le tam_indice como -1 pois está em complemento de 2 (=255)

    fclose(arq); // fecha arquivo
}


int main(){

    FILE* arq;
    METADADOS dados={padrao,tamanho_cluster,inicio_ind,pos_root};

   cria_arquivo(dados, arq);
   //le_metadados(arq);
}
