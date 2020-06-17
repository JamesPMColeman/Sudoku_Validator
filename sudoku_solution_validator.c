#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define N 9
#define T 27

typedef int bool;
#define TRUE 1
#define FALSE 0

int sudokuPuzzle[N][N]; 

bool rowResults[N];
bool colResults[N];
bool subResults[N];

typedef struct {
		int topRow;
		int bottomRow;
		int leftCol;
		int rightCol;
	} region;

void *validateRegion(void *rg);
void recordBool(region *rg, int valid);
void printRegionResult(char *, pthread_t tid[], bool results[]);

int main() {
	int i, j;

	pthread_t tid_row[N];
	pthread_t tid_col[N];
	pthread_t tid_sub[N];
	pthread_attr_t attr[T];

	region rows[N];
	region cols[N];
	region subs[N];

	char * input = "SudokuPuzzle.txt";
	FILE *sudoku_test;

	sudoku_test = fopen(input, "r");
	if (sudoku_test == NULL) {

		printf("ERROR: Cannot find '%s'\n", input);
		exit(1);
	}
	printf("\n================================");
	printf("=================================\n");
	printf("*\t\tEvaluating %s\t\t\t*\n\n", input);
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {

			fscanf(sudoku_test, "%d", &sudokuPuzzle[i][j]);
			if (sudokuPuzzle[i][j] <= 0 || sudokuPuzzle[i][j] > 9) {
				printf("\n\nERROR: '%s' contains an incomplete puzzle.\n\n", input);
				exit(1);
			}
			printf("%d\t", sudokuPuzzle[i][j]);
		}
		printf("\n");
	}
	fclose(sudoku_test);

	for (i = 0; i < T; i++) 
		pthread_attr_init(&(attr[i]));
	
	for (i = 0; i < N; i++) {
		region x;
		x.topRow = i;
		x.bottomRow = i;
		x.leftCol = 0;
		x.rightCol = 8;
		rows[i] = x; 
	}
	for (i = 0; i < N; i++) {
		region x;
		x.topRow = 0;
		x.bottomRow = 8;
		x.leftCol = i;
		x.rightCol = i;
		cols[i] = x; 
	}
	for (i = 0; i < N; i++) {
		region x;
		x.topRow = 3 * (i / 3);
		x.bottomRow = 3 * (i / 3) + 2;
		x.leftCol = 3 * (i % 3);
		x.rightCol = 3 * (i % 3) + 2;
		subs[i] = x; 
	}
	for (i = 0; i < N; i++) {
		pthread_create(&(tid_row[i]), &(attr[i]), validateRegion, &(rows[i]));
		pthread_create(&(tid_col[i]), &(attr[i + 9]), validateRegion, &(cols[i]));
		pthread_create(&(tid_sub[i]), &(attr[i + 18]), validateRegion, &(subs[i]));
	}
	for (i = 0; i < N; i++) {
		pthread_join(tid_row[i], NULL);
		pthread_join(tid_col[i], NULL);
		pthread_join(tid_sub[i], NULL);
	}
	printRegionResult("Column", tid_col, colResults);
	printRegionResult("Row", tid_row, rowResults);
	printRegionResult("Subgrid", tid_sub, subResults);

	for (i = 0; i < N; i++) {

		if (rowResults[i] == FALSE || colResults[i] == FALSE || subResults[i] == FALSE) {
			printf("\n*\t\tSudoku Puzzle: Invalid\t\t\t\t*");
			printf("\n=================================================================\n\n");
			exit(0);
		}
	}
	printf("\n*\t\tSudoku Puzzle: Valid\t\t\t\t*"); 
	printf("\n=================================================================\n\n");
	return 1;
}

void *validateRegion(void *rg) {

	region *local;
	int i, j, z = 0;
	int rs[N];
	pthread_t this;

	local = (region *)rg;

	for (i = local->topRow; i <= local->bottomRow; i++) {
		for (j = local->leftCol; j <= local->rightCol; j++) {
			rs[z++] = sudokuPuzzle[i][j];
		}
	}

	this = pthread_self();

	for (z = 0; z < N; z++) {
		for (i = z + 1; i < N; i++) {

			if (rs[z] == rs[i]) {
				printf("%lX TRow: %d, BRow: %d, LCol: %d, RCol: %d invalid!\n", 
					  (unsigned long)this, local->topRow, local->bottomRow, 
					  local->leftCol, local->rightCol);

				recordBool(local, FALSE);
				pthread_exit(0);
			}
		}
	}

	printf("%lX TRow: %d, BRow: %d, LCol: %d, RCol: %d valid!\n", 
		  (unsigned long)this, local->topRow, local->bottomRow, 
		  local->leftCol, local->rightCol);
	
	recordBool(local, TRUE);
	pthread_exit(0);
}

void recordBool(region *rg, bool valid) {

	if (rg->topRow == rg->bottomRow)
		rowResults[rg->topRow] = valid;

	else if (rg->leftCol == rg->rightCol)
		colResults[rg->leftCol] = valid;

	else 
		subResults[rg->topRow + (rg->rightCol / 3)] = valid;

}

void printRegionResult(char *message, pthread_t tid[], bool results[]) {
	int i;

	for (i = 0; i < N; i++) {

		if (results[i] == 1)
			printf("%s: %lX valid\n", message, (unsigned long)tid[i]);
		else
			printf("%s: %lX invalid\n", message, (unsigned long)tid[i]);
	}
}
