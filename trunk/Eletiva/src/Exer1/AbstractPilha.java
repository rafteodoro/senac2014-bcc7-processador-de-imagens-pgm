package Exer1;
public class AbstractPilha
{
	
	private char []pilha;
	private final char caracterBase = '#';
	private final char caracterTopo = '$';
	
	AbstractPilha(int tamanho)
	{
		pilha = new char[tamanho];
		pilha[0] = getCaracterBase();
		pilha[1] = getCaracterTopo();
	}
	
	public void insere(char dado)
	{
		int topo = encontraTopo();
		if(!isCheio())
		{
			this.pilha[topo] = dado;
			this.pilha[topo+1] = getCaracterTopo();
		}
		else
		{
			System.out.println("Erro ao inserir! A Pilha esta cheia!");
		}
	}
	
	public void remove()
	{
		int topo = encontraTopo();
		if(!isVazio())
		{
			this.pilha[topo] = ' ';
			this.pilha[topo-1] = getCaracterTopo();
		}
		else 
		{
			System.out.println("Nao ha nada para remover");
		}
		
	}
	
	public boolean isVazio()
	{
		if(encontraTopo() == 1)
			return true;
		else
			return false;
	}
	
	public boolean isCheio()
	{
		if(encontraTopo() == this.pilha.length)
			return true;
		else
			return false;
	}
	
	private int encontraTopo()
	{
		for(int i=1; i<this.pilha.length; i++)
		{
			if(this.pilha[i] == getCaracterTopo())
			{
				return i;
			}
		}
		System.out.println("ERRO, TOPO NAO ENCONTRADO");
		return pilha.length - 1;
	}
	
	public char getDado()
	{
		if(!isVazio())
			return pilha[encontraTopo()-1];
		else
			return ' ';
	}

	public char[] getPilha() {
		return pilha;
	}

	public void setPilha(char[] pilha) {
		this.pilha = pilha;
	}

	public char getCaracterBase() {
		return caracterBase;
	}

	public char getCaracterTopo() {
		return caracterTopo;
	}
}
