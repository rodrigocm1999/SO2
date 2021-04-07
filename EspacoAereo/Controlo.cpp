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


	//TODO verificar se ja está a correr antes de inicar as cenas, se ja estiver a correr avisa e termina

	//TODO o controlo vai ter sempre uma maneira de comunicar os os aviões e passageiros. Talvez criar classes para esconder a merda

	//TODO Permite a criação de aeroportos, mediante indicação pela interface com o utilizador do nome do aeroporto e das suas coordenadas
	//	Não deverá existir nenhum outro aeroporto num raio de 10 “posições” nem nenhum outro aeroporto com esse nome. Não existe o conceito de “remover aeroportos”

	//TODO Pode assumir que existe um número máximo tanto de aeroportos como de aviões. 
	// Estas quantidades deverão estar definidas no Registry.Quando os valores máximos são atingidos, os
	// novos aviões que se tentem ligar ao sistema serão ignorados até que este tenha espaço para eles.

	//TODO Interface com utilizador: interface gráfica Win32 que apresenta todo o espaço aéreo e os seus elementos. Os 
	//	aeroportos e os aviões são representados graficamente de forma distinta.Esta informação estará permanentemente visível e sempre atualizada

	//TODO Interface com utilizador: interface gráfica Win32 que apresenta todo o espaço aéreo e os seus elementos. Os 
	//	aeroportos e os aviões são representados graficamente de forma distinta.Esta informação estará permanentemente visível e sempre atualizada
	//		
	//		TODO Numa versão inicial (Meta 1) será aceite uma versão simplificada da interface segundo o 
	//			paradigma consola.Não é para desenhar uma matriz, mas sim para apresentar mensagens de texto tais como “avião ID na posição x, y”.

	//TODO O controlador aéreo considerará que um avião que não dá sinal de vida durante 3 segundos deixou de existir
	//	quer esteja em voo ou parado no aeroporto.O avião é identificado perante o controlador aéreo através do seu ID de processo

	//TODO Comandos:
	//	Encerrar todo o sistema (todas as aplicações são notificadas).
	//	Criar novos aeroportos.
	//	Suspender / ativar a aceitação de novos aviões por parte dos utilizadores.
	//	Listar todos os aeroportos, aviões e passageiros existentes no sistema(com os detalhes de cada um, por	exemplo, destino, no caso dos passageiros).
}
