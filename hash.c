#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h> 
#define SEED    0x12345678
#define SEED2   0x87654321

typedef struct {
     uintptr_t * table;
     int size;
     int max;
     int tipo;
     float taxa;
     uintptr_t deleted;
     char * (*get_key)(void *);
}thash;

//umas variaveis 
thash hash_simples, hash_duplo;
tcep * dados[10000];
int num = 0;

//funcoes pro hash
uint32_t hashf(const char* str, uint32_t h){
    /* One-byte-at-a-time Murmur hash 
    Source: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp */
    for (; *str; ++str) {
        h ^= *str;
        h *= 0x5bd1e995;
        h ^= h >> 15;
    }
    return h;
}

uint32_t hash1(const char *key, int table_size){
    return hashf(key, SEED)%table_size;
}
uint32_t hash2(const char *key, int table_size){
    return(hashf(key, SEED2)%(table_size-1))+1;
}

int posicao(thash *h, const char *key, int i){
    int h1 = hash1(key, h->max);
    int h2 = hash2(key, h->max);
    if(h->tipo == 1)
        return (h1+i*h2)%h->max;
    else
        return (h1+i)%h->max;
}

//pra taxa
int hash_insere(thash * h, void * bucket);

int hash_nvtam (thash *h){
    int max_ant = h->max;
    uintptr_t *table_ant = h->table;

    h->max = max_ant*2;
    h->table = calloc(h->max, sizeof(void*));
    if(h->table==NULL){
        h->table = table_ant;
        h->max = max_ant;
        return EXIT_FAILURE;
    }
    
    h->size = 0;
    for (int i=0; i<max_ant;i++){
        if(table_ant[i]!=0 && table_ant[i]!= h->deleted){
            void *reg = (void *)table_ant[i];
            hash_insere(h,reg);
        }
    }

    free(table_ant);
    return EXIT_SUCCESS;
}

int hash_insere(thash * h, void * bucket){
    if ((float)(h->size+1)/h->max > h->taxa){
        if (hash_nvtam(h)!= EXIT_SUCCESS){
            free(bucket);
            return EXIT_FAILURE;
        }
    }
    char *key = h->get_key(bucket);
    for (int i = 0; i<h->max; i++){
        int pos = posicao(h, key, i);
        if (h->table[pos] == 0 || h->table[pos] == h->deleted) {
            h->table[pos] = (uintptr_t)bucket;
            h->size++;
            return EXIT_SUCCESS;
        }
    }
    free(bucket);
    return EXIT_FAILURE;
}


int hash_constroi(thash * h,int nbuckets, char * (*get_key)(void *), int tipo, float ataxa ){
    h->table =calloc(h->max = nbuckets+1, sizeof(void *));
    if (h->table == NULL){
        return EXIT_FAILURE;
    }
    h->max = nbuckets+1;
    h->size = 0;
    h->deleted = (uintptr_t)&(h->size);
    h->get_key = get_key;
    h->taxa = ataxa;
    h->tipo = tipo;
    return EXIT_SUCCESS;

}

//busca
void * hash_busca(thash *h, const char * key){
    for (int i = 0; i < h->max; i++) {
        int pos = posicao(h, key, i);
        if (h->table[pos] == 0) {
            return NULL;
        }
        if (h->table[pos] != h->deleted) {
            void *reg = (void *)h->table[pos];
            if (strcmp(h->get_key(reg), key) == 0) {
                return reg;
            }
        }
    }
    return NULL;
}
//igual
void hash_apaga(thash *h){
    int pos;
    for(pos =0;pos< h->max;pos++){
        if (h->table[pos] != 0){
            if (h->table[pos]!=h->deleted){
                free((void *)h->table[pos]);
            }
        }
    }
    free(h->table);
}

//regs
typedef struct{
    char cep[10]; 
    char cidade[100];
    char estado[3]; 
    char chave[6]; 
}tcep;

char * get_key(void * reg){
    return (*((tcep *)reg)).chave;
}


void *aloca_reg(char *cidade, char *cep, char *estado){
    tcep *reg = malloc(sizeof(tcep));
    strcpy(reg->cep, cep);
    strncpy(reg->chave, cep, 5);
    reg->chave[5] = '\0';
    strcpy(reg->cidade, cidade);
    strcpy(reg->estado, estado);
    return reg;
}
//arquivo
int carregando_ceps(const char *arquivo_csv, thash *h, int limite){
    FILE *fp = fopen(arquivo_csv, "r");
    if (fp==NULL){
        perror("erro");
        return EXIT_FAILURE;
    }

    char linha[256];
    int linha_atual = 0;
    int quant = 0;


    while (fgets(linha, sizeof(linha), fp)){
        if (linha_atual++ == 0){
            continue;
        }
        char *estado = strtok(linha, ";");
        char *cidade = strtok(NULL, ";");
        char *cep = strtok(NULL, ";\n");

        if (estado && cidade && cep){
            void *reg = aloca_reg(cidade, cep, estado);
            if (h){
                hash_insere(h, reg);
            }else{
                dados[num++]=(tcep *)reg;
                if (num>=10000){
                    break;
                }
            quant++;
            if (limite>0 && quant>=limite){
            break;
        }
        }
        }
    }
    fclose(fp);
    return EXIT_SUCCESS;
}

