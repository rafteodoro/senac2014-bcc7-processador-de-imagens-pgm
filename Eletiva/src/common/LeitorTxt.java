package common;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

public class LeitorTxt extends AbstractIoTxt {

	public LeitorTxt(String diretorio) {
		super(diretorio);
		// TODO Auto-generated constructor stub
	}
	
	private FileReader arquivo;
	protected BufferedReader buf;
	
	public String leTexto() {
		try {
			arquivo = new FileReader(diretorio);
			buf = new BufferedReader(arquivo);
			while(buf.ready())
			{
				texto.append(buf.readLine());
			}
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			System.out.println("ERRO! Nao foi possivel abrir o arquivo " + diretorio);
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("ERRO! Nao foi possivel ler o arquivo " + diretorio);
			e.printStackTrace();
		}
		return texto.toString();
	}
}
