#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _OPENMP 
#include <omp.h>
#else if 
#define omp_get_wtime( ) 0 
#define omp_get_num_threads() 0
#endif

int threadLigado;

typedef struct numComplexo {
    float real;
    float imag;
} Comp;

Comp **matrizC;

typedef struct imagem{
        int altura;
        int largura;
        unsigned char **data;
} Img;

FILE *abreArquivo(const char *diretorio, const char *tipo){
     FILE *arquivo;
     if(!(arquivo = fopen(diretorio, tipo))) {
          printf("Arquivo '%s' nao pode ser aberto\n",diretorio);
		  return NULL;
     }
     else
     {
         return arquivo;          
     }   
}

unsigned char **alocaMatrizChar(int a, int l) {
	int i;
	unsigned char **data = malloc (sizeof(unsigned char *) * a);
	for(i=0; i<a; i++){
		data[i] = malloc(sizeof(unsigned char) * l);
	}

	return data;
}

double **alocaMatrizDouble(int a, int l) {
	int i;
	double **data = malloc(sizeof(double *) * a);
	for(i=0; i<a; i++){
		data[i] = malloc(sizeof(double) * l);
	}

	return data;
}

Comp **alocaMatrizComp(int a, int l) {
	int i;
	Comp **data = malloc(sizeof(Comp *) * a);
	for(i=0; i<a; i++){
		data[i] = malloc(sizeof(Comp) * l);
	}
	return data;
}

void desalocaMatrizChar(unsigned char **data, int a) {
    int i;
	for(i=0; i<a; i++){
        if(data[i]){
            free(data[i]);
        }
    }
	free(data);
}

void desalocaMatrizDouble(double **data, int a) {
    int i;
	for(i=0; i<a; i++){
        if(data[i]){
            free(data[i]);
        }
    }
	free(data);
}

void desalocaMatrizComp(Comp **data, int a) {
    int i;
	for(i=0; i<a; i++){
        if(data[i]){
            free(data[i]);
        }
    }
	free(data);
}

void desalocaImg(Img *img){
     printf("DESALOCAIMG img->altura:%d img->largura:%d \n", img->altura, img->largura);
     if(!img){
         return;
     }
     if(img->data!=NULL){
         desalocaMatrizChar(img->data, img->altura);
     }
}

int getCabecalho(FILE *arquivo, int *altura, int *largura) {

	int i, buf = 0;

	/*Verifica 'P' no inicio do arquivo*/
	buf = getc(arquivo);
	if(buf != 'P'){
		printf("erro, arquivo PGM invalido\n");
		return -1;
	}

	/*Verifica versÃÂ³o da imagem*/
	buf = getc(arquivo);

	if(buf!='5') {
        printf("erro, versao invalida ou nao suportada!\n");
		return -1;	
	}
	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		return -1;
	}

	/*verifica largura*/
	fscanf(arquivo, "%d", &buf);
	if(buf>=0)
		*largura = buf;
	else {
		printf("erro ao pegar largura da imagem!\n");
		return -1;
	}

	/*verifica altura*/
	fscanf(arquivo, "%d", &buf);
	if(buf>=0)
		*altura = buf;
	else {
		printf("erro ao pegar altura da imagem!\n");
		return -1;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		return -1;
	}

	/*verifica maxCor*/
	fscanf(arquivo, "%d", &buf);
	if(buf!=255){
		printf("erro ao pegar maxCor!\n");
		return -1;
	}

	/*verifica quebra de linha*/
	buf = getc(arquivo);
	if(buf != '\n') {
		printf("cabecalho invalido\n");
		return -1;
	}

	printf("GETCABECALHO largura:%d altura:%d\n", *largura, *altura);
	return 0;
}

int armazenaDados(FILE * arquivo, unsigned char **data, int a, int l) {
	int i, j,
		buf;//buf do tipo int para detectar o EOF

	for(i=0; i<a; i++) {
		for(j=0; j<l; j++) {
			buf = getc(arquivo);
			if(buf > 255 || buf < 0 || buf == EOF) {
				printf("ALERTA:Dados da imagem inconsistentes!buf=%d, i=%d, j=%d\n",buf, i, j);
				return -1;
			}
			else {
				data[i][j] = buf;
			}
		}
	}
	buf = getc(arquivo);
	if(buf!=EOF) {
		printf("ALERTA:Dados da imagem inconsistentes!\n");
		return -1;
	}

	else return 0;
}

