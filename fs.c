#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "disk.h"
#include "fs.h"

// Inicializacao do disco
void INIT() {
	disco = malloc(get_block_num() * get_block_size());
}

// Inicializa o disco para o estado vazio
// Preenche o superbloco
// Coloca todos os blocos na lista de blocos livres
int ffs_format_disk(int size, int i_size) {
  // se forem parametros validos
  if((size >= 32) && (i_size > 0) && (i_size < size)) {
    // define numero de blocos no disco
    set_block_num(size);
    // realoca disco na memoria
    disco = realloc(disco, get_block_num() * get_block_size());

		// numero de blocos para a lista de livres
    int tamLivres;
    if (get_block_num() % get_block_size() == 0)
      tamLivres = get_block_num()/get_block_size();
    else
      tamLivres = get_block_num()/get_block_size() + 1;
    // aloca lista de blocos livres
    char *bufferFreeList = NULL;
    bufferFreeList = malloc(get_block_size());
    // indica todos os blocos de dados como livres
    for(int i = 0; i < get_block_size(); ++i) {
      bufferFreeList[i] = '0';
    }
    // escreve lista de blocos livres no disco
    for(int i = 0; i < tamLivres; ++i) {
      fd_write_raw(i_size + i + 1, bufferFreeList);
    }
		free(bufferFreeList);

    // aloca superbloco
    struct super_block *bufferSuper = NULL;
    bufferSuper = malloc(sizeof(struct super_block));
    bufferSuper->size = size;
    bufferSuper->iSize = i_size;
    bufferSuper->tamLivre = tamLivres;
    // escreve superbloco no disco
    fd_write_super_block(0, bufferSuper);
		free(bufferSuper);

    // numero de inodes em um bloco de inodes
		int numInodes = (get_block_size() / I_NODE_SIZE);
		// percorre blocos de inode
    for(int i = 1; i <= i_size; ++i) {
			// aloca bloco de inode
      struct i_node_block *bufferInodeBlock = NULL;
      bufferInodeBlock = malloc(sizeof(struct i_node_block));
      bufferInodeBlock->p = malloc(numInodes * sizeof(struct i_node));
      // inicializa dados de cada inode de um bloco de inodes
      for (int j = 0; j < numInodes; ++j) {
        bufferInodeBlock->p[j].size = 64;
        bufferInodeBlock->p[j].flags = 0;
        bufferInodeBlock->p[j].owner = 0;
        bufferInodeBlock->p[j].file_size = 0;
        bufferInodeBlock->p[j].p = malloc(13 * sizeof(int));
        for(int k = 0; k < 13; k++)
          bufferInodeBlock->p[j].p[k] = -1;
      }
      fd_write_i_node_block(i, bufferInodeBlock);
			free(bufferInodeBlock);
    }

		// aloca na memoria a tabela de arquivos abertos
    tabArq = malloc(sizeof(struct tabelaArq));
    tabArq->bitmap = malloc(20);
    for (int i = 0; i < 20; ++i) {
      tabArq->bitmap[i] = '0';
    }
    tabArq->fdArray = malloc(20 * sizeof(struct fd));
    // confirma operacao
    return 1;
  }
  // valores invalidos
  return 0;
}

// Fecha todos os arquivos abertos
// Encerra o disco
int ffs_shutdown(){
  // libera vetor de descritores
  free(tabArq->fdArray);
  // libera bitmap
  free(tabArq->bitmap);
  // libera tabela de arquivos abertos
  free(tabArq);
  // encerra disco e retorna estatisticas
  return fd_stop();
}

