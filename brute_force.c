#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <crypt.h>

#define TAM_HASH 256

/// @brief Gera uma hash a partir da string com o valor da senha
/// a ser tentada agora.
///
/// @param senha String com o vaklor de uma possivel senha
///
/// @param hash Hash resultante da string 'senha'
void CalculaHashSenha(const char *senha, char *hash);

/// @brief Seta string 'senha' para o proximo valor a ser analizado.
///
/// @param senha String com o valor de uma possivel senha.
///
/// @return >= 0 enquanto está gerando as senhas, -1 caso tenha gerado todas
/// as possiveis senhas e nao encontrou a senha correta
int IncrementaSenha(char *senha);

/// @brief Verifica se a hash gerada pela string 'senha' é identica à hash
///informada inicialmente.
///
/// @param hash_alvo String com a hash informada inicialmente
///
/// @param senha String com o valor de uma possivel senha.
///
/// @return 1 se a senha gerar a hash esperada, senao retorna 0.
int TestaSenha(const char *hash_alvo, const char *senha);

/// @brief Numero de digitos que compem a senha a ser descoberta
int tamanhoSenha = 0;

/// @brief String que será usada para tentar descobrir a senha
char *senha;

int main(int argc, char *argv[])
{
	//flag usada  para saber se a senha foi encontrada
	int achou = 0;

	//variaveis para controle do tempo de duracao da execucao do programa
	clock_t start = 0;
	clock_t finish = 0;

	//verifica se o programa foi executado com a quantidade correta de
	//argumentos, em caso negativo, é apresentada uma mensagem indicando
	//como executar corretamente
	if(argc < 3)
	{
		printf("Uso: %s <tamanho da senha> <hash>", argv[0]);
		return 1;
	}

	//pega o tamanho da senha recebido via argumento e transforma em inteiro
	tamanhoSenha = atoi(argv[1]);
	//aloca memoria para a string que será usada para tentar descobrir a senha,
	//é alocado um byte a mais para o fim da string.
	senha = malloc(sizeof(char) * (tamanhoSenha + 1));
	//inicia os valores da string usada para tentar descobrir a senha com
	//caracteres '0'
	memset(senha, '0', tamanhoSenha);
	//seta o ultimo byte para NULL
	senha[tamanhoSenha] = '\0';

	//init timer
	start = clock();

	//enquanto nao descobriu a senha e ainda há codigos a serem testados, executa
	while ((achou=TestaSenha(argv[2], senha)) == 0 && IncrementaSenha(senha) != -1);

	//finish timer
	finish = clock();

	if(achou)
	{
		printf("%f\n",(float)(finish - start)/CLOCKS_PER_SEC);
	}

	return 0;
}

int TestaSenha(const char *hashAlvo, const char *senha) {
	int result = 0;

	char hashCalculado[TAM_HASH + 1];

	//limpa a variavel hashCalculado
	memset(hashCalculado, '0', TAM_HASH);
	hashCalculado[TAM_HASH] = '\0';

	//gera uma hash a partir da string de senha atual
	CalculaHashSenha(senha, hashCalculado);

	//se as hashes forma iguais, retorna 1
	if (!strcmp(hashAlvo, hashCalculado)) {
		result = 1;
	}

	return result;
}

int IncrementaSenha(char *senha)
{
	//vai incrementando os valores dos codigos de senha dos caracter mais à
	//direita para os caracteres mais à esquerda. Ex.: 0000, 0001, 0002 ... 0010...
	int index = tamanhoSenha - 1;

	while (index >= 0)
	{
		//posicao da senha, com valor diferente de 9,
		//incrementa aquela posicao da senha e retorna -2
		if (senha[index] != '9')
		{
			senha[index]++;

			index = -2;
		}
		//posicao da senha, com valor igual a 9,
		//seta pra '0'
		else
		{
			senha[index] = '0';
		}

		//vai para o indice previo do caracter da senha
		index--;
	}

	return index;
}

void CalculaHashSenha(const char *senha, char *hash) {
	struct crypt_data data;
	data.initialized = 0;

	//pega a hash gerada a partir da string de senha atual
	strcpy(hash, crypt_r(senha, "aa", &data));
}
