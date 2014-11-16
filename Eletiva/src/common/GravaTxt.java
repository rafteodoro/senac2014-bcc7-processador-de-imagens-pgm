package common;

import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

public class GravaTxt extends AbstractIoTxt {

	public GravaTxt(String diretorio, StringBuffer texto) {
		super(diretorio);
		this.texto = texto;
		// TODO Auto-generated constructor stub
	}
	
	private FileWriter  arquivo;
	private PrintWriter buf;
	
	public void gravaTexto() {
		try {
			arquivo = new FileWriter(diretorio);
			buf = new PrintWriter(arquivo);
				buf.write(texto.toString());
			arquivo.close();	
		}  
		catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("ERRO! Nao foi possivel Gravar o arquivo " + diretorio);
			e.printStackTrace();
		}
	}
}

