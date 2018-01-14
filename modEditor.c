#include <stdio.h>
void printConnectsTo(int bus);
void printModOuts(void);
void printModIns(int id);
void printMods(void);
void *patchtexteditor(void *arg);

void storePatch(void);
void loadPatch(int prg);
void clearPatches(void);

volatile int statpb=-1;
volatile float statvl;
volatile int chpb=-1;
volatile int chid;
volatile int chin;

FILE *cmdfile;
FILE *rspfile;

void printModOuts(void) {
	int i, ii, iii, id;
	int temp;

	for (i=0;i<120;i++) {
		temp=0;
		for (id=0;id<numberOfModules;id++)
			for (ii=0;ii<modIns[id];ii++)
				if (patchIn[id][ii]==i) 
					temp=1;
			if (temp==1) {
				fprintf(rspfile,"\n %03d MIDI CC          ",i);
				printConnectsTo(i);
			}
	}
	fprintf(rspfile,"\n 120 PitchBend        ");
	printConnectsTo(120);

	fprintf(rspfile,"\n\n");

	for (i=0;i<numberOfModules;i++) {
		for(ii=0;ii<modOuts[i];ii++) {
			fprintf(rspfile," %02d ",patchOut[i][ii]); // patchnumber
			fprintf(rspfile," %02d ",i); // id
			for(iii=0;iii<9;iii++)
				fprintf(rspfile, "%c",modName[i][iii]);
			fputc(' ',rspfile);
			for(iii=0;iii<4;iii++)
				fprintf(rspfile, "%c", modOutsName[i][ii*4+iii]);
			printConnectsTo(patchOut[i][ii]);
			fprintf(rspfile,"\n");
		}
	}
	fprintf(rspfile, "\n 995 Gate\n 996 Note\n 997 Clear\n 998 Load\n 999 Save\n 1000+ Set static value to bus\n");
}

void printConnectsTo(int bus) {
	int id, i, ii;

	for (id=0;id<numberOfModules;id++) {
		for (i=0;i<modIns[id];i++)
			if (patchIn[id][i]==bus) {
				fprintf(rspfile," > %02d ",id); // id
				for(ii=0;ii<9;ii++)
					fprintf(rspfile, "%c", modName[id][ii]);
				fprintf(rspfile, "%c",' ');
				for(ii=0;ii<4;ii++)
					fprintf(rspfile, "%c", modInsName[id][i*4+ii]);
			}
	}
}

void printModIns(int id) {
	int i, ii;

	for(i=0;i<9;i++) // Module name
		fprintf(rspfile, "%c", modName[id][i]);
	for(i=0;i<modIns[id];i++) {
		fprintf(rspfile,"\n %02d: ",i);
		for(ii=0;ii<4;ii++)
			fprintf(rspfile, "%c", modInsName[id][i*4+ii]);
		fprintf(rspfile," %d",patchIn[id][i]);
	}
	return;
}

void printMods(void) {
	int id, i;

	for(id=0;id<numberOfModules;id++) {
		fprintf(rspfile,"\n %02d: ",id); // id
		for(i=0;i<9;i++)
			fprintf(rspfile,"%c",modName[id][i]);
	}
	return;
}

void savePatch(void) {
	int id, in;

	for(id=0;id<numberOfModules;id++) {
		for (in=0;in<modIns[id];in++)
			fprintf(rspfile, "patchIn[%d][%d] = %d; ", id, in, patchIn[id][in]);
		fprintf(rspfile, "patchNote[%d] = %d; ", id, patchNote[id]);
		fprintf(rspfile, "patchGate[%d] = %d; ", id, patchGate[id]);
	}
	in = getpToolCmd();
}

void patch_set(int pb, int id, int in) {
	chpb=pb;
	chid=id;
	chin=in;
	printf("patchset %d %d %d\n",pb, id, in);
	return;
}

void patch_static(int pb, float vl) {
	statpb=pb;
	statvl=vl;
	printf("patchstatic %d %f\n",pb, vl);
	return;
}