// Cria um arquivo vazio
int ffs_create(){
	struct i_node_block *bufferInodeBlock = NULL;
	bufferInodeBlock = malloc(sizeof(struct i_node_block));
  // le superbloco
  struct super_block *bufferSuper = NULL;
	bufferSuper = malloc(sizeof(struct super_block));
  fd_read_super_block(0, bufferSuper);
  // numero de inodes no bloco de inodes
  int numInodes = (get_block_size() / I_NODE_SIZE);
  for(int i = 1; i <= bufferSuper->iSize; ++i) {
    // le o bloco de inode
    fd_read_i_node_block(i, bufferInodeBlock);
    // percorre inodes
    for(int j = 1; j <= numInodes; ++j) {
      // se inode estiver livre
      if(!bufferInodeBlock->p[j-1].flags){
        // ocupa inode
        bufferInodeBlock->p[j-1].flags = 1;
        fd_write_i_node_block(i, bufferInodeBlock);
				// retorna inumber
        free(bufferInodeBlock);
        free(bufferSuper);
        return (j + (i-1) * numInodes);
      }
    }
  }
  // sem espaco para novos inodes
	free(bufferInodeBlock);
  free(bufferSuper);
  return 0;
}

// Localiza um arquivo existente
// Retorna o indice do fd correspondente
int ffs_open(int i_number){
  struct i_node_block *bufferInodeBlock = NULL;
	bufferInodeBlock = malloc(sizeof(struct i_node_block));
  // le superbloco
  struct super_block *bufferSuper = NULL;
	bufferSuper = malloc(sizeof(struct super_block *));
  fd_read_super_block(0, bufferSuper);
  // conta inumber dos inodes
  int countInumber = 1;
  // numero de inodes a percorrer no bloco
  int numInodes = (get_block_size() / I_NODE_SIZE);
  // percorre blocos de inode
  for(int i = 1; i <= bufferSuper->iSize; ++i) {
    // le o bloco de inode
    fd_read_i_node_block(i, bufferInodeBlock);
    // percorre inodes
    for(int j = 1; j <= numInodes; ++j) {
      // se for o inumber desejado
      if (countInumber == i_number) {
        // se o arquivo existir
        if(bufferInodeBlock->p[j-1].flags) {
          // vincula fd com inode
          for(int k = 0; k < 20; ++k) {
            // se ha vaga para abrir o arquivo
            if(tabArq->bitmap[k] == '0') {
              // coloca arquivo no array de descritores
              tabArq->fdArray[k].i_node = bufferInodeBlock->p[j-1];
              tabArq->fdArray[k].inumber = i_number;
              tabArq->fdArray[k].seekptr = 0;
              // abre o arquivo
              tabArq->bitmap[k] = '1';
              // retorna o indice do fd
							free(bufferInodeBlock);
							free(bufferSuper);
              return k;
            }
          }
          // ja tem 20 arquivos abertos
					free(bufferInodeBlock);
					free(bufferSuper);
          return -1;
        } else {
          // arquivo nao existe
					free(bufferInodeBlock);
					free(bufferSuper);
          return -2;
        }
      }
      // incrementa contador de inumber
      ++countInumber;
    }
  }
  // inumber nao encontrado
	free(bufferInodeBlock);
	free(bufferSuper);
  return -1;
}

// Recebe descritor de arquivos
// Retorna o inumber de um arquivo aberto
int ffs_i_number(int fd){
  // se arquivo esta aberto
  if(tabArq->bitmap[fd] == '1') {
    // recebe descritor do arquivo aberto
    struct fd desc = tabArq->fdArray[fd];
    // retorna inumber
    return desc.inumber;
  }
  // arquivo nao esta aberto
  return -1;
}

