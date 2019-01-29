/*
Name: Vibhav Chemarla
Date: 1/27/2019
CS4760 OS Project 1
Practice making system calls with the use of getopt, fork, and perror
*/


#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char **argv)
{
	int optionIndex, numOfForks;
	char *fileName = NULL;
	opterr = 0;
	

	while ((optionIndex = getopt(argc, argv, ":hi::o::")) != -1)
	{
		switch(optionIndex)
		{
			case 'h':
				printf("List of valid command line argument usage\n");
				printf("-h	      :  Lists all possible command line arguments, their explanation,and usage.\n");
				printf("-i <argument> :  This option will take a file name as an argument and open the given\n\t\t file and parse its contents to continue the program.\n\t\t If no argument is provided, (input.dat)will be used as\n\t\t a default argument.\n");
				printf("-o <argument> :  This option will output data from the program to the given file name\n\t\t argument. If no argument is provided, (output.dat) will be used as a\n\t\t default argument.\n");
				break;
			case 'i':
				if(optarg == NULL)
				{
					optarg = "input.dat";
				}
				fileName = optarg;
				printf("Input File: %s\n",fileName);
				printf("Opening File...\n");
				FILE* in_file = fopen(fileName, "r");
				if(in_file == NULL)
				{
					printf("Could not open file\n");
					return 1;
				}
				if(in_file != NULL)
				{
					printf("File opened successfully!\n");
					char numberBuffer[50];
					while(fgets(numberBuffer, sizeof numberBuffer, in_file))
					{
						printf("%s\n", numberBuffer);
					}
					
				}
				printf("Closing File...\n");
				fclose(in_file);
				
				break;
			case 'o':
				if(optarg == NULL)
				{
					optarg = "output.dat";
				}
				fileName = optarg;
				printf("Output File: %s\n", fileName);
				break;
			default:
				printf("a.out: Error: Incomplete argument usage. Use '-h' argument for valid usage instructions.\n\n Example: ./a.out -h\n");
				break;
		}
	}
	return 0;
}
