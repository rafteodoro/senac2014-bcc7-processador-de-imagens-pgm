package arvore;

public class No {
	private No esq;
	private No dir;
	private No pai;
	private int dado;
	private int ocorrencia;
	private int n_elemento;
	
	public No getEsq() {
		return esq;
	}
	
	public void setEsq(No esq) {
		this.esq = esq;
	}
	
	public No getDir() {
		return dir;
	}
	
	public void setDir(No dir) {
		this.dir = dir;
	}

	public int getDado() {
		return dado;
	}

	public void setDado(int dado) {
		this.dado = dado;
		this.ocorrencia = 1;
	}

	public int getOcorrencia() {
		return ocorrencia;
	}

	public void setOcorrencia(int ocorrencia) {
		this.ocorrencia = ocorrencia;
	}

	public int getN_elemento() {
		return n_elemento;
	}

	public void setN_elemento(int n_elemento) {
		this.n_elemento = n_elemento;
	}

	public No getPai() {
		return pai;
	}

	public void setPai(No pai) {
		this.pai = pai;
	}
	
	
	
}

