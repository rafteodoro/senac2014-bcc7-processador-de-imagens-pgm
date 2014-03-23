#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
#include "allegro5/allegro_image.h"

#define QTD_BT 8//quantidade de botoes
#define QTD_BT_FILEIRA 6//quantidade de botoes
#define QTD_VERSOES 1//quantidade de versoes suportadas
#define QTD_INTERVALO 244//quantidade maxima de intervalos
/*Estrutura do botao*/
typedef struct {
	int x, y; /*coordenadas*/
	ALLEGRO_BITMAP *img; /*imagem do botao*/
	bool ativo;//indica se o botao esta ativo
} Botao;

/* Estrutura do undo/redo */
struct Nodo {
       unsigned char  **data;
       int largura, altura;
       struct Nodo* prox;
       struct Nodo* ante;
};

struct Nodo *head;

const char VERSOES[QTD_VERSOES] = {'5'};
char tipo[2];//tipo de PGM
char *erroMsgBuf;//mesagem de erro gerada pelo metodo
const int btL = 90, btA =25;//altura e largura dos botoes
int bL, bA; //altura e largura da barra de botoes
int jL, jA; //altura e largura da janela
int corFundo = 125;
/*Cria uma interface grafica nativa para o usuario selecionar um arquivo
@param
	ALLEGRO_DISPLAY *janela - janela onde ira mostrar a interface 
*/
char * uiGetDiretorio(ALLEGRO_DISPLAY *janela, const char *msg, int flag) {
	ALLEGRO_FILECHOOSER *file_dialog;
	file_dialog = al_create_native_file_dialog("c:/", msg, "*.*",flag);
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
int getCabecalho(FILE *arquivo, int *maxCor, char *tipo, Nodo *no) {
	
	int i, buf = 0;

	/*Verifica 'P' no inicio do arquivo*/
	buf = getc(arquivo);
	if(buf == 'P')
		tipo[0] = buf;
	else {
		printf("erro, arquivo PGM invalido\n");
		erroMsgBuf = "Arquivo PGM invalido.";
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
		erroMsgBuf = "Arquivo PGM invalido.";
		return -1;
	}

	/*verifica largura*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		no->largura = buf;
	else {
		printf("erro ao pegar largura da imagem!\n");
		erroMsgBuf = "Arquivo PGM invalido.";
		return 0;
	}

	/*verifica altura*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		no->altura = buf;
	else {
		printf("erro ao pegar altura da imagem!\n");
		erroMsgBuf = "Arquivo PGM invalido.";
		return 0;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		erroMsgBuf = "Arquivo PGM invalido.";
		return -1;
	}

	/*verifica maxCor*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		*maxCor = buf;
	else {
		printf("erro ao pegar maxCor!\n");
		erroMsgBuf = "Arquivo PGM invalido.";
		return 0;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		erroMsgBuf = "Arquivo PGM invalido.";
		return -1;
	}

	printf("GETCABECALHO Tipo:%s Largura:%d Altura: %d CorMax:%d\n", tipo,  no->largura, no->altura, *maxCor);
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
	if(h<bA)
		h=bA;
	
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
	if(x<0)x=0;
	if(y<0)y=0;
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
int carregaImagem (ALLEGRO_DISPLAY *janela, int *maxCor, char *tipo, Nodo *no) {
	FILE *arquivo;

	/*ponteiro para o arquivo de imagem no disco*/
	arquivo = abreArquivo("r+b", janela);
	if(arquivo==NULL) return -2;

	/*pega tamanho e tipo da imagem(cabecalho)*/
	if(getCabecalho(arquivo, maxCor, tipo, no)==-1) {
		return -1;
	}

	/*aloca memoria para a matriz*/
	no->data = alocaMatriz(no->altura, no->largura);

	/*armazena os bits na matriz*/
	if(armazenaDados(arquivo, no->data, no->altura, no->largura, *maxCor)==-1) {
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
			//if(data[i][j] <0 || data[i][j]>maxCor) {
				printf("Erro! dados inconsistentes\n");
				erroMsgBuf = "Erro de dados na gravacao da imagem.";
			//	return -1; 
			//}
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
    
    j = *altura;
    *altura = *largura;
    *largura = j;    
    return matriz;
}

/*define automaticamente a posicao do botao, btL = largura do botao*/
void defBotaoPos(Botao b[]) {
	int i, j=0, btnCont=0, x;
	int qtdFileira=1 + (QTD_BT/QTD_BT_FILEIRA);
	for(i=0; i<qtdFileira; i++){
		x=0;
		for(j=0; j<QTD_BT_FILEIRA; j++){
			b[btnCont].x = x;
			b[btnCont].y = (btA*i);
			btnCont++;
			if(btnCont >= QTD_BT)
				break;
			x += btL;
		}
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
	b[0].img = carregaBitmapBT("carregar.png");
	b[1].img = carregaBitmapBT("salvar.png");
	b[2].img = carregaBitmapBT("girar_horario.png");
	b[3].img = carregaBitmapBT("girar_anti.png");
	b[4].img = carregaBitmapBT("undo.png");
	b[5].img = carregaBitmapBT("redo.png");
	b[6].img = carregaBitmapBT("alteraCor.png");
	b[7].img = carregaBitmapBT("dithering.png");
	
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
			r=.2;
			g=.2;
			b=.2;
			a=.4;
		}
		else {
			r=1;
			g=1;
			b=1;
			a=1;
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


struct Nodo* GetNovoNodo (unsigned char **data, int lar, int alt){
     struct Nodo* novoNodo = (struct Nodo*)malloc (sizeof(struct Nodo));
     novoNodo->data = data ;
     novoNodo->largura = lar;
     novoNodo->altura = alt;
     novoNodo->ante = NULL;
     novoNodo->prox = NULL;
     return novoNodo; 
}

void InserirNodo (unsigned char **data, int lar, int alt) {
     struct Nodo* novoNodo = GetNovoNodo(data, lar, alt);
     
     if (head == NULL) {
              head = novoNodo;
     }
     
     else{
          head->ante = novoNodo;
          novoNodo->prox = head;
          head = novoNodo;
     }     
}

unsigned char **reducaoCores(unsigned char **data, int altura, int largura, int maxCor){
	int qtd = 0, pixel;
	int i, j, k;
	unsigned char **matriz;
	const char *diretorio = "cor.txt";
	matriz = alocaMatriz(altura, largura);
	FILE *arquivo;

	if((arquivo = fopen(diretorio, "r+b"))==NULL) {
		printf("Arquivo '%s' nao pode ser aberto\n",diretorio);
		erroMsgBuf = "Arquivo cor.txt nao pode ser encontrado.";
		desalocaMatriz(matriz, altura);
		fclose(arquivo);
		return NULL;
	}

	if(fscanf(arquivo, "%d", &qtd) != 1){
		printf("Texto formatado errado\n");
		erroMsgBuf = "Arquivo de configuracao invalido.";
		desalocaMatriz(matriz, altura);
		fclose(arquivo);
		return NULL;
	}
	
	if(qtd <2 || qtd > maxCor -1){
        printf("a qtd cor nao esta entre 2 e %d, qtd lido:%d\n",maxCor, qtd);
		erroMsgBuf = "Arquivo de configuracao invalido.";
		desalocaMatriz(matriz, altura);
		fclose(arquivo);
        return NULL;       
	}

    fclose(arquivo);
	int intervalo[QTD_INTERVALO];
	for(i=0; i < qtd; i++){
		intervalo[i] = (maxCor/(qtd-1))*i;	
            printf("qtd = %d, maxcor = %d intervalo %d\n", qtd, maxCor, intervalo[i]);		
	}

	for(j=0; j < altura; j++){
		for(i=0; i < largura; i++){
			int aux = maxCor;
			int flag = -1;
			
			for(k=0; k < qtd; k++){
				if(abs(data[j][i]-intervalo[k]) < aux){
					flag = k;
					aux = abs(data[j][i]-intervalo[k]);
				}
			}
			matriz[j][i] = intervalo[flag];
		}
	}
	
	return matriz;
}

unsigned char **dithering(unsigned char **data, int altura, int largura, int maxCor){
	int i,j;
	unsigned char **r, **matriz, antigo;
	double erro;

	matriz = alocaMatriz(altura, largura);
	r = reducaoCores(data, altura, largura, maxCor);
	
	if(r==NULL){
		desalocaMatriz(matriz, altura);
		return NULL;
	}

	for(i=0; i<altura-1; i++){
		for(j=0; j<largura-1; j++){
			antigo = data[i][j];
			matriz[i][j] = r[i][j];
			erro = antigo - matriz[i][j];

			double buf = (7.0/16.0) * erro;		
			matriz[i][j+1] = matriz[i][j+1] + buf;
	
			if (j!=0){
				buf=(3.0/16.0) * erro;
				matriz[i+1][j-1] = matriz[i+1][j-1] + buf;
			}
			
			buf=(5.0/16.0) * erro;
			matriz[i+1][j] = matriz[i+1][j] + buf;
			
			buf = ((1.0/16.0) * erro);
			matriz[i+1][j+1] = matriz[i+1][j+1] + buf;
		}
	}
	
	return matriz;
}

/*metodo principal, gerencia a janela*/
int main(int argc, char argv[]) {	
    
    ///TESTE//
    head = (struct Nodo*)malloc (sizeof(struct Nodo));    
    int undo=0, redo=0;
    
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
	int maxCor = 0;//inteiro que representa tonalidade de cor maxima

	/*sinaliza fechamento da janela*/
	bool fechaJanela = false;
	
	/*sinaliza existencia de arquivo aberto*/
	bool arquivoAberto = false;
 
    Nodo *temp;
	
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
	
    bL = btL * QTD_BT_FILEIRA;
	bA = btA * (1+(QTD_BT/QTD_BT_FILEIRA));

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
	int i;	
	for(i=0; i< QTD_BT; i++)
		botoes[i].ativo=false;	
	
	botoes[0].ativo=true;

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
			printf("Pressionou botao %d\n", verificaClique(evento.mouse.x, evento.mouse.y, botoes));
			
			Nodo *temp;
			
			switch (verificaClique(evento.mouse.x, evento.mouse.y,botoes)) {
				
				case 0:/*carrega e desenha a imagem*/
					if(botoes[0].ativo==true) {	
						/*caso algum arquivo estiver aberto, limpa dados*/
						if(arquivoAberto==true) {
							desalocaMatriz(head->data, head->altura);
							head->altura =0;
							head->largura =0;
							maxCor =0;
							arquivoAberto = false;
							undo=0;
							redo=0;
							botoes[4].ativo=false;
	                        botoes[5].ativo=false;
						}
						if(arquivoAberto==false) {
							/*carrega imagem na matriz*/
							int resposta = carregaImagem (janela, &maxCor, tipo, head);
							if(resposta <0) {
								if(resposta == -1)//caso reposta seja -2, a mesangem de erro ja foi tratada pelo metodo.
									al_show_native_message_box(janela, "Erro", "Erro ao abrir imagem.", erroMsgBuf, NULL, ALLEGRO_MESSAGEBOX_ERROR);
						
								printf("Erro ao desenhar imagem!\n");
					
								/*desativa botoes*/
								botoes[1].ativo=false;
								botoes[2].ativo=false;
								botoes[3].ativo=false;
								botoes[4].ativo=false;
	                            botoes[5].ativo=false;
	                            botoes[6].ativo=false;
								botoes[7].ativo=false;
								
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
							botoes[1].ativo=true;
							botoes[2].ativo=true;
							botoes[3].ativo=true;
                            botoes[6].ativo=true;
							botoes[7].ativo=true;
                            
							/*desenha a imagem na janela, 0 e bA sao as coordenadas x,y de onde vai comeca desenhar a imagem
							sendo que bA representa a altura do menu*/
							desenha(janela, head->data, head->altura, head->largura, (bL - head->largura)/2, bA);
							
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
						int resposta = gravaImagem(janela, tipo, head->altura, head->largura, maxCor, head->data);
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
						InserirNodo(head->data, head->largura, head->altura);
                        undo++;
						redo=0;
						head->data = rotacao(head->prox->data, &head->altura, &head->largura, 'D');
						desenha(janela, head->data, head->altura, head->largura, (bL - head->largura)/2, bA);
						
						botoes[1].ativo=true;
						botoes[4].ativo=true;
						botoes[5].ativo = false;
						desenhaMenu(janela, botoes);
						al_flip_display();
					}
					else {
						printf("Botao inativo\n");
					}
				break;

				case 3:/*gira anti-horario*/
					if(botoes[3].ativo==true) {
						InserirNodo(head->data, head->largura, head->altura);
                        undo++;
						redo=0;
                        head->data = rotacao(head->prox->data, &head->altura, &head->largura, 'E');
						desenha(janela, head->data, head->altura, head->largura, (bL - head->largura)/2, bA);
						
						botoes[1].ativo=true;
						botoes[4].ativo=true;
						botoes[5].ativo = false;
						desenhaMenu(janela, botoes);
						al_flip_display();
					}
					else {
						printf("Botao inativo\n");
					}
				break;
				
				case 4:/*Undo*/
					if(botoes[4].ativo==true) {
						
						
                        head = head->prox;
                        desenha(janela, head->data, head->altura, head->largura, (bL - head->largura)/2, bA);
                             
					     undo--;
						 redo++;

						if(undo==0)
							botoes[4].ativo=false;
    						botoes[1].ativo=true;
    						botoes[5].ativo=true;
						desenhaMenu(janela, botoes);
						al_flip_display();
					}
					else {
						printf("Botao inativo\n");
					}
				break;

				case 5:/*Redo*/
					if(botoes[5].ativo==true) {
						
						
						head = head->ante;
                        desenha(janela, head->data, head->altura, head->largura, (bL - head->largura)/2, bA);

					     redo--;
                        undo++;
						if(redo==0)
                            botoes[5].ativo=false;
                            
						botoes[1].ativo=true;
						botoes[4].ativo=true;
						desenhaMenu(janela, botoes);
						al_flip_display();
					}
					else {
						printf("Botao inativo\n");
					}
				break;
				
				case 6:                     
					if(botoes[6].ativo==true){
                        InserirNodo(head->data, head->largura, head->altura);
                        head->data = reducaoCores(head->prox->data, head->prox->altura, head->prox->largura,  maxCor);
						if(head->data==NULL){
							al_show_native_message_box(janela, "Erro", "Erro ao carregar arquivo de configuracao de cores.", erroMsgBuf, NULL, ALLEGRO_MESSAGEBOX_ERROR);
							head = head->prox;
							free(head->ante);
							head->ante=NULL;
                            break;
						}
				        undo++;
                        redo=0;
						botoes[1].ativo=true;
                        botoes[4].ativo=true;
						botoes[5].ativo = false;
                        desenha(janela, head->data, head->altura, head->largura, (bL - head->largura)/2, bA);
                        desenhaMenu(janela, botoes);
						al_flip_display();	
                    }
                    else {
                         printf("Botao inativo\n");
                    }
	            break;
	            
				case 7:
					if(botoes[7].ativo==true){
						InserirNodo(head->data, head->largura, head->altura);
                        head->data = dithering(head->prox->data, head->prox->altura, head->prox->largura,  maxCor);
						if(head->data==NULL){
							al_show_native_message_box(janela, "Erro", "Erro dithering", erroMsgBuf, NULL, ALLEGRO_MESSAGEBOX_ERROR);
							head = head->prox;
							free(head->ante);
							head->ante=NULL;
                            break;
						}
				        undo++;
                        redo=0;
						botoes[1].ativo=true;
                        botoes[4].ativo=true;
						botoes[5].ativo = false;
                        desenha(janela, head->data, head->altura, head->largura, (bL - head->largura)/2, bA);
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
	if(head->data!=NULL && arquivoAberto == true) {
		desalocaMatriz(head->data, head->altura);
	}
	
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