void editor_doparams(void) {
	if (chpb!=-1) {
		patchIn[chid][chin]=chpb;
		chpb=-1;
		printf("didpatch\n");
	}
	if (statpb!=-1) {
		patchBus[statpb][1] = patchBus[statpb][0] = statvl;
		statpb=-1;
		printf("didstatic\n");
	}
	chpb = -1;
	statpb = -1;
	return;
}

int getpToolCmd(void) {
	int temp;

	if (rspfile != NULL) {
		fclose(rspfile);
		rspfile = fopen("/tmp/OMF.rsp","w");
		fprintf(rspfile,"0\n");
		fclose(rspfile);
	}

	while(1) {
		cmdfile = fopen("/tmp/OMF.cmd","r");
        	if (cmdfile != NULL) {
                        fclose(cmdfile);
                        fopen("/tmp/OMT.cmd","r");
                        if (cmdfile != NULL) {
                                fscanf(cmdfile,"%d",&temp);
				remove("/tmp/OMF.cmd");
				remove("/tmp/OMT.cmd");
				rspfile = fopen("/tmp/OMT.rsp","w");
				return temp;
                        }
                }
	}
}

void *patchtexteditor(void *arg) {
	int p, id, in;

	while(1)
		if (getpToolCmd() == 10000)
			break;
	printf("OpenModular pToolPatcher running\n");

	fprintf(rspfile,"\n\nOpenModular TextPatcher\n\n");

	while(1) {
		printModOuts();

		fprintf(rspfile,"\nEnter patchbus\n");
		p = getpToolCmd();

		if ( p > 799 ) {
			if (p == 997) clearPatches();
			if (p == 998) {
				fprintf(rspfile,"\nEnd stored PatchNumber to load\n");
				in = getpToolCmd();
				loadPatch(in);
			}
			if (p == 999) storePatch();
			if (p == 996) {
				printMods();
				fprintf(rspfile, "\nEnter Module Id to assign a note bus\n");
				id = getpToolCmd();
				if ( id > numberOfModules ) {
					fprintf(rspfile, "\nInvalid module ID.\n");
				} else {
					fprintf(rspfile, "\nBus (0=first, 1=sec, 2=no\nEnter Notebus\n");
					p = getpToolCmd();
					if (!(p>2)) patchNote[id]=p;
				}
			} else if (p==995) {
				printMods();
				fprintf(rspfile,"\nEnter Module Id to assign a gate bus\n\n");
				id = getpToolCmd();
				if ( id > numberOfModules ) {
					fprintf(rspfile,"\nInvalid module ID.\n");
				} else {
					fprintf(rspfile,"\nEnter gate bus> ");
					p = getpToolCmd();
					if (!(p>2)) patchGate[id]=p;
				}
			}
			if ((p > 999) && (p < 1121)) {
				if ((p-1000<NOPATCHBUS) && (p>1000)) {
					fprintf(rspfile, "\nSet static value to bus %03d (-100 to 100) > ", p-1000);
					id = getpToolCmd();
					//patchBus[p-1000][1] = patchBus[p-1000][0] = (float)(id)/100.0;
					patch_static(p-1000, (float)(id)/100.0);
				} else {
					fprintf(rspfile,"\n Invalid bus number.\n");
				}
			}
		}
		else if ( p > NOPATCHBUS ) {
			fprintf(rspfile,"\n Invalid patchbus, out of range\n");
		} else {
			printMods();
			fprintf(rspfile,"\n\nEnter Module Id\n");
			id = getpToolCmd();
			if ( id > numberOfModules ) {
				fprintf(rspfile,"\n Invalid module ID.\n");
			} else {
				printModIns(id);
				fprintf(rspfile, "\n\nEnter Input");
				in = getpToolCmd();
				if (in > modIns[id]) {
					fprintf(rspfile,"Invalid module input, out is of range.\n");
				} else {
					patch_set(p, id, in);
				}
			}
		}
		usleep(1000);
	}
	return NULL;
}
