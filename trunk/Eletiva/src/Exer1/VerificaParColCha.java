package Exer1;

public class VerificaParColCha extends AbstractPilha
{
	private String texto;
	
	public VerificaParColCha(String texto) {
		super(texto.length());
		this.texto = texto;
	}
	
	private void preenchePilha()
	{
		for(int i=0; i< this.texto.length(); i++)
		{
			switch (texto.charAt(i))
			{
				case '(':
					insere('(');
					break;
				
				case '[':
					insere('[');
					break;
				
				case '{':
					insere('{');
					break;
				
				default:
					break;
			}
		}
	}
	
	public boolean isValido()
	{
		preenchePilha();
		for(int i=0; i< this.texto.length(); i++)
		{
			switch (texto.charAt(i))
			{
				case ')':
					if(getDado() == '(')
					{
						remove();
						break;
					}
					else
						return false;
				
				case ']':
					if(getDado() == '[')
					{
						remove();
						break;
					}
					else
						return false;
				
				case '}':
					if(getDado() == '{')
					{
						remove();
						break;
					}
					else
						return false;
				
				default:
					break;
			}
		}
		
		if(isVazio())
			return true;
		else
			return false;
	}
}
