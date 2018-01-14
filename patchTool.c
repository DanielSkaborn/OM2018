// patchTool

#include <stdio.h>
#include <pthread.h>


FILE *cmdfile;
FILE *rspfile;

pthread_t numpadscan;

volatile int scanNumFlag = 2;
volatile int number;

void *inputnumbers(void *arg) {
	while(1) {
		if (scanNumFlag == 0) {
			usleep(1000);
			printf("\n >>> ");
			scanf("%d",&number);
			scanNumFlag = 1;
		}
		usleep(10000);
	}
}

int main(void) {
	char c;

	cmdfile = fopen("/tmp/OMT.cmd", "w");
	fprintf(cmdfile,"10000\n");
	fclose(cmdfile);

	cmdfile = fopen("/tmp/OMF.cmd", "w");
	fprintf(cmdfile,"0\n");
	fclose(cmdfile);

	printf("\n\nOpenModularPatchTool version 0.1\n\nInitializing communication with the OpenModular...\n\n\n");

	pthread_create(&numpadscan, NULL, inputnumbers, NULL);

	while(1) {
		rspfile = fopen("/tmp/OMF.rsp","r");
		if (rspfile != NULL) {
			fclose(rspfile);
			rspfile = fopen("/tmp/OMT.rsp","r");
			if (rspfile != NULL) {
				while(1) {
					c = fgetc(rspfile);
					if (feof(rspfile))
						break;
					printf("%c",c);
				}
				fclose(rspfile);

				remove("/tmp/OMT.rsp");
				remove("/tmp/OMF.rsp");
				scanNumFlag = 0;
			}
		}
		usleep(100000);
		if (scanNumFlag == 1) {
			scanNumFlag = 2;

			cmdfile = fopen("/tmp/OMT.cmd","w");
			fprintf(cmdfile, "%d\n", number);
			fclose(cmdfile);

			cmdfile = fopen("/tmp/OMF.cmd","w");
			fprintf(cmdfile, "\n");
			fclose(cmdfile);
		}
	}
	return 0;
}


