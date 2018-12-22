#ifndef _FAKEFS_H_
#define _FAKEFS_H_

#define FFS_SEEK_SET 0
#define FFS_SEEK_CUR 1
#define FFS_SEEK_END 2

//Estrutura da tabela de arquivos abertos
struct tabelaArq {
  char *bitmap; //Ponteiro para alocar um array de bytes que possui 0s e 1s
  struct fd *fdArray;
};

//Estrutura de descritor de arquivo
struct fd {
  struct i_node i_node; //inode do arquivo
  int inumber; //inumber do arquivo
  int seekptr; //ponteiro de busca atual
};

struct tabelaArq *tabArq;

void INIT();

int ffs_format_disk(int size, int i_size);

int ffs_shutdown();

int ffs_create();

int ffs_open(int i_number);

int ffs_i_number(int fd);

int ffs_read(int fd, char * buffer, int size);

int ffs_write(int fd, char * buffer, int size);

int ffs_seek(int fd, int offset, int whence);

int ffs_close(int fd);

int ffs_delete(int i_number);


#endif /* _FAKEFS_H_ */