Img *obtemImagem(const char * diretorio)
{
    FILE *arquivo;
    Img *img;
    int l, a;
    
    arquivo = abreArquivo(diretorio, "r+b");
    if(arquivo==NULL){
         return NULL;            
    }
      
    if(getCabecalho(arquivo, &a, &l)!=0) {
        fclose(arquivo);
        return NULL;   
    }
    
    img = malloc ( sizeof(Img));
    if(img==NULL){
        return NULL;              
    }
    
    img->altura=a;
    img->largura=l;
    img->data = alocaMatrizChar(img->altura, img->largura);
    if(img->data==NULL) {
        fclose(arquivo);
        desalocaImg(img);
        return NULL;        
    }
    
    if(armazenaDados(arquivo, img->data, img->altura, img->largura)!=0){
        fclose(arquivo);
        desalocaImg(img);
        return NULL;                         
    }
    
    fclose(arquivo);
    printf("OBTEMIMAGEM imagem carregada!\n");
    return img;
}

int putCabecalho(FILE *out, int altura, int largura) {
	char buf[20];
	sprintf(buf, "%s\n%d %d\n%d\n", "P5", largura, altura, 255);

	if(fputs(buf,out)==EOF) {
		printf("Erro ao gravar cabecalho");
		return -1;
	}

	printf("PUTCABECALHO:\n%s\n",&buf);
	return 0;
}

int gravaDados(FILE *out, unsigned char **data, int altura, int largura){
	int i, j;
	for(i=0; i<altura; i++) {
		for(j=0; j<largura; j++){
			putc(data[i][j], out);
		}
	}
	return 0;
}

int gravaImagem(Img *img, const char *diretorio){
    FILE *arquivo;
    arquivo = abreArquivo(diretorio, "w+b");
    
    if(arquivo==NULL){
        return -1;                  
    }
    
    if(putCabecalho(arquivo, img->altura, img->largura)!=0){
        fclose(arquivo);
        return -1;
    }
    
    if(gravaDados(arquivo,img->data,img->altura,img->largura)!=0){
        fclose(arquivo);
        return -1;                                                              
    }
    
    fclose(arquivo);
    printf("GRAVAIMAGEM arquivo gravado: '%s'\n", diretorio);
    return 0;
}

