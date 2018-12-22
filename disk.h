#ifndef _FAKEDISK_H_
#define _FAKEDISK_H_

#define I_NODE_SIZE 64

void * disco;

struct i_node{
	int size; //default 64 bytes
	int flags;
	int owner;
	int file_size;
	int *p; // ponteiro para um Array de ponteiros de tamanho 10

};

struct i_node_block{
	//Array de inodes
	//int block_size; //definido no disk global como 512 bytes
	//int inode_size; //recebe valor do size da struct i_node
	struct i_node *p; //ponteiro para array de inodes
};

struct indirect_block{
	//Precisa ter ponteiros para os proximos
	//BLOCK_SIZE/4 blocos diretos do arquivo
	// int block_size;  //default 512
	int *p; //ponteiro para blocos indiretos ou blocos de dados
};

struct super_block{
	//Numero de blocos no sistema
	int size;
	//Numero de blocos de indice
	int iSize;
	//Primeiro bloco na lista de livres
	// int freeList;
	//Tamanho da lista de livres
	int tamLivre;
};


/* Funcoes para mudar e obter o tamanho e numero de blocos simulados */
void set_block_size(int block_size); //default 512
int get_block_size();

void set_block_num(int block_num); //default 256
int get_block_num();

/* Como definido no livro */
void fd_read_raw(int block_num, char * buffer);

void fd_write_raw(int block_num, char * buffer);

void fd_read_super_block(int block_num, struct super_block * buffer);

void fd_write_super_block(int block_num, struct super_block * buffer);

void fd_read_i_node_block(int block_num, struct i_node_block * buffer);

void fd_write_i_node_block(int block_num, struct i_node_block * buffer);

void fd_read_indirect_block(int block_num, struct indirect_block * buffer);

void fd_write_indirect_block(int block_num, struct indirect_block * buffer);

int fd_stop();

#endif /* _FAKEDISK_H_ */
