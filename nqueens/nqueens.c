#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include <sys/time.h>

int* InitTabuleiro(int n_rows){
    int *tabuleiro;
    tabuleiro = malloc((n_rows*n_rows) * sizeof(int));
    #pragma opm parallel
    {
    #pragma opm parallel for
    for (int i=0; i < n_rows*n_rows; ++i)
        tabuleiro[i] = 0;
    }
    return tabuleiro;
}

int* CopyTabuleiro(int *tabuleiro, int n_rows){
    int* tab = malloc((n_rows*n_rows) * sizeof(int));
    #pragma opm parallel
    {
    #pragma opm parallel for
    for (int i=0; i < n_rows*n_rows; ++i)
        tab[i] = tabuleiro[i];
    }
    return tab;
}

int* ClearTabuleiro(int *tabuleiro, int n_rows){
    #pragma opm parallel
    {
    #pragma opm parallel for
    for (int i=0; i < n_rows*n_rows; ++i)
		if(tabuleiro[i] == 1)
        	tabuleiro[i] = 0;
    }
    return tabuleiro;
}

int* QueenMoves(int *tabuleiro, int n_rows, int queen){
	int i=(queen-n_rows);
#pragma opm parallel
    {
	// coluna
	while(i >= 0){
		tabuleiro[i] = 1;
		i-=n_rows;
	}
	i=(queen+n_rows);
	while(i < n_rows*n_rows){
		tabuleiro[i] = 1;
		i+=n_rows;
	}

	// linha
	int aux = queen % n_rows;
	i=queen-1;
	while(i >= (queen-aux)){
		tabuleiro[i] = 1;
		--i;
	}
	aux = n_rows - aux -1;
	i=queen+1;
	while(i <= (queen+aux)){
		tabuleiro[i] = 1;
		++i;
	}

	// diagonal principal
	i = queen-(n_rows + 1);
	while((i >= 0) && ((i % n_rows) < 7)){
		tabuleiro[i] = 1;
		i -= n_rows + 1;
	}
	i = queen+(n_rows + 1);
	while((i < n_rows*n_rows) && ((i % n_rows) > 0)){
		tabuleiro[i] = 1;
		i += n_rows + 1;
	}

	// diagonal secundaria
	i = queen-(n_rows - 1);
	while((i >= 0) && ((i % n_rows) > 0)){
		tabuleiro[i] = 1;
		i -= n_rows - 1;
	}
	i = queen+(n_rows - 1);
	while((i < n_rows*n_rows) && ((i % n_rows) < (n_rows-1))){
		tabuleiro[i] = 1;
		i += n_rows - 1;
	}
    }
	return tabuleiro;
}

/*void PrintTabuleiro(int tabuleiro[], int n_rows){
	int aux=0;
    for (int i=0; i < n_rows*n_rows; ++i){
        if((i%n_rows) == 0)
            printf("\n");

		if(tabuleiro[i] == 1){
        	tabuleiro[i] = 0;
			aux =1;
		}

        printf("| ");
        printf("%d | ",tabuleiro[i]);

		if(aux == 1){
			tabuleiro[i] = 1;
		}
    }
	printf("\n");
}*/


int QuantQueens(int tabuleiro[], int n_rows){
	int queen = 0;
	#pragma opm parallel
    {
    #pragma opm parallel for
	for (int i=0; i < n_rows*n_rows; ++i){
		if(tabuleiro[i] == 2){
			++queen;
		}
	}
    }
	return queen;
}

int* Simula(int *possibilidade, int *tabuleiro, int n_rows, int *p_size, int *qt){
	int *tab;
	int i, j, queen;

    queen=QuantQueens(tabuleiro, n_rows); //verifica quantidade de rainhas
    //printf("%d\n",queen);
	if(queen == n_rows){ //se a solucao der certo
		int flag=1; //marca se achou uma diferença entre os tabuleiros
		int flag2=1; //marca se achou um tabuleiro igual
        #pragma opm parallel
        {
            #pragma opm parallel for
		for(i=0; i < (*p_size); i+=(n_rows*n_rows)){
           // #pragma omp critical
                flag = 0;
            if(flag2==0){//se achou tabuleiro igual, não faz nada
                flag2=0;
            }
            else{

			for(j=0; j < (n_rows*n_rows) && flag!=1; ++j){
				if(possibilidade[i+j] != tabuleiro[j]){ //se os valores forem diferentes
					flag = 1; // tabuleiros diferentes
				}
			}
        }
			if(flag==0){ //se o tabuleiro existir no historico
				flag2=0;
			}
            }
		}
		if(flag2 != 0){//se o tabuleiro não estava no historico
			{
			(*p_size) += (n_rows*n_rows);
			int comeco = (*p_size) - (n_rows * n_rows);
			possibilidade = realloc(possibilidade, sizeof(int)*(*p_size) );
			j = 0;
			#pragma opm parallel
        {
            #pragma opm parallel for
			for(i=comeco; i < comeco+(n_rows * n_rows); ++i){//adiciona no historico
				possibilidade[i] = tabuleiro[j];
				++j;
			}
        }
			//PrintTabuleiro(tabuleiro, n_rows);
			*qt= *qt+1;
			}
			return possibilidade;
		}

	}

	else{

        #pragma opm parallel
        {
            #pragma opm parallel for
        for (i=0; i< n_rows*n_rows; ++i){
            if(tabuleiro[i] == 0){ // se posição disponivel
                tab = CopyTabuleiro(tabuleiro, n_rows); //cria tabuleiro auxiliar
                tab[i] = 2; // coloca rainha
                tab = QueenMoves(tab, n_rows, i); //pinta movimentos
                possibilidade= Simula(possibilidade, tab, n_rows, p_size, qt); //chama recursão para colocar proxima rainha
                free(tab);
            }
        }
        }
	}

	return possibilidade;
}

int main(int argc, char *argv[]){
    double start_time, end_time;
    omp_set_num_threads(30);
    start_time = omp_get_wtime();
    int n_rows = atoi(argv[1]);
	int *qt = malloc(sizeof(int));
	*qt = 0;
	int *p_size = malloc(sizeof(int));
	(*p_size) = 0;

    int *tabuleiro, *possibilidade;
	possibilidade = malloc((n_rows*n_rows)*sizeof(int));
    tabuleiro = InitTabuleiro(n_rows);

    Simula(possibilidade, tabuleiro, n_rows, p_size, qt);
    end_time = omp_get_wtime();
    printf("\nNumero de solucoes: %d\n", *qt);
    printf("\nO tempo de execucao é %g sec\n", end_time - start_time);
    return 0;
}