unsigned char **calcFourier(unsigned char **data, int altura, int largura){
    unsigned char **M1;
    Comp **matrizL;
    double **matrizEspectro;
    double espectro, auxMaxEspectro=0, maxEspectro=0, calc=0, cosX, senX, somaReal, somaImag, start, end, tempTotal; 
    int i, j, k, qtdThead;
    
    M1 = alocaMatrizChar(altura, largura);
    matrizEspectro = alocaMatrizDouble(altura, largura);
    matrizL = alocaMatrizComp(altura, largura);
	matrizC = alocaMatrizComp(altura, largura);
    
    printf("CALCFOURIER Calculando...\n");
    
	start = omp_get_wtime( );
    printf("\nCALCFOURIER for 1...\n"); 
	
	//if(threadLigado==0)
//		omp_set_num_threads(1);
    #pragma omp parallel
    {
    	qtdThead = omp_get_num_threads();
    	#pragma omp for private (j, k, cosX, senX, somaReal, somaImag)
        for (i = 0; i < altura; i++){
            for (j = 0; j < largura; j++){
            	somaReal = 0.0;
                somaImag = 0.0;
				for (k = 0; k < largura; k++){
                    cosX = cos(((2.0 * M_PI)*(j * k)) / largura);
                    senX = sin(((2.0 * M_PI)*(j * k)) / largura);
                    somaReal += (data[i][k] * cosX) - (0.0 * senX);
                    somaImag += (0.0 * cosX)+(data[i][k] * senX) ;
                }
				matrizL[i][j].real = somaReal;
                matrizL[i][j].imag = somaImag;
            }
       
        }
	}
    end = omp_get_wtime( );
	printf("CALCFOURIER tempo de exec:%f Qtd threads:%d\n", end-start,  qtdThead);
	
	tempTotal = end-start;
	
	printf("\nCALCFOURIER for 2...\n");
	start = omp_get_wtime( );
	#pragma omp parallel
    {
       qtdThead = omp_get_num_threads();
	    #pragma omp for private (i, k, cosX, senX, somaReal, somaImag)
        for (j = 0; j < largura; j++){
            for (i = 0; i < altura; i++){
                somaReal = 0.0;
                somaImag = 0.0;
				for (k = 0; k < altura; k++){
                    cosX = cos(((2.0 * M_PI)*(i * k)) / altura);
                    senX = sin(((2.0 * M_PI)*(i * k)) / altura);
                    somaReal += (matrizL[k][j].real * cosX) - (matrizL[k][j].imag * senX);
                    somaImag += (matrizL[k][j].real * senX) + (matrizL[k][j].imag * cosX);
                }
                matrizC[i][j].real = somaReal;
                matrizC[i][j].imag = somaImag; 
            }
        }
    }
	end = omp_get_wtime( );
	printf("CALCFOURIER tempo de execucao:%f Qtd threads:%d\n\n", end-start, qtdThead);
	
	tempTotal += end-start;
	printf("CALCFOURIER tempo de execucao Total:%f\n", tempTotal);
	
    for (i = 0; i < altura; i++){
        for (j = 0; j < largura; j++){
            calc= (((matrizC[i][j].real) * (matrizC[i][j].real)) + ((matrizC[i][j].imag) * (matrizC[i][j].imag)));
            espectro = log(1+sqrt(calc));
            matrizEspectro[i][j] = espectro;
            calc=0.0;
        }
    }
    
    for (i = 0; i < altura; i++){
        for (j = 0; j < largura; j++){
            auxMaxEspectro = matrizEspectro[i][j];
            if (auxMaxEspectro > maxEspectro){
                maxEspectro = auxMaxEspectro;
            }
        }
    }
    
    for (i=0;i<altura;i++){
        for (j=0;j<largura;j++){
            M1[i][j] = round((matrizEspectro[i][j]/maxEspectro)*255);
        }
    }
    
    desalocaMatrizComp(matrizL, altura);

    desalocaMatrizDouble(matrizEspectro, altura);
 
    printf("CALCFOURIER calculo completo!\n");
    return M1;
}

int v(double x){
	if (x>255)
		return 255;
	else if (x<0)
		return 0;
	else
		return round(x);
}

