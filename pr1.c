#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

/*main function*/ 

int main(int argc, char *argv[]) 				/* expects two arguments the file name and time for interrupt*/
{
	int p1[2], p2[2];    						/* two integer pointers namely p1 and p2 are created*/
    int result, mode;    						/* result differentiates between child and parent*/
	int memory[2000];							/* integer array for memory space*/
	
	/*File opening and reading to memory*/
	FILE *fp;
	char buf[200];
	int index =0, value,i, time = (int)argv[2];
	fp = fopen(argv[1], "r");
	if (fp == NULL) 
	{
		printf("File does not exist please check!\n");
	}
	while (fgets(buf,200, fp)!=NULL)
	{
		if(buf[0]=='.')
		{
			if(sscanf(buf, ".%d", &value)==1)
			{
				index = value;
			}		
		}
		else
		{
			if(sscanf(buf, "%d", &value)==1)
			{
				memory[index]=value;
				index++;
			}
		}
	}
    fclose(fp);
	
   /*two pipes are created - if pipe creation fails the program exits with an error*/
   result = pipe(p1);
   if (result == -1)
		exit(1);
	
   result = pipe(p2);
   if (result == -1)
		exit(1);
	
   /*program is forked into two processes each for CPU and Memory Interfaces*/
   result = fork();
   
   if (result == 0) //CPU Process
   {
		int PC, SP, IR, AC, X, Y, SP1, PC1; /*registers for the memory are declared*/
		int add, add1, flag;
		char command;
		close(p1[0]); /*the reading end of p1 is closed*/
		close(p2[1]); /*the writing end of p2 is closed*/
		
		PC=0, flag = 0, mode =0, SP=999; /*initial conditions of CPU is set*/
		/* mode 0 : user mode
		   mode 1 : system mode */
		
		for(;;)  /*this loop runs until flag is 1*/
		{
			if((PC == time)&&(mode==0)) /*handles timer interrupt */
			{
				time = time+time; /*interrupt is reset for after next time commands*/
				mode = 1;         /*mode changed to system mode*/
				SP1 = SP;
				PC1 = PC;
				SP =1999;			/*SP and PC set accordingly*/
				PC = 1000;
				command = 'w';       /*pushing user Stack Pointer and Program counter into System Stack*/
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));
				write(p1[1], &SP1, sizeof(int));	
				SP--;
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));
				write(p1[1], &PC1, sizeof(int));
			}
			command = 'r';					/*reading Instruction in PC*/
			write(p1[1], &command, sizeof(char));
			write(p1[1], &PC, sizeof(int));	
			PC++;
			read(p2[0], &IR, sizeof(int));
			switch(IR)						/*switching between various possible instructions*/
			{
				case 1 :                   /*Load the value at address in PC into the AC*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &value, sizeof(int));
				AC = value;
				break;
				
				case 2:					/*Load the value at the address into the AC*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &add, sizeof(int));
				write(p1[1], &command, sizeof(char));
				write(p1[1], &add, sizeof(int));	
				read(p2[0], &value, sizeof(int));
				AC = value;
				break;

				case 3:                /*Load the value from the address found in the address into the AC*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &add, sizeof(int));
				write(p1[1], &command, sizeof(char));
				write(p1[1], &add, sizeof(int));	
				read(p2[0], &add, sizeof(int));
				write(p1[1], &command, sizeof(char));
				write(p1[1], &add, sizeof(int));	
				read(p2[0], &value, sizeof(int));
				AC = value;
				break;
				
				case 4:				/*Load the value at (address+X) into the AC*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &add, sizeof(int));
				add = add+X;
				write(p1[1], &command, sizeof(char));
				write(p1[1], &add, sizeof(int));	
				read(p2[0], &value, sizeof(int));
				AC = value;
				break;
				
				case 5:				/*Load the value at (address+Y) into the AC*/
			    command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &add, sizeof(int));
				add = add+Y;
				write(p1[1], &command, sizeof(char));
				write(p1[1], &add, sizeof(int));	
				read(p2[0], &value, sizeof(int));
				AC = value;
				break;
				
				case 6:			/*Load from (Sp+X) into the AC*/
				command = 'r';
				add = SP + X;
				write(p1[1], &command, sizeof(char));
				write(p1[1], &add, sizeof(int));	
				read(p2[0], &value, sizeof(int));
				AC=value;
				break;
				
				case 7:			/*Store the value in the AC into the address*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &add, sizeof(int));
				command = 'w';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &add, sizeof(int));
				write(p1[1], &AC, sizeof(int));
				break;
				
				case 8:			/*Gets a random int from 1 to 100 into the AC*/
				AC = rand() % 100 + 1;
				printf("AC = %d\n", AC);
				break;
				
				case 9:			/*If port=1, writes AC as an int to the screen and If port=2, writes AC as a char to the screen*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &value, sizeof(int));
				if(value==1)
				{	printf("%d",AC);}
				else if(value==2)
				{	printf("%c",AC);}
				break;
				
				case 10:		/*Add the value in X to the AC*/
				AC=AC+X;
				break;
				
				case 11:		/*Add the value in Y to the AC*/
				AC=AC+Y;
				break;
				
				case 12:		/*Subtract the value in X from the AC*/
				AC=AC-X;
				break;
				
				case 13:		/*Subtract the value in Y from the AC*/
				AC=AC-Y;
				break;
				
				case 14:		/*Copy the value in the AC to X*/
				X=AC;
				break;
				
				case 15:		/*Copy the value in X to the AC*/
				AC=X;
				break;
				
				case 16:		/*Copy the value in the AC to Y*/
				Y=AC;
				break;
				
				case 17:		/*Copy the value in Y to the AC*/
				AC=Y;
				break;
				
				case 18:		/*Copy the value in AC to the SP*/
				SP=AC;
				break;
				
				case 19:			/*Copy the value in SP to the AC */
				AC=SP;
				break;
				
				case 20:		/*Jump to the address*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &add, sizeof(int));
				PC = add;
				break;
				
				case 21:		/*Jump to the address only if the value in the AC is zero*/
				if(AC == 0)
				{
					command = 'r';
					write(p1[1], &command, sizeof(char));
					write(p1[1], &PC, sizeof(int));
				}					
					PC++;
				if(AC == 0)
				{
					read(p2[0], &add, sizeof(int));
					PC = add;
				}
				break;
				
				case 22:		/*Jump to the address only if the value in the AC is not zero*/
				if(AC != 0)
				{
					command = 'r';
					write(p1[1], &command, sizeof(char));
					write(p1[1], &PC, sizeof(int));	
				}
					PC++;
				if(AC != 0)
				{
					read(p2[0], &add, sizeof(int));
					PC = add;
				}
				break;			
				
				case 23:		/*Push return address onto stack, jump to the address*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &PC, sizeof(int));	
				PC++;
				read(p2[0], &add, sizeof(int));
				SP--;
				command = 'w';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));
				write(p1[1], &PC, sizeof(int));	
				PC = add;
				
				break;
				
				case 24:		/*Pop return address from the stack, jump to the address*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));	
				read(p2[0], &add, sizeof(int));
				SP++;
				PC = add;
				break;
				
				case 25:		/*Increment the value in X*/
				++X;
				break;
				
				case 26:		/*Decrement the value in X*/
				--X;
				break;
				
				case 27:		/*Push AC onto stack*/
				SP--;
				command = 'w';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));
				write(p1[1], &AC, sizeof(int));		
				break;
				
				case 28:		/*Pop from stack into AC*/
				command = 'r';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));	
				read(p2[0], &AC, sizeof(int));
				SP++;
				break;
				
				case 29:		/*Set system mode, switch stack, push SP and PC, set new SP and PC*/
				mode = 1; 
				SP1 = SP;
				PC1 = PC;
				SP =1999;
				PC = 1500;
				command = 'w';
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));
				write(p1[1], &SP1, sizeof(int));	
				SP--;
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));
				write(p1[1], &PC1, sizeof(int));
				break;
				
				case 30:		/*Restore registers, set user mode*/
				command = 'r';
				mode =0;
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));	
				read(p2[0], &PC1, sizeof(int));
				SP++;
				write(p1[1], &command, sizeof(char));
				write(p1[1], &SP, sizeof(int));	
				read(p2[0], &SP1, sizeof(int));
				SP++;
				SP = SP1;
				PC = PC1;
				break;
				
				case 50 :		/*End execution*/
				command = 'e';				
				write(p1[1], &command, sizeof(char));
				flag = 1;
				break;				
			}
			if(flag == 1)
			{break;}
		}
		  
   }
   else            // Memory Process
   {	 
		char command;
		int add;
		int value;
		int i;
	    close(p1[1]);
		close(p2[0]);
		for(;;)
		{
			read(p1[0], &command, sizeof(char));
			if(command == 'r')  /*if command is read, the memory sends back the value at the address*/
			{
				read(p1[0], &add, sizeof(int));
				value = memory[add];		
				write(p2[1], &value, sizeof(int));
			}
			else if(command == 'w') /*if command is write, the memory stores the value received at the given address*/
			{
				read(p1[0], &add, sizeof(int));
				read(p1[0], &value, sizeof(int));
				memory[add]=value;
			}
			else if(command == 'e') /*if command is e, the memory process exits*/
			{
			    break;
			}
		}
   }
}