//funcoes pra teste p ver no relatorio
double tempo(){
    return (double)clock()/CLOCKS_PER_SEC;
}

void insere1000(const char *filename) {
    thash h;
    hash_constroi(&h, 1000, get_key, 0, 0.75);
    carregando_ceps(filename, &h, 0);
    hash_apaga(&h);
}

void insere6100(const char *filename) {
    thash h;
    hash_constroi(&h, 6100, get_key, 0, 0.75);
    carregando_ceps(filename, &h, 0);
    hash_apaga(&h);
}

void insercao(const char *filename) {
    double inicio, tempo1000, tempo6100;

    inicio = tempo();
    insere1000(filename);
    tempo1000 = tempo() - inicio;

    inicio = tempo();
    insere6100(filename);
    tempo6100 = tempo() - inicio;

    printf("tempo de 1000 buckets: %.6f s\n", tempo1000);
    printf("tempo de 6100 buckets: %.6f s\n", tempo6100);
    printf("overhead: %.2f%%\n", ((tempo1000 - tempo6100) / tempo6100) * 100.0);
}

void busca10_0() {
    for (int i = 0; i < num / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca20_0() {
    for (int i = 0; i < num / 5; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca30_0() {
    for (int i = 0; i < (num * 3) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca40_0() {
    for (int i = 0; i < (num * 4) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca50_0() {
    for (int i = 0; i < num / 2; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca60_0() {
    for (int i = 0; i < (num * 6) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca70_0() {
    for (int i = 0; i < (num * 7) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca80_0() {
    for (int i = 0; i < (num * 8) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca90_0() {
    for (int i = 0; i < (num * 9) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca99_0() {
    for (int i = 0; i < (num * 99) / 100; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca10_1() {
    for (int i = 0; i < num / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca20_1() {
    for (int i = 0; i < num / 5; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca30_1() {
    for (int i = 0; i < (num * 3) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca40_1() {
    for (int i = 0; i < (num * 4) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca50_1() {
    for (int i = 0; i < num / 2; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca60_1() {
    for (int i = 0; i < (num * 6) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca70_1() {
    for (int i = 0; i < (num * 7) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca80_1() {
    for (int i = 0; i < (num * 8) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca90_1() {
    for (int i = 0; i < (num * 9) / 10; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void busca99_1() {
    for (int i = 0; i < (num * 99) / 100; i++) {
        hash_busca(&hash_simples, dados[i]->chave);
    }
}

void ocupacao(){
    hash_constroi(&hash_simples, 6100, get_key, 0, 0.99);
    for (int i = 0; i < num; i++){
        tcep * copia = (tcep *)aloca_reg(dados[i]->cidade, dados[i]->cep, dados[i]->estado);
        hash_insere(&hash_simples, copia);
    }

    hash_constroi(&hash_duplo, 6100, get_key, 1, 0.99);
    for (int i = 0; i < num; i++){
        tcep * copia = (tcep *)aloca_reg(dados[i]->cidade, dados[i]->cep, dados[i]->estado);
        hash_insere(&hash_duplo, copia);
    }
}

void apaga(){
    hash_apaga(&hash_simples);
    hash_apaga(&hash_duplo);
}

void testes(){
    busca10_0(); busca10_1();
    busca20_0(); busca20_1();
    busca30_0(); busca30_1();
    busca40_0(); busca40_1();
    busca50_0(); busca50_1();
    busca60_0(); busca60_1();
    busca70_0(); busca70_1();
    busca80_0(); busca80_1();
    busca90_0(); busca90_1();
    busca99_0(); busca99_1();
}

//main e busca do cep
int main(int argc, char* argv[]){
    if (argc < 2) {
        printf("Uso: %s <arquivo_csv>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *arquivo_csv = argv[1];
    const char *busca;
    if (argc > 2) {
        busca = argv[2];
    } else {
        busca = "69928"; //botar aqui os 5 primeiros numeros de qual cep quer achar
    }

    if (carregando_ceps(arquivo_csv, NULL, 0) != EXIT_SUCCESS) {
        printf("erro na hora de carregar os dados do arquivo!\n");
        return EXIT_FAILURE;
    }

    printf("total de registros: %d \n\n", num);

    thash h_temp;
    if (hash_constroi(&h_temp, 6100, get_key, 1, 0.75) != EXIT_SUCCESS) {
        printf("erro na hora de construir tabela hash\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num; i++) {
        tcep * copia = (tcep *)aloca_reg(dados[i]->cidade, dados[i]->cep, dados[i]->estado);
        hash_insere(&h_temp, copia);
    }

    printf("buscando por: %s\n", busca);

    char cep_chave[6];
    strncpy(cep_chave, busca, 5);
    cep_chave[5] = '\0';
    
    tcep *resultado = (tcep *)hash_busca(&h_temp, cep_chave);
    if (resultado) {
        printf("registro encontrado: \n");
        printf("CEP: %s000 | Cidade: %s | Estado: %s\n", busca, resultado->cidade, resultado->estado);
    } else {
        printf("CEP %s não encontrado.\n", busca);
    }
    
    hash_apaga(&h_temp);

    ocupacao();
    testes();
    insercao(arquivo_csv);
    apaga();
    
    for (int i = 0; i < num; i++) {
        free(dados[i]);
    }
}