unsigned char **calcInversaFourier(int altura, int largura){
	unsigned char **J;
    Comp **matrizL;
    Comp **matrizJ;
    double **matrizEspectro;
    double espectro, auxMaxEspectro=0, maxEspectro=0, calc=0, cosX, senX, somaReal, somaImag, start, end, tempTotal; 
    int i, j, k, qtdThead=0;
    
    J = alocaMatrizChar(altura, largura);
    matrizL = alocaMatrizComp(altura, largura);
    matrizJ = alocaMatrizComp(altura, largura);
    
    printf("CALCINVERSA Calculando...\n");
    printf("CALCINVERSA for 1...\n"); 
    
    //if(threadLigado==0)
//		omp_set_num_threads(1);
    
	start = omp_get_wtime( );
    #pragma omp parallel
    {
    	qtdThead = omp_get_num_threads();     
        #pragma omp for private (j, k, cosX, senX, somaReal, somaImag)
        for (i = 0; i < altura; i++){
            for (j = 0; j < largura; j++){
            	somaReal = 0.0;
                somaImag = 0.0;
                for (k = 0; k < largura; k++){
                    cosX = cos(((2.0 * M_PI)*(j * k)) / largura);
                    senX = sin(((2.0 * M_PI)*(j * k)) / largura);
                    somaReal += (matrizC[i][k].real * cosX) - (matrizC[i][k].imag * senX);
                    somaImag += (matrizC[i][k].real * senX) + (matrizC[i][k].imag * cosX) ;
                }
                
                matrizL[i][j].real = (somaReal/largura);
                matrizL[i][j].imag = somaImag/largura;

            }
        }
    }
    end = omp_get_wtime( );
	printf("CALCINVERSA tempo de execucao:%f Qtd threads:%d\n\n", end-start, qtdThead);
	tempTotal = end-start;
        
		//ANDREEEEEEEEEE
        //NESSA PARTE TEM QUE RECEBER O MATRIZC DO FOURIER NORMAL PRA CALCULAR O MATRIZL ACIMA!
        
    printf("CALCINVERSA for 2...\n");
    start = omp_get_wtime( );
    #pragma omp parallel
    {
        qtdThead = omp_get_num_threads();
		#pragma omp for private (i, k, cosX, senX, somaReal, somaImag)
        for (j = 0; j < largura; j++){
            for (i = 0; i < altura; i++){
            	somaReal = 0.0;
                somaImag = 0.0;
                for (k = 0; k < altura; k++){
                    cosX = cos(((2.0 * M_PI)*(i * k)) / altura);
                    senX = sin(((2.0 * M_PI)*(i * k)) / altura);
                    somaReal += (matrizL[k][j].real * cosX) + (matrizL[k][j].imag * senX);
                    somaImag += (matrizL[k][j].real * senX) - (matrizL[k][j].imag * cosX);
                }
                matrizJ[i][j].real = v(somaReal/altura);
                matrizJ[i][j].imag = somaImag;
            }
        }   
    }
    end = omp_get_wtime( );
    printf("CALCINVERSA tempo de execucao:%f Qtd threads:%d\n\n", end-start, qtdThead);
	
	tempTotal += end-start;
	printf("CALCINVERSA tempo de execucao Total:%f\n", tempTotal);
    
    
    for(i = 0;i < altura; i++){
    	for(j = 0;j < largura; j++){
    		J[i][j]=matrizJ[i][j].real;
    	}
    }
    
    desalocaMatrizComp(matrizL, altura);
    desalocaMatrizComp(matrizJ, altura);
 
    printf("CALCINVERSA calculo completo!\n");
    return J;
}


Img *fourier(Img *img){
    Img *imgF;
    
    imgF = malloc (sizeof(Img));
    if(imgF==NULL){
        return NULL;              
    }
    
    imgF->altura = img->altura;
    imgF->largura = img->largura;
    
    imgF->data = calcFourier(img->data, img->altura, img->largura);
    if(!imgF->data){
        return NULL;
    }
    return imgF;
}

Img *invFourier(Img *img){
    Img *imgF;
    
    imgF = malloc (sizeof(Img));
    if(imgF==NULL){
        return NULL;              
    }
    
    imgF->altura = img->altura;
    imgF->largura = img->largura;
    
    imgF->data = calcInversaFourier(img->altura, img->largura);
    if(!imgF->data){
        return NULL;
    }
    return imgF;
}

int main(int argc, char argv[]){
    const char *imagemEntrada = "imagem04.pgm";
    const char *imagemSaida = "espectro.pgm";
    const char *imagemFinal = "saida.pgm";
    Img *img, *imgF, *imgFinal;
    int op=0;
    printf("\nSelecione o modo de execucao: \n\n 1 - Sequencial \n 2 - Paralelo \n");
    scanf("%d",&op);
    
    if(op==2)
	{
		threadLigado=1;
	}
	else{
		threadLigado=0;
	}
		
    img = obtemImagem(imagemEntrada);
    if(img == NULL){
        system("pause");
        exit(0); 
    }   
    
    imgF = fourier(img);
    if(!imgF){
        desalocaImg(img);
        desalocaImg(imgF);
        system("pause");
        exit(0); 
    }
    
    gravaImagem(imgF, imagemSaida);
    
    imgFinal = invFourier(imgF);
    if(!imgFinal){
        desalocaImg(imgF);
        desalocaImg(imgFinal);
        system("pause");
        exit(0); 
    }

    gravaImagem(imgFinal, imagemFinal);
    
    desalocaMatrizComp(matrizC,imgF->altura);
    desalocaImg(imgFinal);
    desalocaImg(imgF);
    desalocaImg(img);
    system("pause");
    return 0;
}
