#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"

int b_size = 512; // tamanho de cada bloco em bytes
int b_num = 256; // numero total de blocos no disco
int b_indic = 128; // numero de blocos de indice

int nReads = 0; // numero de leituras
int nWrites = 0; // numero de escritas

// altera o tamanho de bloco
void set_block_size(int block_size) {
  // se for um parametro valido
  if(block_size > 0) {
    // novo tamanho de bloco
    b_size = block_size;
  }
}
// retorna o tamanho do bloco
int get_block_size() {
  return b_size;
}
// altera o numero total de blocos no disco
void set_block_num(int block_num) {
  // se for um parametro valido
  if(block_num > 0) {
    // novo numero de blocos
    b_num = block_num;
  }
}
// retorna o numero de blocos de indice
int get_block_num() {
  return b_num;
}

// leitura de um bloco qualquer para o buffer
void fd_read_raw(int block_num, char * buffer) {
  // se o numero do bloco estiver dentro do intervalo do disco
  if(block_num >= 0 && block_num < b_num) {
    // ponteiro auxiliar
    void *aux = NULL;
    char *c = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // desloca o ponteiro auxiliar para o bloco desejado
    aux = aux + (block_num * b_size);
    c = aux;
    // passa conteudo do bloco para o buffer
    for(int i = 0; i < b_size; ++i, ++c)
      buffer[i] = *c;
    // incrementa numero de leituras
    ++nReads;
  }
}
// escrita de um bloco qualquer do buffer
void fd_write_raw(int block_num, char * buffer) {
  // se o numero do bloco estiver dentro do intervalo do disco
  if(block_num >= 0 && block_num < b_num) {
    // ponteiro auxiliar
    void *aux = NULL;
    char *c = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // desloca o ponteiro auxiliar para o bloco desejado
    aux = aux + (block_num * b_size);
    c = aux;
    // passa conteudo do buffer para o bloco
    for(int i = 0; i < b_size; ++i, ++c)
      *c = buffer[i];
    // incrementa numero de escritas
    ++nWrites;
  }
}
// leitura de superbloco
void fd_read_super_block(int block_num, struct super_block * buffer) {
  // se esta no bloco correto
  if(block_num == 0) {
    // ponteiro auxiliar
    struct super_block *aux = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // passa conteudo do bloco para o buffer
    buffer->size = aux->size;
    buffer->iSize = aux->iSize;
    buffer->tamLivre = aux->tamLivre;
    // incrementa numero de leituras
    ++nReads;
  }
}
// escrita de superbloco
void fd_write_super_block(int block_num, struct super_block * buffer) {
  // se esta no bloco correto
  if(block_num == 0) {
    // ponteiro auxiliar
    struct super_block *aux = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // passa conteudo do buffer para o bloco
    aux->size = buffer->size;
    aux->iSize = buffer->iSize;
    aux->tamLivre = buffer->tamLivre;
    // incrementa numero de escritas
    ++nWrites;
  }
}
// leitura de bloco de indice
void fd_read_i_node_block(int block_num, struct i_node_block * buffer) {
  // se esta na faixa de blocos de indice
  if(block_num > 0 && block_num < b_indic) {
    // ponteiro auxiliar
    void *aux = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // desloca o ponteiro auxiliar para o bloco desejado
    aux = aux + (block_num * b_size);
    // ponteiro auxiliar para estrutura
    struct i_node_block *v = NULL;
    // posiciona o ponteiro auxiliar da estrutura para o bloco desejado
    v = aux;
    // passa conteudo do bloco para o buffer
    buffer->p = v->p;
    // incrementa numero de leituras
    ++nReads;
  }
}
// escrita de bloco de indice
void fd_write_i_node_block(int block_num, struct i_node_block * buffer) {
  // se esta na faixa de blocos de indice
  if(block_num > 0 && block_num < b_indic) {
    // ponteiro auxiliar
    void *aux = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // desloca o ponteiro auxiliar para o bloco desejado
    aux = aux + (block_num * b_size);
    // ponteiro auxiliar para estrutura
    struct i_node_block *v = NULL;
    // posiciona o ponteiro auxiliar da estrutura para o bloco desejado
    v = aux;
    // passa conteudo do buffer para o bloco
    v->p = buffer->p;
    // incrementa numero de escritas
    ++nWrites;
  }
}
// leitura de bloco indireto
void fd_read_indirect_block(int block_num, struct indirect_block * buffer) {
  // se esta na faixa de blocos de dados
  if(block_num >= b_indic && block_num < b_num) {
    // ponteiro auxiliar
    void *aux = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // desloca o ponteiro auxiliar para o bloco desejado
    aux = aux + (block_num * b_size);
    // ponteiro auxiliar para estrutura
    struct indirect_block *v = NULL;
    // posiciona o ponteiro auxiliar da estrutura para o bloco desejado
    v = aux;
    // passa conteudo do bloco para o buffer
  	buffer->p = v->p;
    // incrementa numero de leituras
    ++nReads;
  }
}
// escrita de bloco indireto
void fd_write_indirect_block(int block_num, struct indirect_block * buffer) {
  // se esta na faixa de blocos de dados
  if(block_num >= b_indic && block_num < b_num) {
    // ponteiro auxiliar
    void *aux = NULL;
    // posiciona ponteiro auxiliar no inicio
    aux = disco;
    // desloca o ponteiro auxiliar para o bloco desejado
    aux = aux + (block_num * b_size);
    // ponteiro auxiliar para estrutura
    struct indirect_block *v = NULL;
    // posiciona o ponteiro auxiliar da estrutura para o bloco desejado
    v = aux;
    // passa conteudo do buffer para o bloco
  	v->p = buffer->p;
    // incrementa numero de escritas
    ++nWrites;
  }
}
// encerra as operacoes de leitura e de escrita
int fd_stop() {
  // retorna estatisticas de leituras e de escritas
  return nReads + nWrites;
}
