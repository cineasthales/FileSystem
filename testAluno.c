#include "disk.h"
#include "fs.h"
#include "simplegrade.h"
#include <stdio.h>
#include <string.h>

#define CLEANUP(a, x, size)\
	for(int k=0; k<size; ++k) a[k] = x;

void test_1(){

	DESCRIBE("Testes no disco");

	IF("Aumento dimensoes do disco");
	THEN("Disco tem as novas dimensoes");

	set_block_num(256);
	isEqual(get_block_num(), 256, 1);
	set_block_size(4096);
	isEqual(get_block_size(), 4096, 1);

	IF("Diminuo dimensoes do disco");
	THEN("Disco tem as novas dimensoes");

	set_block_num(32);
	isEqual(get_block_num(), 32, 1);
	set_block_size(512);
	isEqual(get_block_size(), 512, 1);

	char buffer[512], result[512];
	int i;
	CLEANUP(buffer, 'X', 32);
	CLEANUP(result, ' ', 32);

	IF("Escrevo em um bloco");
	THEN("Le o mesmo valor");
	fd_write_raw(10, buffer);
	fd_read_raw(10, result);
	for(i=0; (i<512)&&(result[i]==buffer[i]); ++i);
	isEqual(i, 512, 1);

	IF("Escrevo em outro bloco");
	THEN("Le o mesmo valor");
	CLEANUP(buffer, 'Y', 512);
	CLEANUP(result, ' ', 512);
	fd_write_raw(31, buffer);
	fd_read_raw(31, result);
	for(i=0; (i<512)&&(result[i]==buffer[i]); ++i);
	isEqual(i, 512, 1);

	IF("Tento escrever em um bloco invalido");
	THEN("Nao deve escrever nem ler");
	CLEANUP(buffer, 'Z', 512);
	CLEANUP(result, ' ', 512);
	fd_write_raw(32, buffer);
	fd_read_raw(32, result);
	isNotEqual(result[0], 'Z', 1);

	fd_stop();
}

void test_2(){
	DESCRIBE("Testes de formatacao");

	IF("Disco nao tem blocos");
	THEN("Nao deve formatar");
	isEqual(ffs_format_disk(0,32), 0, 1);

	IF("Disco nao tem blocos de inode");
	THEN("Nao deve formatar");
	isEqual(ffs_format_disk(32,0), 0, 1);

	IF("Disco nao tem o minimo de blocos");
	THEN("Nao deve formatar");
	isEqual(ffs_format_disk(16,8), 0, 1);

	IF("Parametros estao corretos");
	THEN("Deve formatar");
	isEqual(ffs_format_disk(32,1), 1, 1);
	isEqual(ffs_format_disk(32,2), 1, 1);
	isEqual(ffs_format_disk(32,4), 1, 1);
	isEqual(ffs_format_disk(32,8), 1, 1);
	isEqual(ffs_format_disk(32,16), 1, 1);

	fd_stop();
}


void test_3(){
	char buffer[512], res[512];
	int i;
	CLEANUP(buffer, 'P', 512);
	CLEANUP(res, ' ', 512);

	DESCRIBE("Testes com arquivo");
	set_block_size(512);
	ffs_format_disk(32,1);
	int i_num = ffs_create();
	int fd = ffs_open(i_num);
	// 1 para superbloco
	// 1 para inodes
	// 1 para lista de livres
	// 1 indireto
	// sobram 28
	IF("Quero escrever todos os blocos livres");
	THEN("Deve escrever tudo e ler os mesmos valores");
	for(i=0; i<28; ++i){
		ffs_write(fd, buffer, 512);
	}
	ffs_seek(fd, 0, 0);
	int ok = 1;
	for(i=0; i<28; ++i){
		ffs_read(fd, res, 512);
		for(int j=0; (j<512)&&ok; j++){
			if (buffer[j]!=res[j]){
				ok = 0;
			}
		}
	}
	isNotEqual(ok,0,1);
	ffs_close(fd);

	IF("Quero abrir um arquivo que nao existe");
	THEN("Nao deve abrir");
	isEqual(ffs_open(5),-2,1);

	IF("O inumber passado for maior do que o permitido");
	THEN("Deve cancelar a operacao");
	isEqual(ffs_open(100),-1,1);

	IF("O descritor passado nao for de um arquivo aberto");
	THEN("Deve cancelar a operacao");
	isEqual(ffs_i_number(20),-1,1);

	IF("Quero ler do arquivo fechado");
	THEN("Nao deve ler");
	isEqual(ffs_read(fd, res, 512),-1,1);

	IF("Quero fechar o arquivo novamente");
	THEN("Deve cancelar a operacao");
	isEqual(ffs_close(fd),0,1);

	IF("Quero reabrir e ler o arquivo que existe");
	THEN("Os dados sao lidos");
	fd = ffs_open(i_num);
	CLEANUP(res,' ', 512);
	ffs_read(fd, res, 512);
	ok = 1;
	for(int j=0; (j<512)&&ok; j++){
		if (buffer[j]!=res[j]){
			ok = 0;
		}
	}
	isNotEqual(ok,0,1);

	IF("Quero apagar o arquivo aberto");
	THEN("Deve impedir a operacao");
	isEqual(ffs_delete(i_num),-1,1);

	ffs_close(fd);
	IF("Quero apagar o arquivo depois de fechado");
	THEN("Os dados sao apagados");
	ffs_delete(i_num);
	fd = ffs_open(i_num);
	CLEANUP(res,' ', 512);
	ffs_read(fd, res, 512);
	ok = 1;
	for(int j=0; (j<512)&&ok; j++){
		if (buffer[j]==res[j]){
			ok = 0;
		}
	}
	isEqual(ok,1,1);
	ffs_close(fd);

	IF("Desligo o disco");
	THEN("Retorna numero de leituras e de escritas");
	isEqual(ffs_shutdown(),351,1);
}

int main(){

  INIT();

	test_1();
	test_2();
	test_3();


	GRADEME();

	if (grade==maxgrade)
		return 0;
	else return grade;

}
