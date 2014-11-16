package Exer1;

public class executa {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		VerificaParColCha verifica = new VerificaParColCha("aaaa[dawdaw(dwad{aaaaa})]");
		
		if(verifica.isValido())
		{
			System.out.println("CADEIA VÁLIDA!");
		}
		else
			System.out.println("CADEIA INVALIDA!");

	}

}
