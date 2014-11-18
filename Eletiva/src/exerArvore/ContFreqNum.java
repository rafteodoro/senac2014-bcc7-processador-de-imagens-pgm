package exerArvore;

import java.util.Scanner;

import common.GravaTxt;
import common.LeitorTxt;
import arvore.Arvore;
import arvore.No;

public class ContFreqNum {
	private Arvore arvore;
	private LeitorTxt leitor;
	private GravaTxt grava;
	private final String DIRETORIO_ENTRADA = "entrada.txt";
	private final String DIRETORIO_SAIDA = "saida.txt";
	private int numeros [];
	private int qtdNum;
	
	public static void main(String[] args) {
		ContFreqNum main = new ContFreqNum(); 
		main.getNumeros();
		main.preencheArvore();
		main.gravaArvore();
	}
	
	private void getNumeros() {
		leitor = new LeitorTxt(DIRETORIO_ENTRADA);
		Scanner in = new Scanner(leitor.leTexto());
		numeros = new int[leitor.getTexto().length()];
		
		for(int i=0; i<numeros.length; i++) {
			numeros[i] = in.nextInt();
			if(numeros[i] == 0) {
				qtdNum = i;
				break;
			}
		}
	}
	
	public void preencheArvore() {
		arvore = new Arvore(numeros[0]);
		for(int i=1; i<qtdNum; i++) {
			if(!arvore.existeElemen(arvore.getRaiz(), numeros[i])){
				No novoNo = new No();
				novoNo.setDado(numeros[i]);
				arvore.inserir(novoNo, arvore.getRaiz());
			}
		}
		System.out.println("ARVORE PREENCHIDA");
		
		arvore.rotacaoD(arvore.getRaiz().getEsq().getEsq());
	}
	
	
	public void gravaArvore() {
		StringBuffer arvoreString = new StringBuffer(arvore.toString());
		grava = new GravaTxt(DIRETORIO_SAIDA, arvoreString);
		grava.gravaTexto();
		System.out.println("ARVORE GRAVADA:" + DIRETORIO_SAIDA);
	}
}