// Le arquivo
// Le ate buffer.length() bytes a partir do ponteiro de busca atual
// Retorna o número de bytes lidos
// Se ponteiro de busca for maior ou igual ao tam do arq: retorna 0
// Caso contrario incrementa o ponteiro de busca pelos n bytes lidos
int ffs_read(int fd, char * buffer, int size){
	// se o arquivo esta aberto
	if(tabArq->bitmap[fd] == '1') {
		// estrutura para receber bloco de inode
		struct i_node_block *bufferInodeBlock = NULL;
		bufferInodeBlock = malloc(sizeof(struct i_node_block));
		int indiceBlocoInode = (tabArq->fdArray[fd].inumber / (get_block_size() / I_NODE_SIZE)) + 1;
		fd_read_i_node_block(indiceBlocoInode, bufferInodeBlock);
		int indiceInode = tabArq->fdArray[fd].inumber % (get_block_size() / I_NODE_SIZE);

		struct i_node in = bufferInodeBlock->p[indiceInode];

		// se ponteiro de busca + size ultrapassa o tamanho do arquivo, retorna 0
		if ((tabArq->fdArray[fd].seekptr + size) > in.file_size) {
			return -2;
		}

		// bloco de dados do arquivo em que esta o seekptr
		int blocoPtr;
		if((tabArq->fdArray[fd].seekptr) % get_block_size() == 0)
			blocoPtr = tabArq->fdArray[fd].seekptr / get_block_size();
		else
			blocoPtr = tabArq->fdArray[fd].seekptr / get_block_size() + 1;
		// bloco de dados do arquivo em que o seekptr vai parar com a leitura
		int blocoArq;
		if((tabArq->fdArray[fd].seekptr + size) % get_block_size() == 0)
			blocoArq = (tabArq->fdArray[fd].seekptr + size) / get_block_size();
		else
			blocoArq = (tabArq->fdArray[fd].seekptr + size) / get_block_size() + 1;

		// le superbloco
		struct super_block *bufferSuper = NULL;
		bufferSuper = malloc(sizeof(struct super_block));
		fd_read_super_block(0, bufferSuper);

		blocoPtr += 1 + bufferSuper->iSize + bufferSuper->tamLivre;
		blocoArq += 1 + bufferSuper->iSize + bufferSuper->tamLivre;

		// chars de inicio e de fim da leitura
		int inicio = tabArq->fdArray[fd].seekptr;
		int fim = tabArq->fdArray[fd].seekptr + size;

		// limites de leitura em cada bloco
		int inicioBloco, fimBloco;
		// indice do buffer de leitura
		int k = 0;
		// aloca string para leitura de dados
		char *dados = NULL;
		dados = malloc(get_block_size());

		// percorre cada bloco a ser lido
		for(int i = blocoPtr; i <= blocoArq; ++i) {
			// le um bloco de dados
			fd_read_raw(i, dados);
			// se for apenas um bloco a ler
			if(blocoPtr == blocoArq) {
				inicioBloco = inicio % get_block_size();
				fimBloco = fim % get_block_size();
			// se for mais de um bloco
			} else {
				// se for o primeiro bloco
				if (i == blocoPtr) {
					inicioBloco = inicio % get_block_size();
					fimBloco = get_block_size();
				// se for o ultimo bloco
				} else if (i == blocoArq) {
					inicioBloco = 0;
					fimBloco = fim % get_block_size();
				// se for um bloco intermediario
				} else {
					inicioBloco = 0;
					fimBloco = get_block_size();
				}
			}
			for (int j = inicioBloco; j < fimBloco; ++j) {
				buffer[k] = dados[j];
				++k;
			}
		}

		// libera dados
		free(dados);
		// incrementa ponteiro de busca atual
		tabArq->fdArray[fd].seekptr += size;
		// retorna numero de caracteres lidos
		return k;
	}
	// arquivo nao esta aberto
  return -1;
}

