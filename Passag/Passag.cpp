#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <string>

using namespace std;

#ifdef UNICODE
#define tcout wcout
#define tcin wcin
#define tstring wstring
#define tstringstream wstringstream
#else
#define tcout cout
#define tcin cin
#define tstring string
#define tstringstream stringstream
#endif

#define BUFFER_SIZE 512

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	int val = _setmode(_fileno(stdin), _O_WTEXT);
	val = _setmode(_fileno(stdout), _O_WTEXT);
	val = _setmode(_fileno(stderr), _O_WTEXT);
#endif

	//TODO Funcionamento: 
	//	É indicado por argumento da linha de comandos o aeroporto de origem, o aeroporto de destino, o nome do passageiro, e, opcionalmente,
	//	o tempo(em segundos) que fica a aguardar até embarcar.Os nomes dos aeroportos origem e destino devem existir no sistema, caso contrário o programa termina de imediato
	//	O passageiro é atribuído ao aeroporto origem, ficando a aguardar que exista um avião disponível para o aeroporto destino. 
	//		Quando tal avião existir, o passageiro embarca automaticamente e, ao chegar ao aeroporto destino, desembarca e o programa termina.
	//		Caso tenha sido indicado um tempo de espera máximo, o passageiro desiste automaticamente de viajar se o tempo indicado passar 
	//			e não for atribuído a nenhum avião, sai do aeroporto e o programa termina.

	//TODO Interação:
	//	O utilizador é automaticamente informado de quando embarca, da posição em que está quando está em voo, e quando chega
	//	O utilizador pode sempre interagir com esta aplicação para a terminar. Se o fizer, considera - se que o passageiro deixou de existir
	
	
	//TODO Comandos: 
	//	Deve adicionar os comandos que considerar necessários para a utilização do programa tendo em vista a funcionalidade requerida
}