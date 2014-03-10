#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include "allegro5/allegro_image.h"

#define QTD_BT 4//quantidade de botoes
#define QTD_VERSOES 1//quantidade de versoes suportadas

/*Estrutura do botao*/
typedef struct {
	int x, y; /*coordenadas*/
	ALLEGRO_BITMAP *img; /*imagem do botao*/
	bool ativo;//indica se o botao esta ativo
} Botao;

const char VERSOES[QTD_VERSOES] = {'5'};
char tipo[2];//tipo de PGM
char *erroMsgBuf;//mesagem de erro gerada pelo metodo
const int btL = 70, btA =50;//altura e largura dos botoes
int bL, bA; //altura e largura da barra de botoes
int jL, jA; //altura e largura da janela
int corFundo = 125;
/*Cria uma interface grafica nativa para o usuario selecionar um arquivo
@param
	ALLEGRO_DISPLAY *janela - janela onde ira mostrar a interface 
*/
char * uiGetDiretorio(ALLEGRO_DISPLAY *janela, const char *msg, int flag) {
	ALLEGRO_FILECHOOSER *file_dialog;
	file_dialog = al_create_native_file_dialog("c:/", msg, "*.pgm*",flag);
	al_show_native_file_dialog(janela, file_dialog);
	return (char*)al_get_native_file_dialog_path(file_dialog, 0);
}

/*Metodo responsavel por fazer apontamendo para um arquivo no disco
@param
	FILE *arquivo - ponteiro para arquivo no disco
	char *tp - identifica o tipo de IO (gravacao ou leitura)
	ALLEGRO_DISPLAY *janela - janela onde ira mostrar a interface 
*/
FILE *abreArquivo(char *tp, ALLEGRO_DISPLAY *janela) {
	FILE *arquivo;
	char *msg;
	int flag=0;
	
	if(tp == "r+b") {
		msg = "Abrir Imagem";
		flag=0;
	}
	
	if(tp == "w+b") {
		msg = "Salvar Imagem";
		flag = 2;
	}
	else {
		msg = "Arquivo";	
	}
	
	const char *diretorio = uiGetDiretorio(janela, msg, flag);
	if(diretorio == NULL) {
		printf("Operacao cancelada\n");
	}
	
	else if((arquivo = fopen(diretorio, tp))==NULL) {
		printf("Arquivo '%s' nao pode ser aberto\n",diretorio);
		erroMsgBuf = "Arquivo nao pode ser aberto.";
	}
	
	else {
		printf("O arquivo %s foi aberto\n",diretorio);
		return arquivo;
	}
	return NULL;
}

/*aloca memoria para a matriz
@param
	int a - altura da matriz (qtd de linhas)
	int l - larguda da matriz (qtd de colunas)
*/
unsigned char **alocaMatriz(int a, int l) {
	int i;
	unsigned char **data = (unsigned char **) malloc (sizeof(unsigned char *) * a);
	for(i=0; i<a; i++) {
		data[i] = (unsigned char *) malloc (sizeof(unsigned char) * l);
	}
	
	return data;
}

/*libera memoria utilizada pela matriz
@param
	unsigned char data** - matriz
	int a - altura da matriz (qtd de linhas)
*/
void desalocaMatriz(unsigned char **data, int a) {
	int i;
	for(i=0; i<a; i++)
		free(data[i]);
	free(data);
}

