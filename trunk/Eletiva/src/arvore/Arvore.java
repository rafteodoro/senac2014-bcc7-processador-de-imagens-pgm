package arvore;

public class Arvore 
{
	private No raiz;
	
	public Arvore (int dado)
	{
		raiz = new No();
		raiz.setDado(dado);
	}
	
	public No getRaiz() {
		return raiz;
	}

	public void setRaiz(No raiz) {
		this.raiz = raiz;
	}
	
	public void rotacaoD (No no)
	{
		No temp = no.getPai();
		
		no.getPai().getPai().setEsq(no);
		no.setPai(no.getPai().getPai());
		no.setN_elemento(no.getN_elemento() - 1);
		no.getEsq().setN_elemento(no.getEsq().getN_elemento() - 1);
		
		temp.setEsq(no.getDir());
		no.getDir().setPai(temp);
		
		no.setDir(temp);
		temp.setPai(no);
		temp.setN_elemento(temp.getN_elemento() + 1);
		temp.getDir().setN_elemento(temp.getDir().getN_elemento() + 1);
	}
	
	public void rotacaoE (No no)
	{
		No temp = no.getPai();
		
		no.getPai().getPai().setDir(no);
		no.setPai(no.getPai().getPai());
		no.setN_elemento(no.getN_elemento() - 1);
		no.getDir().setN_elemento(no.getDir().getN_elemento() - 1);
		
		temp.setDir(no.getEsq());
		no.getEsq().setPai(temp);
		
		no.setEsq(temp);
		temp.setPai(no);
		temp.setN_elemento(temp.getN_elemento() + 1);
		temp.getEsq().setN_elemento(no.getEsq().getN_elemento()-1);
		
	}
	
	public No inserir(No novo, No anterior)
	{
		return inserir(novo, anterior, null, 0);
	}
	
	public No inserir(No novo, No anterior, No pai, int n_elemento){
	      if (anterior != null){
	         pai = anterior;
	    	  
	    	 if (novo.getDado() < anterior.getDado())
	             anterior.setEsq(this.inserir(novo, anterior.getEsq(), pai, n_elemento + 1));
	         else {
	             if(novo.getDado() > anterior.getDado())
	                anterior.setDir(this.inserir(novo, anterior.getDir(), pai, n_elemento + 1));
	             else     
	                return null;
	         }
	      } else {	      
	           anterior = novo;
	           anterior.setN_elemento(n_elemento);
	           anterior.setPai(pai);
	      }
	      return anterior;
	  }
	
	 private boolean emOrdem(No no, int dado, boolean existe) {
	     if(no != null) {
	    	 if(no.getDado() == dado) {
	    		 no.setOcorrencia(no.getOcorrencia() + 1);
	    		 existe = true;
	    	 }
	    	 else{
	    		 if(emOrdem(no.getEsq(), dado, existe))
	    			 return true;
	    		 if(emOrdem(no.getDir(), dado, existe))
	    			 return true;
	    	 }
	     }
	     return existe;
	 }
	 
	 public boolean existeElemen(No no, int dado){
		 return emOrdem(no, dado, false);
	 }
	 
	 private void emOrdem(No no, StringBuffer arvoreString)
	 {
		 if(no != null) {
			 emOrdem(no.getEsq(), arvoreString);
	         System.out.print(no.getDado() + "  |  " + no.getOcorrencia() + "  |     " + no.getN_elemento() + "\n");
	         arvoreString.append(no.getDado() + " " + no.getOcorrencia() + "\n");
	         emOrdem(no.getDir(), arvoreString);
	     }
	 }
	 
	 public String toString(){
		 StringBuffer arvoreString = new StringBuffer();
		 System.out.print("N  | Qtd | n_elemento\n");
		 emOrdem(this.raiz, arvoreString);
		 return arvoreString.toString();
	 }
}

