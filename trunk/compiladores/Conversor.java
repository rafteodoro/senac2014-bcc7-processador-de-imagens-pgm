

import java.util.ArrayList;

public class Conversor {
	StringBuffer cadeia;
	int posCadeia;
	
	public Conversor(StringBuffer cadeia)
	{
		this.cadeia = cadeia;
		posCadeia = 0;
	}
	
	public void automatoNumerico()
	{
		int estado = 1;
		switch (estado)
		{
			case 1:
				if(cadeia.charAt(posCadeia)=='-')
					estado = 2;
			break;
				
			case 2:
				if(cadeia.charAt(posCadeia)=='0')
					estado = 4;
			break;
				
			case 3:
				if(cadeia.charAt(posCadeia)=='0' || 
				   cadeia.charAt(posCadeia)=='1' ||
				   cadeia.charAt(posCadeia)=='2'||
				   cadeia.charAt(posCadeia)=='3'||
				   cadeia.charAt(posCadeia)=='4'||
				   cadeia.charAt(posCadeia)=='5'||
				   cadeia.charAt(posCadeia)=='6'||
				   cadeia.charAt(posCadeia)=='7'||
				   cadeia.charAt(posCadeia)=='8'||
				   cadeia.charAt(posCadeia)=='9')
					estado = 4;
			break;
				
			case 4:
				if(cadeia.charAt(posCadeia)=='0' || 
				   cadeia.charAt(posCadeia)=='1' ||
				   cadeia.charAt(posCadeia)=='2'||
				   cadeia.charAt(posCadeia)=='3'||
				   cadeia.charAt(posCadeia)=='4'||
				   cadeia.charAt(posCadeia)=='5'||
				   cadeia.charAt(posCadeia)=='6'||
				   cadeia.charAt(posCadeia)=='7'||
				   cadeia.charAt(posCadeia)=='8'||
				   cadeia.charAt(posCadeia)=='9')
					estado = 4;
			break;
				
			case 5:
				
			break;				
				
			case 6:
				
			break;
			
			case 7:
				
			break;
		}
	}
	
	
}