/*pega cabecalho da imagem
@param
	FILE *arquivo - ponteiro para arquivo no disco
	int *altura - altura da matriz (qtd de linhas)
	int *largura - larguda da matriz (qtd de colunas)
	int *maxCor -  inteiro que representa tonalidade de cor maxima
	char *tipo - tipo de PGM
*/
int getCabecalho(FILE *arquivo, int *altura, int *largura, int *maxCor, char *tipo) {
	
	int i, buf = 0;

	/*Verifica 'P' no inicio do arquivo*/
	buf = getc(arquivo);
	if(buf == 'P')
		tipo[0] = buf;
	else {
		printf("erro, arquivo PMG invalido\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return -1;
	}

	/*Verifica versão da imagem*/
	buf = getc(arquivo);
	for(i=0; i< QTD_VERSOES; i++) {
		if(buf==VERSOES[i])
			break;	
	}
	if(i < QTD_VERSOES) {
		tipo[1] = buf;
	}
	else {
		printf("erro, versao invalida ou nao suportada!\n");
		erroMsgBuf = "Versao invalida ou nao suportada.";
		return -1;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return -1;
	}

	/*verifica largura*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		*largura = buf;
	else {
		printf("erro ao pegar largura da imagem!\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return 0;
	}

	/*verifica altura*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		*altura = buf;
	else {
		printf("erro ao pegar altura da imagem!\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return 0;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return -1;
	}

	/*verifica maxCor*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		*maxCor = buf;
	else {
		printf("erro ao pegar maxCor!\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return 0;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return -1;
	}

	printf("GETCABECALHO Tipo:%s Altura:%d Largura:%d CorMax:%d\n", tipo, *altura, *largura, *maxCor);
	return 0;
}

/*pega bits da imagem e armazena
@param
	FILE * arquivo - ponteiro para o arquivo no disco
	unsigned char ** data - matriz onde sera armazenado os bits
	int a - altura da imagem
	itn l - largura da imagem
*/
int armazenaDados(FILE * arquivo, unsigned char **data, int a, int l, int m) {
	int i, j, 
		buf;//buf do tipo int para detectar o EOF
	
	for(i=0; i<a; i++) {
		for(j=0; j<l; j++) {
			buf = getc(arquivo);
			if(buf > m || buf < 0 || buf == EOF) {
				printf("Dados inconsistentes!\n");
				erroMsgBuf = "Dados da imagem inconsistentes.";
				return -1;
			}
			else {
				data[i][j] = buf;
			}
		}
	}
	buf = getc(arquivo);
	if(buf!=EOF) {
		erroMsgBuf = "Dados da imagem inconsistentes.";
		return -1;
	}

	else return 0;
}

/*Altera o tamanho da janela
@param
	ALLEGRO_DISPLAY *janela - janela onde ira desenhar a imagem
	int h - altura da imagem
	itn w - largura da imagem
*/
void resize(ALLEGRO_DISPLAY *janela, int h, int w) {
    if(w<bL)
		w=bL;
	
	al_resize_display(janela, w, h + bA);
	al_clear_to_color(al_map_rgb(255, 255, 255));
	al_flip_display();	
}

/*Desenha a imagem na janela
@param
	ALLEGRO_DISPLAY *janela - janela onde ira desenhar a imagem
	unsigned char ** data - matriz onde sera armazenado os bits
	int a - altura da imagem
	int l - largura da imagem
	int x, y - coordenadas em que ira desenhar  
*/
void desenha(ALLEGRO_DISPLAY *janela, unsigned char **data, int a, int l, int x, int y) {	
	/*reajusta o tamanho da janela para a imagem aberta*/
	resize(janela, a, l);
    al_clear_to_color(al_map_rgb(corFundo, corFundo, corFundo));
	int i, j;
	for (i=0; i<a; i++) {
		for(j=0; j<l; j++) {
			al_draw_pixel(x + j, y + i, al_map_rgb(data[i][j],data[i][j],data[i][j]));
		}
	}
}

/*funcao responsavel por carregar imagem
@param	
	ALLEGRO_DISPLAY *janela - janela onde ira desenhar a imagem
	int a - altura da imagem
	itn l - largura da imagem
	int *maxCor -  inteiro que representa tonalidade de cor maxima
	char *tipo - tipo de PGM
	unsigned char ** data - matriz onde sera armazenado os bits
*/
int carregaImagem (ALLEGRO_DISPLAY *janela, int *altura,int *largura, int *maxCor, char *tipo, unsigned char ***data) {
	FILE *arquivo;

	/*ponteiro para o arquivo de imagem no disco*/
	arquivo = abreArquivo("r+b", janela);
	if(arquivo==NULL) return -2;

	/*pega tamanho e tipo da imagem(cabecalho)*/
	if(getCabecalho(arquivo, altura, largura, maxCor, tipo)==-1) {
		return -1;
	}

	/*aloca memoria para a matriz*/
	*data = alocaMatriz(*altura, *largura);

	/*armazena os bits na matriz*/
	if(armazenaDados(arquivo, *data, *altura, *largura, *maxCor)==-1) {
		int resposta = 0;
		resposta = al_show_native_message_box(janela, "Alerta", erroMsgBuf, "Deseja continuar?", NULL, ALLEGRO_MESSAGEBOX_OK_CANCEL);
		
		if(resposta == 1)
			return 0;

		else
			return -2;
	}

	/*destroy ponteiro*/
	fclose(arquivo);
}

/*grava cabecalho no arquivo no disco
@param
	FILE *out - ponteiro para um arquivo no disco
	char *tipo - tipo de PGM
	int altura - altura da imagem
	int largura - largura da imagem
	int maxCor - inteiro que representa tonalidade de cor maxima
*/
int putCabecalho(FILE *out, char *tipo, int altura, int largura, int maxCor) {
	char buf[20];
	sprintf(buf, "%s\n%d %d\n%d\n", tipo, largura, altura, maxCor);
	
	if(fputs(buf,out)==EOF) {
		printf("Erro ao gravar cabecalho");
		erroMsgBuf = "Erro ao tentar gravar cabecalho da imagem.";
		return -1;
	}
			
	//printf("PUTCABECALHO:%s",&buf);
	return 0;
}

/*grava bits da imagem no arquivo no disco
@param
	FILE *out - ponteiro para um arquivo no disco
	unsigned char **data - matrix com os bits da imagem
	int altura - altura da imagem
	int largura - largura da imagem
	int maxCor - inteiro que representa tonalidade de cor maxima
*/
int gravaDados(FILE *out, unsigned char **data, int altura, int largura, int maxCor){
	int i, j;
	for(i=0; i<altura; i++) {
		for(j=0; j<largura; j++){
			if(data[i][j] <0 || data[i][j]>maxCor) {
				printf("Erro! dados inconsistentes\n");
				erroMsgBuf = "Erro de dados na gravacao da imagem.";
				return -1; 
			}
			putc(data[i][j], out);
		}
	}
	return 0;
}

/*funcao responsavel por gravar a imagem no disco
@param
	ALLEGRO_DISPLAY *janela - janela onde ira desenhar a imagem
	char tipo - tipo de PGM
	int altura - altura da matriz (qtd de linhas)
	int largura - larguda da matriz (qtd de colunas)
	int maxCor - inteiro que representa tonalidade de cor maxima
	unsigned char ** data - matriz onde sera armazenado os bits
*/
int gravaImagem(ALLEGRO_DISPLAY *janela, char *tipo, int altura, int largura, int maxCor, unsigned char **data){
	FILE *out = abreArquivo("w+b", janela);
    if(out==NULL) return -2;
	if(putCabecalho(out, tipo, altura, largura, maxCor)==-1){return-1;}
	if(gravaDados(out, data,altura,largura,maxCor)==-1){return-1;}
	fclose(out);
	printf("Imagem Salva!\n");
	return 0;
}

unsigned char **rotacao(unsigned char **data, int *altura, int *largura, char sentido) {
    unsigned char **matriz = alocaMatriz(*largura, *altura);         
    int i,j;
    int aux = *largura-1;
    
    printf("%d %d\n", *altura, *largura);
    if(sentido == 'E') {
        for(i=0; i<*largura; i++) {
            for(j=0; j<*altura; j++) {
                matriz[i][j] = data[j][aux];
            }
            aux--;
        }
    } 
    else { 
         aux = *altura-1;
        for(i=0; i<*altura; i++) {
            for(j=0; j<*largura; j++) {
                matriz[j][i] = data[aux][j];
            }
            aux--;
        }
    }
    desalocaMatriz(data, *altura);
    j = *altura;
    *altura = *largura;
    *largura = j;    
    return matriz;
}

/*define automaticamente a posicao do botao, btL = largura do botao*/
void defBotaoPos(Botao b[]) {
	int i, x=0;
	for(i=0; i<QTD_BT; i++) {
		b[i].x = x;
		b[i].y = 0;
		x += btL;
	}
}

/*carrega a imagem do botao*/
ALLEGRO_BITMAP *carregaBitmapBT(char * dir) {
	/*cria o bitmap para imagem*/
	ALLEGRO_BITMAP *b = al_load_bitmap(dir);

	if (!b) {
        fprintf(stderr, "Falha ao carregar imagem %s.\n", dir);
		al_destroy_bitmap(b);
		erroMsgBuf = "Falha ao carregar imagem do botao.";
        return NULL;
    }
	return b;
}


/*
Cria os botoes do menu.
Caso for adicionar um botao novo, 
aumentar a variavel QTD_BT (variavel #define no comeco do arquivo que 
representa a quantidade de botoes)*/
int criaMenu(Botao b[]) {
	/*carrega imagens dos botoes.*/
	int i;

	/*carrega imagens dos botoes*/
	b[0].img = carregaBitmapBT("carregar.bmp");
	b[1].img = carregaBitmapBT("salvar.bmp");
	b[2].img = carregaBitmapBT("girar_horario.bmp");
	b[3].img = carregaBitmapBT("girar_anti.bmp");
	
	
	for(i=0; i<QTD_BT;i++) {
		if(b[i].img==NULL)/*verifica se todos os botoes foram carregados com sucesso*/
			return -1;
		
		else
			al_set_target_bitmap(b[i].img);/*nao tenho certeza o que isso faz =P */
	}

	/*define posicoes dos botoes*/
	defBotaoPos(b);
	
	return 0;
}

/*desenha botoes do menu*/
int desenhaMenu(ALLEGRO_DISPLAY *janela, Botao bt[]) {
	int i;
	float r=1, g=1, b=1, a=1;//cores red green blue, 1=100%
	
	for(i=0; i<QTD_BT;i++) {
		/*se o botao estiver inativo, escurece o botao*/
		if(bt[i].ativo==false) {
			r=.3;
			g=.3;
			b=.3;
		}
		else {
			r=1;
			g=1;
			b=1;
		}
		/*desenha o bitmap com as cores especificadas pelo al_map_rgba_f*/
		al_draw_tinted_bitmap(bt[i].img, al_map_rgba_f(r, g, b, a),bt[i].x ,bt[i].y, 0);
	}

	return 0;
}
/*verifica se foi clicado na posicao de algum botao e retorna o numero do botao que foi clicado
caso nao clique em nenhum botao , retorn a -1*/
int verificaClique(int x, int y, Botao b[]) {
	int i=0;
	for(i=0; i<QTD_BT; i++) {
		if(x > b[i].x && x < b[i].x + btL && y > b[i].y && y<b[i].y + btA)
			return i;
	}
	return -1;
}

/*metodo principal, gerencia a janela*/
int main(int argc, char argv[]) {	
	/*janela principal*/
    ALLEGRO_DISPLAY *janela = NULL;

	/*fila de eventos*/
	ALLEGRO_EVENT_QUEUE *filaEvento = NULL;

	/*evento atual*/
	ALLEGRO_EVENT evento;
	
	/*ponteiro para um arquivo no dico*/
	FILE *arquivo;

	/*array de botoes*/
	Botao botoes[QTD_BT];
	
	/*definicoes do arquivo*/
	int altura = 0,//altura da imagem
		largura = 0, //largura da imagem
		maxCor = 0;//inteiro que representa tonalidade de cor maxima

	/*estrutura para armazenar os bits do arquivo*/
	unsigned char **data = NULL;
	
	/*sinaliza fechamento da janela*/
	bool fechaJanela = false;
	
	/*sinaliza existencia de arquivo aberto*/
	bool arquivoAberto = false;
 
	
	/*inicializa biblioteca principal*/
	if(al_init()==-1)
	{
		printf("Erro ao iniciar biblioteca do allegro!");
		exit(0);
	}

	 /*inicializa bibliotecas*/
    if( al_install_mouse()==-1 ||
		al_install_keyboard()==-1 ||
		al_init_primitives_addon()==-1 ||
		al_init_image_addon() ==-1||
		(filaEvento = al_create_event_queue())==false) {
		al_show_native_message_box(janela, "ERRO FATAL", "O Programa nao pode ser inciado", "Erro ao iniciar bibliotecas do allegro.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		printf("Erro ao inicializar biblioteca(s)!\n"); 
		exit(0);
	}
	/*define tamanho da barra de botoes*/
	
    bL = btL * QTD_BT;
	bA = btA;

	/*define tamanho da janela*/
	jL = bL ;
	jA = 400 ;

	/*cria uma janela*/
    janela = al_create_display(jL, jA);

	/*cria barra de botoes*/
	if(criaMenu(botoes)==-1) {
		al_show_native_message_box(janela, "ERRO FATAL", "Falha ao cria barra de botoes", erroMsgBuf, NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return 0;
	}

	/*define botoes ativos e inativos*/
	botoes[0].ativo=true;
	botoes[1].ativo=false;
	botoes[2].ativo=false;
	botoes[3].ativo=false;

    /*registra os eventos*/
	al_register_event_source(filaEvento, al_get_display_event_source(janela));
	al_register_event_source(filaEvento, al_get_mouse_event_source());
	al_register_event_source(filaEvento, al_get_keyboard_event_source());

	/*define a janela em que vai ser desenhado o botao*/
	al_set_target_bitmap(al_get_backbuffer(janela));
	
	/*preenche a janela com a cor branca*/
    al_clear_to_color(al_map_rgb(corFundo, corFundo, corFundo));

	/*desenha o menu*/
	desenhaMenu(janela, botoes);

	/*atualiza tela*/
    al_flip_display();

	/*fluxo principal da janela*/
	while(fechaJanela == false) {
		
		/*pega evento da fila*/
		al_wait_for_event(filaEvento, &evento);

		/*fecha a janela (termina aplicacao)*/
		if(evento.type==ALLEGRO_EVENT_DISPLAY_CLOSE) {
			fechaJanela = true;
			break;
		}
			
		
		if(evento.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
			printf("precionou botao %d\n", verificaClique(evento.mouse.x, evento.mouse.y, botoes));
			
			switch (verificaClique(evento.mouse.x, evento.mouse.y,botoes)) {
				
				case 0:/*carrega e desenha a imagem*/
					if(botoes[0].ativo==true) {	
						/*caso algum arquivo estiver aberto, limpa dados*/
						if(arquivoAberto==true) {
							desalocaMatriz(data, altura);
							altura =0;
							largura =0;
							maxCor =0;
							arquivoAberto = false;
						}
						if(arquivoAberto==false) {
							/*carrega imagem na matriz*/
							int resposta = carregaImagem (janela, &altura,&largura, &maxCor, tipo, &data);
							if(resposta <0) {
								if(resposta == -1)//caso reposta seja -2, a mesangem de erro ja foi tratada pelo metodo.
									al_show_native_message_box(janela, "Erro", "Erro ao abrir imagem.", erroMsgBuf, NULL, ALLEGRO_MESSAGEBOX_ERROR);
						
								printf("Erro ao desenhar imagem!\n");
					
								/*desativa botoes*/
								botoes[1].ativo=false;
								botoes[2].ativo=false;
								botoes[3].ativo=false;
								
								/*limpa a tela*/
								al_clear_to_color(al_map_rgb(corFundo, corFundo, corFundo));
								
								/*redesenha menu*/
								desenhaMenu(janela, botoes);

								/*atualiza tela*/
								al_flip_display();
								break;
							}
					
							/*sinaliza como arquivo aberto*/
							arquivoAberto = true;
							
							/*ativa botoes*/
							botoes[2].ativo=true;
							botoes[3].ativo=true;

							/*desenha a imagem na janela, 0 e bA sao as coordenadas x,y de onde vai comeca desenhar a imagem
							sendo que bA representa a altura do menu*/
							desenha(janela, data, altura, largura, 0, bA);
							
							/*redesenha menu*/
							desenhaMenu(janela, botoes);

							/*atualiza tela*/
							al_flip_display();
						}
					}
					else {
						printf("Botao inativo\n");
					}
				break;//fim case 0
			
				case 1:/*grava imagem*/
					if(botoes[1].ativo == true) {
						int resposta = gravaImagem(janela, tipo, altura, largura, maxCor, data);
						if(resposta ==-1 ) {
							printf("Erro ao salvar imagem!\n");
							al_show_native_message_box(janela, "Erro", "Erro ao salvar imagem.", erroMsgBuf, NULL, ALLEGRO_MESSAGEBOX_ERROR);
							break;
						}
						if(resposta==-2) {//usuario clico em cancelar
							break;
						}
					
					}
					else {
						printf("Botao inativo\n");			
					}
					botoes[1].ativo=false;
					desenhaMenu(janela, botoes);
					al_flip_display();
				break;//fim case1

				case 2:/*gira horario*/
					if(botoes[2].ativo==true) {
						data = rotacao(data, &altura, &largura, 'D');
						desenha(janela, data, altura, largura, 0, bA);
						botoes[1].ativo=true;
						desenhaMenu(janela, botoes);
						al_flip_display();
					}
					else {
						printf("Botao inativo\n");
					}
				break;

				case 3:/*gira anti-horario*/
					if(botoes[3].ativo==true) {
						data = rotacao(data, &altura, &largura, 'E');
						desenha(janela, data, altura, largura, 0, bA);
						botoes[1].ativo=true;
						desenhaMenu(janela, botoes);
						al_flip_display();
					}
					else {
						printf("Botao inativo\n");
					}
				break;
		
				default:
				break;
			}//fim switch
		}//fim if(evento.mouse.button)
	}//fim while

	/*limpeza*/
	if(data!=NULL && arquivoAberto == true) {
		desalocaMatriz(data, altura);
	}
	int i;
	for(i=0; i<QTD_BT; i++){
		al_destroy_bitmap(botoes[i].img);
	}
	al_shutdown_image_addon();
    al_destroy_event_queue(filaEvento);
	al_uninstall_mouse();
    al_uninstall_keyboard();
	al_destroy_display(janela);
    return 0;
}


