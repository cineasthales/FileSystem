#include "disk.h"
#include "fs.h"
#include "simplegrade.h"
#include <stdio.h>
#include <string.h>

#define CLEANUP(a, x, size)\
	for(int k=0; k<size; k++) a[k] = x;

void test_disk(){
	char buffer[2048], res[2048];
	int i;
	for(i=0; i<2048; i++){
		buffer[i]='X';
		res[i]=' ';
	}

	DESCRIBE("Testes de inicializacao e parametros do disco");

	IF("Modifico o numero de blocos");
	THEN("Ler o numero de blocos deve retornar o mesmo resultado");

	set_block_num(32);
	isEqual(get_block_num(), 32, 1);

	IF("Modifico o tamanho dos blocos");
	THEN("Ler o tamanho dos blocos deve retornar o mesmo resultado");

	set_block_size(2048);
	isEqual(get_block_size(), 2048, 1);

	IF("Escrevo em um bloco qualquer coisa");
	THEN("A leitura do bloco deve retornar o valor escrito");
	fd_write_raw(0, buffer);
	fd_read_raw(0, res);
	for(i=0; (i<2048)&&(res[i]==buffer[i]); i++);
	isEqual(i, 2048, 1);

	IF("Escrevo em um bloco inexistente");
	THEN("A leitura do bloco nÃ£o deve retornar o valor escrito");
	CLEANUP(buffer, ' ', 2048);
	CLEANUP(res, ' ', 2048);
	fd_write_raw(get_block_num(), buffer);
	fd_read_raw(get_block_num(), res);
	isNotEqual(res[0], 'X', 1);
	fd_stop();

}

void test_format(){
	DESCRIBE("Testes de formatacao do disco");
	set_block_num(512);

	IF("Parametros invalidos");
	THEN("Deve falhar");
	isEqual(ffs_format_disk(512,0), 0, 1);
	isEqual(ffs_format_disk(0,20), 0, 1);
	isEqual(ffs_format_disk(512,512), 0, 1);

	IF("Parametros validos");
	THEN("Deve formatar");
	isEqual(ffs_format_disk(512,64), 1, 1);
	isEqual(ffs_format_disk(512,16), 1, 1);
	isEqual(ffs_format_disk(512,32), 1, 1);
	fd_stop();
}


void test_um_arquivo(){
	char buffer[512], res[512];
	int i;
	CLEANUP(buffer, 'K', 512);
	CLEANUP(res, ' ', 512);

	DESCRIBE("Teste com um arquivo");
	set_block_size(512);
	ffs_format_disk(64,1); // 64 blocos de 512 B = 32 KiB
	// 1 bloco para superbloco
	// 1 bloco de inodes
	// sobram 62
	int i_num  = ffs_create();
	int fd =  ffs_open(i_num);

	IF("Escrevo 60 blocos");
	THEN("Deve escrever");
	for(i=0; i<60; i++){
		ffs_write(fd, buffer, 512);
	}
	THEN("Deve ler os mesmos valores");
	ffs_seek(fd, 0, 0);
	int ok = 1;
	for(i=0; i<60; i++){
		ffs_read(fd, res, 512);
		for(int j=0; (j<512)&&ok; j++){
			if (buffer[j]!=res[j]){
				ok = 0;
			}
		}
	}
	isNotEqual(ok,0,5);
	ffs_close(fd);
	THEN("Se reabrir, os valores devem continuar la");
	fd = ffs_open(i_num);
	CLEANUP(res,' ', 512);
	ffs_read(fd, res, 512);
	ok = 1;
	for(int j=0; (j<512)&&ok; j++){
		if (buffer[j]!=res[j]){
			ok = 0;
		}
	}
	isNotEqual(ok,0,5);
	ffs_close(fd);
	ffs_delete(i_num);
	THEN("Se apagarmos, os valores nao devem continuar la");
	fd = ffs_open(i_num);
	CLEANUP(res,' ', 512);
	ffs_read(fd, res, 512);
	ok = 1;
	for(int j=0; (j<512)&&ok; j++){
		if (buffer[j]==res[j]){
			ok = 0;
		}
	}
	isEqual(ok,1,5);
	ffs_close(fd);

	THEN("Deve ter espaco para criar um novo arquivo");

	fd =  ffs_open(i_num);

	IF("Escrevo 60 blocos");
	THEN("Deve escrever");
	for(i=0; i<60; i++){
		ffs_write(fd, buffer, 512);
	}
	THEN("Deve ler os mesmos valores");
	ffs_seek(fd, 0, 0);
	ok = 1;
	for(i=0; i<60; i++){
		ffs_read(fd, res, 512);
		for(int j=0; (j<512)&&ok; j++){
			if (buffer[j]!=res[j]){
				ok = 0;
			}
		}
	}
	fd_stop();
}

void test_varios_arquivos(){
	char buffer[512], res[512];
	int i;
	CLEANUP(buffer, 'K', 512);
	CLEANUP(res, ' ', 512);

	DESCRIBE("Teste com cinco arquivos");
	set_block_size(512);
	ffs_format_disk(64,1); // 64 blocos de 512 B = 32 KiB

	int i_num  = ffs_create();
	int fd[5];
	for(i=0; i<5; i++){
		fd[i] = ffs_open(i_num);
	}

	IF("Escrevo 5 blocos para cada arquivo");
	THEN("Deve escrever");
	for (int desc=0; desc<5; desc++){
		for(i=0; i<5; i++){
			ffs_write(fd[desc], buffer, 512);
		}
	}
	THEN("Deve ler os mesmos valores (lendo apenas do ultimo)");
	ffs_seek(fd[4], 0, 0);
	int ok = 1;
	for(i=0; i<5; i++){
		ffs_read(fd[4], res, 512);
		for(int j=0; (j<512)&&ok; j++){
			if (buffer[j]!=res[j]){
				ok = 0;
			}
		}
	}
	isNotEqual(ok,0,10);
	ffs_close(fd[4]);
	THEN("Se reabrir, os valores devem continuar la");
	fd[4] = ffs_open(i_num);
	CLEANUP(res,' ', 512);
	ffs_read(fd[4], res, 512);
	ok = 1;
	for(int j=0; (j<512)&&ok; j++){
		if (buffer[j]!=res[j]){
			ok = 0;
		}
	}
	isNotEqual(ok,0,5);
	ffs_close(fd[4]);
	fd_stop();
}

int main(){

  INIT();

	test_disk();
	test_format();
	test_um_arquivo();
	test_varios_arquivos();


	GRADEME();

	if (grade==maxgrade)
		return 0;
	else return grade;

}