// Escreve arquivo
// Transfere buffer.length() bytes do buffer para o arquivo a partir do
// ponteiro de busca atual, incrementando o ponteiro de busca pelo valor de
// bytes transferidos
int ffs_write(int fd, char * buffer, int size){
  // se o arquivo esta aberto
  if(tabArq->bitmap[fd] == '1') {
		// estrutura para receber inode
    struct i_node in;
    in = tabArq->fdArray[fd].i_node;

		// numero de blocos atual e apos a adicao do buffer
    int tamAtual, tamNovo;
    if (in.file_size % get_block_size() == 0 )
      tamAtual = in.file_size / get_block_size();
    else
      tamAtual = in.file_size / get_block_size() + 1;
    if((tabArq->fdArray[fd].seekptr + size) % get_block_size() == 0)
      tamNovo = (tabArq->fdArray[fd].seekptr + size) / get_block_size();
    else
      tamNovo = (tabArq->fdArray[fd].seekptr + size) / get_block_size() + 1;

    // se o numero de blocos novo for maior, torna-se o tamanho final
		int tamFinal;
    if(tamNovo > tamAtual)
      tamFinal = tamNovo;
    else
      tamFinal = tamAtual;

    // guarda a matriz de blocos do arquivo com tamanho final
    char update[tamFinal][get_block_size()];
    // string temporaria de um bloco
    char temp[get_block_size()];

    // le o superbloco
		struct super_block *bufferSuper = NULL;
		bufferSuper = malloc(sizeof(struct super_block *));
    fd_read_super_block(0, bufferSuper);

    // k: indice de ponteiro absoluto do arquivo
    // l: indice do buffer de entrada
    int k = 0, l = 0;
    // percorre linhas da matriz update (blocos)
    for(int i = 0; i < tamFinal; ++i) {
      // se for bloco atual, temp recebe ele
      if(i < tamAtual) {
        fd_read_raw(in.p[i], temp);
      }
      // se for bloco novo, limpa o temp
      else {
        for(int j = 0; j < get_block_size(); ++j) {
          temp[j] = ' ';
        }
      }
      // percorre colunas da matriz update (bytes)
      for(int j = 0; j < get_block_size(); ++j) {
        // se estiver no intervalo do buffer, adiciona ele ao update
        if (k >= tabArq->fdArray[fd].seekptr && k < (tabArq->fdArray[fd].seekptr + size)) {
          update[i][j] = buffer[l];
          ++l;
        // senao, adiciona o temp
        } else {
          update[i][j] = temp[j];
        }
      ++k;
      }
    }

    // guarda indice no disco de bloco novo para o arquivo
    int blocoNovo, blocoIndNovo;
    // flag
    int deuBreak = 0;
		// contador
    int contaNovos = 0;
    // percorre matriz update
    for(int i = 0; i < tamFinal; ++i) {
      for(int j = 0; j < get_block_size(); ++j) {
        // guarda linha na string temporaria
        temp[j] = update[i][j];
      }
      // se for bloco novo
      if(i >= tamAtual){
        // ponteiro para lista de livres
        char * bufferFreeList = NULL;
				bufferFreeList = malloc(get_block_size());
        // l: indice absoluto da lista de livres
        int l = 0;
        // percorre lista de livres
        for(int j = 0; j < bufferSuper->tamLivre; ++j) {
          // pega um bloco da lista de livres
          fd_read_raw(1 + bufferSuper->iSize + j, bufferFreeList);
          for(int k = 0; k < get_block_num() - (1 + bufferSuper->iSize + bufferSuper->tamLivre); ++k) {
            // se encontrar um bloco livre
            if(bufferFreeList[k] == '0'){
              if(contaNovos == 0) {
                // guarda indice do bloco novo para o arquivo
                blocoNovo = l + 1 + bufferSuper->iSize + bufferSuper->tamLivre;
                // se o bloco novo nao existir no disco
                if (blocoNovo >= get_block_num()) {
                  // nao ha espaco livre
                  return -1;
                }
                // muda o bloco para ocupado
                bufferFreeList[k] = '1';
                // escreve no disco alteracao na lista de livres
                fd_write_raw(1 + bufferSuper->iSize + j, bufferFreeList);
                // incrementa contador de blocos novos
                ++contaNovos;
              } else if(contaNovos == 1 && i >= tamAtual) {
                // guarda indice do bloco indireto novo
                blocoIndNovo = l + 1 + bufferSuper->iSize + bufferSuper->tamLivre;
                // se o bloco novo nao existir no disco
                if (blocoIndNovo >= get_block_num()) {
                  // nao ha espaco livre
                  return -1;
                }
                // muda o bloco para ocupado
                bufferFreeList[k] = '1';
								// escreve no disco alteracao na lista de livres
                fd_write_raw(1 + bufferSuper->iSize + j, bufferFreeList);
                // incrementa contador de blocos novos
                ++contaNovos;
              }
            }
            ++l;
            // se ja pegou dois blocos novos ou um e nao tem bloco indireto novo
            if(contaNovos == 2 || (contaNovos == 1 && tamFinal < 10)) {
              deuBreak = 1;
              break;
            }
          }
          if (deuBreak) {
            deuBreak = 0;
            break;
          }
        }
				free(bufferFreeList);
      }
      // se for bloco direto
      if(i < 10) {
        // se for bloco novo, atribui-o para o arquivo
        if (i >= tamAtual) {
          in.p[i] = blocoNovo;
        }
        // escreve no disco o temp
        fd_write_raw(blocoNovo, temp);
      // se for bloco indireto
      } else {
        // se nao for bloco indireto novo
        if (i < tamAtual) {
          // leitura do bloco indireto
          struct indirect_block *ib = NULL;
					ib = malloc(sizeof(struct indirect_block));
          fd_read_indirect_block(in.p[i], ib);
          // atribui o bloco novo para o arquivo
          ib->p[i-10] = blocoNovo;
          fd_write_raw(blocoNovo, temp);
					free(ib);
        // se for bloco indireto novo
        } else {
          // escrita do bloco indireto
          struct indirect_block *ib = NULL;
					ib = malloc(sizeof(struct indirect_block));
          ib->p = malloc((get_block_size() / 4) * sizeof(int));
          fd_write_indirect_block(blocoIndNovo, ib);
          // escrita do primeiro bloco direto no indireto
          ib->p[i-10] = blocoNovo;
          fd_write_raw(blocoNovo, temp);
					free(ib);
        }
      }
    }

		// atualiza tamanho do arquivo
		struct i_node_block *bufferInodeBlock = NULL;
		bufferInodeBlock = malloc(sizeof(struct i_node_block));
		int indiceBlocoInode = (tabArq->fdArray[fd].inumber / (get_block_size() / I_NODE_SIZE)) + 1;
		fd_read_i_node_block(indiceBlocoInode, bufferInodeBlock);
		int indiceInode = tabArq->fdArray[fd].inumber % (get_block_size() / I_NODE_SIZE);
		bufferInodeBlock->p[indiceInode].file_size += size;
		fd_write_i_node_block(indiceBlocoInode, bufferInodeBlock);

    free(bufferInodeBlock);
		free(bufferSuper);
		return 1;
  }
  // arquivo nao esta aberto
  return -1;
}

