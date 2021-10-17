#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define padrao 256  //padrao representa o 2^8 que aparece diversas vezes no enunciado do problema
//#define tam_cluster 32000  // 32kb

int main(){

    FILE*fat;
    int val=1;
    int tam_indice=padrao;
    int inicio_indice=0;
    int pos_cluster_um=0;       //posição do primeiro cluster
    int distancia=0;            //calcula o tamanho total dos indices , ou seja, (posição inicial + 255)*sizeof(int)
    int i,j;
    int tam_cluster = 32000;

//criação do arquivo

    if(!(fat =fopen ("arquivo1.bin","wb"))) //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
    {
        fwrite(&tam_indice,sizeof(int),1,fat);   //escreve tam do indice
        fwrite(&tam_cluster,sizeof(int),1,fat);  //escreve tam do cluster
        fseek(fat,4*sizeof(int),SEEK_SET); // pula tam_indice,tam_cluster,inicio_indice,inicio_root
        inicio_indice=ftell(fat);           //posição do inicio_indice
        fseek(fat,2*sizeof(int),SEEK_SET); // volta para inicio_indice para escrever a posição
        fwrite(&inicio_indice,sizeof(int),1,fat); //escreve onde inicia o indice
        distancia=(inicio_indice+padrao-1)*sizeof(int); //16+256-1 *8
        fseek(fat,distancia,SEEK_SET); //inicio_ind +256-1
        pos_cluster_um=ftell(fat);
        fseek(fat,3*sizeof(int),SEEK_SET); //retorna ´para a posição de metalinguagem inicio_root
        fwrite(&pos_cluster_um,sizeof(int),1,fat);  //escreve posição do 1°cluster no arquivo ->inicio_root  e passa para a proxima linha
        fseek(fat,pos_cluster_um,SEEK_SET); //acha 1°cluter

        for (i=0;i<padrao;i++){     //qntidade de clusters
            for (j=0;j<tam_cluster;j++) // cada cluster
            {
                fwrite(&val,sizeof(int),1,fat); //printa o valor zero de todos os clusters para determinar o tamanho máx do arquivo
            }
        }
    }
    fclose(fat); // fecha arquivo

}
