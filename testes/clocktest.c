#include <stdio.h>
#include <time.h>

int main(void)
{
	clock_t ticks1, ticks2;

	while(1){
		ticks1=clock();
		ticks2=ticks1;
		while((ticks2/CLOCKS_PER_SEC-ticks1/CLOCKS_PER_SEC)<1)
			ticks2=clock();

		printf("It took %ld ticks to wait one second.\n",ticks2-ticks1);
		printf("This value should be the same as CLOCKS_PER_SEC which is %d.\n",CLOCKS_PER_SEC);
	}
	
	return 0;
}