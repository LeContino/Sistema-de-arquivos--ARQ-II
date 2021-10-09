#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define padrao 256      //padrao representa o 2^8 que aparece diversas vezes no enunciado do problema
#define tam_cluster 32000 // 32kb
#define zero 0

int main(){

    FILE*fat;
    int tam_indice=padrao;
    int inicio_indice=0;
    int pos_cluster_um=0;       //posi��o do primeiro cluster
    int distancia=0;            //calcula o tamanho total dos indices , ou seja, (posi��o inicial + 255)*sizeof(int)
    int i,j;
    int fim_linha=0;

//cria��o do arquivo

    if(!(fat =fopen ("arquivo.txt","wb"))) //abertura do arquivo
        printf("Erro na abertura do arquivo \n");

    else
    {
        fprintf(fat,"%d",tam_indice);   //escreve tam do indice
        fprintf(fat,"%d",tam_cluster);  //escreve tam do cluster
        fseek(fat,4*sizeof(int),SEEK_SET); // pula tam_indice,tam_cluster,inicio_indice,inicio_root
        inicio_indice=ftell(fat);           //posi��o do inicio_indice
        fseek(fat,2*sizeof(int),SEEK_SET); // volta para inicio_indice para escrever a posi��o
        fprintf(fat,"%d",inicio_indice); //escreve onde inicia o indice
        distancia=(inicio_indice+padrao-1)*sizeof(int); //16+256-1 *8
        fseek(fat,distancia,SEEK_SET); //inicio_ind +256-1
        pos_cluster_um=ftell(fat);
        fseek(fat,3*sizeof(int),SEEK_SET); //retorna �para a posi��o de metalinguagem inicio_root
        fprintf(fat,"%d \n",pos_cluster_um);  //escreve posi��o do 1�cluster no arquivo ->inicio_root  e passa para a proxima linha
        fseek(fat,pos_cluster_um,SEEK_SET); //acha 1�cluter

        for (i=0;i<padrao;i++){     //qntidade de clusters
            for (j=0;j<tam_cluster;j++) // cada cluster
            {
                if(j%padrao==0) // segue tamanho dos indices
                {
                    fprintf(fat,"/n"); // passa para a proxima linha
                }
                fprintf(fat,"%d",zero); //printa o valor zero de todos os clusters para determinar o tamanho m�x do arquivo
            }
        }
    }
    fclose(fat); // fecha arquivo


}