// Modifica o ponteiro de busca de um arquivo
int ffs_seek(int fd, int offset, int whence){
  // se o arquivo estiver aberto
  if(tabArq->bitmap[fd] == '1') {

		struct i_node_block *bufferInodeBlock = NULL;
		bufferInodeBlock = malloc(sizeof(struct i_node_block));
		int indiceBlocoInode = (tabArq->fdArray[fd].inumber / (get_block_size() / I_NODE_SIZE)) + 1;
		fd_read_i_node_block(indiceBlocoInode, bufferInodeBlock);
		int indiceInode = tabArq->fdArray[fd].inumber % (get_block_size() / I_NODE_SIZE);

    // estrutura para receber inode
    struct i_node in = bufferInodeBlock->p[indiceInode];
    // recebe o novo valor do ponteiro
    int valorPtr;
    // whence = 0: deslocamento a partir do inicio do arquivo
    if(whence == 0)
      valorPtr = offset;
    // whence = 1: deslocamento a partir do ponteiro de busca atual
    else if(whence == 1)
      valorPtr = tabArq->fdArray[fd].seekptr + offset;
    // whence = 2: deslocamento a partir do final do arquivo
    else if(whence == 2)
      valorPtr = in.file_size + offset - 1;
    // se valor for negativo ou for maior ou igual que o tamanho do arquivo
    // retorna valor invalido
    if(valorPtr < 0 || valorPtr >= in.file_size)
      return -2;
    // atualiza o ponteiro de busca no fdArray
    tabArq->fdArray[fd].seekptr = valorPtr;
    // retorna ponteiro com novo valor
    return valorPtr;
  }
  // arquivo nao esta aberto
  return -1;
}

