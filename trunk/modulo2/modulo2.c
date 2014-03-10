#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>
 
 
typedef struct Botao{
        int x, y;
        ALLEGRO_BITMAP *icone;
}Botao;


Botao botoes[4];

const int NUMVERSOES = 1;
const char VERSOES[1] = {'5'};
char tipo[2];//tipo de PGM
char *erroMsgBuf;//mesagem de erro gerada pelo metodo

/*Cria uma interface grafica nativa para o usuario selecionar um arquivo
@param
	ALLEGRO_DISPLAY *janela - janela onde ira mostrar a interface 
*/
char * uiGetDiretorio(ALLEGRO_DISPLAY *janela, const char *msg, int flag){
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
FILE *abreArquivo(char *tp, ALLEGRO_DISPLAY *janela){
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
	int l - largura da matriz (qtd de colunas)
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
int getCabecalho(FILE *arquivo, int *altura, int *largura, int *maxCor, char *tipo)
{
	
	int i, buf = 0;

	/*Verifica 'P' no inicio do arquivo*/
	buf = getc(arquivo);
	if(buf == 'P')
		tipo[0] = buf;
	else
	{
		printf("erro, arquivo PMG invalido\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return -1;
	}

	/*Verifica versão da imagem*/
	buf = getc(arquivo);
	for(i=0; i< NUMVERSOES; i++)
	{
		if(buf==VERSOES[i])
			break;	
	}
	if(i < NUMVERSOES)
	{
		tipo[1] = buf;
	}
	else
	{
		printf("erro, versao invalida ou nao suportada!\n");
		erroMsgBuf = "Versao invalida ou nao suportada.";
		return -1;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n')
	{
		printf("cabecalho invalido\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return -1;
	}

	/*verifica largura*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		*largura = buf;
	else
	{
		printf("erro ao pegar largura da imagem!\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return 0;
	}

	/*verifica altura*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		*altura = buf;
	else
	{
		printf("erro ao pegar altura da imagem!\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return 0;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n')
	{
		printf("cabecalho invalido\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return -1;
	}

	/*verifica maxCor*/
	fscanf(arquivo, "%d", &buf); 
	if(buf>=0)
		*maxCor = buf;
	else
	{
		printf("erro ao pegar maxCor!\n");
		erroMsgBuf = "Arquivo PMG invalido.";
		return 0;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n')
	{
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
	int i, j, buf;//buf do tipo int para detectar o EOF
	
	for(i=0; i<a; i++) {
		for(j=0; j<l; j++) {
			buf = getc(arquivo);
			if(buf > m || buf < 0 || buf == EOF) {
				printf("Dados inconsistentes!\n");
				return -1;
			}
			else {
				data[i][j] = buf;
			}
		}
	}
}

/*Altera o tamanho da janela
@param
	ALLEGRO_DISPLAY *janela - janela onde ira desenhar a imagem
	int h - altura da imagem
	itn w - largura da imagem
*/
void resize(ALLEGRO_DISPLAY *janela, int h, int w) {
    al_resize_display(janela, w, h);
	al_clear_to_color(al_map_rgb(255, 255, 255));
	al_flip_display();	
}

/*Desenha a imagem na janela
@param
	ALLEGRO_DISPLAY *janela - janela onde ira desenhar a imagem
	unsigned char ** data - m
    atriz onde sera armazenado os bits
	int a - altura da imagem
	itn l - largura da imagem
*/
void desenha(ALLEGRO_DISPLAY *janela, unsigned char **data, int a, int l) {	
    /*reajusta o tamanho da janela para a imagem aberta*/
    resize(janela, a+50, l);
    
    int i, j;
    for (i=0; i<a; i++) {
        for(j=0; j<l; j++) {
            al_draw_pixel(j, i+50, al_map_rgb(data[i][j],data[i][j],data[i][j]));
        }
    }
    al_flip_display();
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
	if(arquivo==NULL) return -1;

	/*pega tamanho e tipo da imagem(cabecalho)*/
	if(getCabecalho(arquivo, altura, largura, maxCor, tipo)==-1) {
		return -1;
	}

	if(strcmp(tipo,"P5")!=0) {
		printf("Versao da imagem PGM invalida ou nao suportada!\n");
		return -1;
	}

	/*aloca memoria para a matriz*/
	*data = alocaMatriz(*altura, *largura);

	/*armazena os bits na matriz*/
	if(armazenaDados(arquivo, *data, *altura, *largura, *maxCor)==-1) {
		return -1;
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
	sprintf(buf, "%s\n%d %d\n%d", tipo, largura, altura, maxCor);
	
	if(fputs(buf,out)==EOF) {
		printf("Erro ao gravar cabecalho");
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
    if(out==NULL) 
        return 0;

	if(putCabecalho(out, tipo, altura, largura, maxCor)==-1)
        return-1;

	if(gravaDados(out, data,altura,largura,maxCor)==-1)
        return-1;

	fclose(out);
	printf("Imagem Salva!\n");
	return 0;
}

/*log - imprime dados da matriz no console*/ 
void imprime(unsigned char **data, int a, int l) {
    int i, j;
	for(i=0; i<a; i++) {
		for(j=0; j<l; j++) {
			printf(" %d,%d = %d ",i, j, data[i][j]);
		}
	}
}

unsigned char **rotacao(unsigned char **data, int *altura, int *largura, char sentido) {
    unsigned char **matriz = alocaMatriz(*largura, *altura);         
    int i,j;
    int aux = *largura-1;
    
    printf("%d %d\n", *altura, *largura);
    if(sentido == 'E') {
        for(i=0; i<*largura; i++){
            for(j=0; j<*altura; j++){
                matriz[i][j] = data[j][aux];
            }
            aux--;
        }
    } 
    else { 
         aux = *altura-1;
        for(i=0; i<*altura; i++){
            for(j=0; j<*largura; j++){
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

/*metodo principal, gerencia a janela*/
int main(int argc, char argv[]) {	
	/*janela principal*/
    ALLEGRO_DISPLAY *janela = NULL;

	/*fila de eventos*/
	ALLEGRO_EVENT_QUEUE *filaEvento = NULL;

	/*evento atual*/
	ALLEGRO_EVENT evento;
	
	ALLEGRO_BITMAP *botao_gravar = NULL, *botao_carregar = NULL, *botao_girar_horario = NULL, *botao_girar_antihorario = NULL;
	
	/*ponteiro para um arquivo no dico*/
	FILE *arquivo;
	
	/*definicoes do arquivo*/
	int altura = 0,//altura da imagem
		largura = 0, //largura da imagem
		maxCor = 0;//inteiro que representa tonalidade de cor maxima
	
	char tipo[3];//tipo de PGM
	
	/*estrutura para armazenar os bits do arquivo*/
	unsigned char **data = NULL;
	
	/*sinaliza fechamento da janela*/
	bool fechaJanela = false;
	
	/*sinaliza existencia de arquivo aberto*/
	bool arquivoAberto = false;
 
    /*inicializa bibliotecas*/
    if(!(al_init() && 
		al_install_mouse()&&
		al_init_image_addon()&&
		al_install_keyboard()&&
		al_init_primitives_addon()&&
		(filaEvento = al_create_event_queue()))){
        printf("Erro ao inicializar biblioteca(s)!\n"); 
        return 0;
    }

 // Alocamos o botão para fechar a aplicação
    botao_carregar = al_load_bitmap("carregar.bmp"); 
    
    if (!botao_carregar)
    {
        fprintf(stderr, "Falha ao criar botão de carregar a imagem.\n");
        al_destroy_display(janela);
        
        return -1; 
    }
    
        // Alocamos o botão para fechar a aplicação
    botao_gravar = al_load_bitmap("salvar.bmp");
    if (!botao_gravar)
    {
        fprintf(stderr, "Falha ao criar botão de salvar a imagem.\n");
        al_destroy_bitmap(botao_carregar);
        al_destroy_display(janela);
        return -1;
    }
        // Alocamos o botão para fechar a aplicação
    botao_girar_horario = al_load_bitmap("girar_horario.bmp");
    if (!botao_girar_horario)
    {
        fprintf(stderr, "Falha ao criar botão de girar a imagem 90º sentido horario.\n");
       al_destroy_bitmap(botao_carregar);
        al_destroy_bitmap(botao_gravar);
        al_destroy_display(janela);
        return -1;
    }
        // Alocamos o botão para fechar a aplicação
    botao_girar_antihorario = al_load_bitmap("girar_anti.bmp");
    if (!botao_girar_antihorario)
    {
        fprintf(stderr, "Falha ao criar botão de girar a imagem 90º sentido anti-horario.\n");
        al_destroy_bitmap(botao_carregar);
        al_destroy_bitmap(botao_gravar);
        al_destroy_bitmap(botao_girar_horario);
        al_destroy_display(janela);
        return -1;
    }

    /*cria uma janela 640x480*/
    janela = al_create_display(640, 50);

    /*registra os eventos*/
	al_register_event_source(filaEvento, al_get_display_event_source(janela));
	al_register_event_source(filaEvento, al_get_mouse_event_source());
	al_register_event_source(filaEvento, al_get_keyboard_event_source());

	/*preenche a janela com a cor branca*/
    al_clear_to_color(al_map_rgb(255, 255, 255));         

	/*atualiza tela*/
    al_flip_display();
    
	/*fluxo principal da janela*/
	while(fechaJanela == false){
		
        /*pega evento da fila*/
		al_wait_for_event(filaEvento, &evento);

		switch (evento.type) {			
			/*fecha a janela (termina aplicacao)*/
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				fechaJanela = true;
			break;
			
			/*carrega imagem em mostra na tela*/
			case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
          
           if (evento.mouse.x >= 0 && evento.mouse.x <= 70 && evento.mouse.y <= 50 && evento.mouse.y >= 0) {
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
                        if(carregaImagem (janela, &altura,&largura, &maxCor, tipo, &data)==-1) {
                            printf("Erro ao desenhar imagem!\n");
                            break;
                        }

					/*sinaliza como arquivo aberto*/
					arquivoAberto = true;

					/*desenha a imagem na janela*/
					desenha(janela, data, altura, largura);
					
					 
				}	
            }
            
            
            if (evento.mouse.x >= 71 && evento.mouse.x <= 140 && evento.mouse.y <= 50 && evento.mouse.y >= 0 && arquivoAberto== true){
                if(arquivoAberto == true) {
					if(gravaImagem(janela, tipo, altura, largura, maxCor, data)==-1) {
						printf("Erro ao salvar imagem!\n");
						break;
					}
				}
				else {
					printf("Nenhum arquivo aberto!\n");			
				}
            }
            
             if (evento.mouse.x >= 141 && evento.mouse.x <= 210 && evento.mouse.y <= 50 && evento.mouse.y >= 0 && arquivoAberto== true)
             {
                 data = rotacao(data, &altura, &largura, 'D');
                 desenha(janela, data, altura, largura);
             }
            
             if (evento.mouse.x >= 211 && evento.mouse.x <= 280 && evento.mouse.y <= 50 && evento.mouse.y >= 0 && arquivoAberto== true){
                                  data = rotacao(data, &altura, &largura, 'E');
                 desenha(janela, data, altura, largura);
                                }
            
			break;
			
		
         case ALLEGRO_EVENT_KEY_DOWN:
		 switch(evento.keyboard.keycode) 
        {
            case ALLEGRO_KEY_ENTER:
               if(arquivoAberto == true) {
					if(gravaImagem(janela, tipo, altura, largura, maxCor, data)==-1) {
						printf("Erro ao salvar imagem!\n");
						break;
					}
				}
				else {
					printf("Nenhum arquivo aberto!\n");			
				}
               break;
 
            case ALLEGRO_KEY_SPACE:
                 data = rotacao(data, &altura, &largura, 'D');
                 desenha(janela, data, altura, largura);
  
            break;

         }
		
            default:
			break;
			
		}
		
		al_set_target_bitmap(botao_gravar);
        al_set_target_bitmap(botao_carregar);
        al_set_target_bitmap(botao_girar_horario);
        al_set_target_bitmap(botao_girar_antihorario);
    
        al_set_target_bitmap(al_get_backbuffer(janela));
        al_draw_bitmap(botao_carregar, 0,0, 0);
        al_draw_bitmap(botao_gravar, 71,0, 0);
        al_draw_bitmap(botao_girar_horario, 141,0, 0);
        al_draw_bitmap(botao_girar_antihorario, 211,0, 0);
    
    	/*atualiza tela*/
        al_flip_display();
	}

	/*limpeza*/
	if(data!=NULL && arquivoAberto == true){
		desalocaMatriz(data, altura);
	}
    al_destroy_event_queue(filaEvento);
	al_uninstall_mouse();
    al_uninstall_keyboard();
    al_destroy_bitmap(botao_carregar);
    al_destroy_bitmap(botao_gravar);
    al_destroy_bitmap(botao_girar_horario);
    al_destroy_bitmap(botao_girar_antihorario);
	al_destroy_display(janela);

    return 0;
}
