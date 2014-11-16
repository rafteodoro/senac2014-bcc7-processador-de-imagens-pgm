package common;

public abstract class AbstractIoTxt {
	protected StringBuffer texto;
	protected String diretorio;
	
	AbstractIoTxt(String diretorio) {
		this.diretorio = diretorio;
		texto = new StringBuffer();
	}

	public String getDiretorio() {
		return diretorio;
	}

	public void setDiretorio(String diretorio) {
		this.diretorio = diretorio;
	}

	public StringBuffer getTexto() {
		return texto;
	}

	public void setTexto(StringBuffer texto) {
		this.texto = texto;
	}
}