// Libera a entrada da tabela de arquivo
int ffs_close(int fd){
  // atualiza o bitmap
  if(tabArq->bitmap[fd] == '1'){
    tabArq->bitmap[fd] = '0';
    return 1;
  }
  // arquivo ja esta fechado
  return 0;
}

// Libera o inode e todos os blocos dos arquivos
// Resulta em erro na tentativa de excluir o arquivo que está aberto
int ffs_delete(int i_number) {
	// percorre fdArray
	for(int i = 0; i < 20; ++i) {
		if (tabArq->fdArray[i].inumber == i_number && tabArq->bitmap[i] == '1') {
			// arquivo aberto
			return -1;
		}
	}

	// estrutura para inode block
	struct i_node_block *bufferInodeBlock = malloc(sizeof(struct i_node_block *));
  // le superbloco
  struct super_block *bufferSuper = malloc(sizeof(struct super_block *));
  fd_read_super_block(0, bufferSuper);

  // calcula numero de inodes no bloco de inodes
  int numInodes = (get_block_size() / I_NODE_SIZE);

  // vetor com blocos a liberar
  int *liberar = NULL;
  int numLiberar = 0;
	// valor do inumber
	int countInumber = 1;

  // percorre blocos de inode
  for(int i = 1; i <= bufferSuper->iSize; ++i) {
  	// le um bloco de inode
		fd_read_i_node_block(i, bufferInodeBlock);
    // percorre inodes
    for(int j = 0; j < numInodes; ++j) {
			// verifica inumber
			if(countInumber == i_number){
				// libera inode
				bufferInodeBlock->p[j].flags = 0;
				int l = 0;
				// enquanto nao encontrar um ponteiro vazio
				while(bufferInodeBlock->p[j].p[l] != -1 && l < 10) {
					if (numLiberar != 0) {
						++numLiberar;
						liberar = realloc(liberar, numLiberar * sizeof(int));
					} else {
						liberar = malloc(sizeof(int));
						++numLiberar;
					}
					// copia valor para vetor
					liberar[numLiberar-1] = bufferInodeBlock->p[j].p[l];
					++l;
				}

				int n = 10;
				while(bufferInodeBlock->p[j].p[n] != -1 && n < 13) {
					++numLiberar;
					liberar = realloc(liberar, numLiberar * sizeof(int));
					liberar[numLiberar-1] = bufferInodeBlock->p[j].p[n];
					++n;
				}

				int m = 0;
				if((l >= 10) && (l < (get_block_size()/4)) && (bufferInodeBlock->p[j].p[l] != -1)){
					struct indirect_block * ib = malloc(sizeof(struct indirect_block));
					fd_read_indirect_block(bufferInodeBlock->p[j].p[l], ib);
					while(ib->p[m] != -1 && m < get_block_size()/4){
						++numLiberar;
						liberar = realloc(liberar, numLiberar * sizeof(int));
						liberar[numLiberar-1] = ib->p[m];
						++m;
					}
					free(ib);
				}
			}
			++countInumber;
    }
	}

	char vazio[get_block_size()];
	for(int i = 0; i < get_block_size(); ++i){
		vazio[i] = ' ';
	}

	for(int i = 0; i < numLiberar; i++){
		fd_write_raw(liberar[i], vazio);
	}

	int i = 0;
	char * bufferFreeList = NULL;
	bufferFreeList = malloc(get_block_size());
	for(int j = 0; j < bufferSuper->tamLivre; ++j) {
		fd_read_raw(1 + bufferSuper->iSize + j, bufferFreeList);
		while (i < numLiberar) {
			bufferFreeList[liberar[i] - (1 + bufferSuper->iSize + bufferSuper->tamLivre)] = '0';
			fd_write_raw(1 + bufferSuper->iSize + j, bufferFreeList);
			++i;
		}
	}

	free(bufferSuper);
	free(bufferInodeBlock);
	free(liberar);
	//liberado espaço corretamente
	return 0;
}
