# include<stdio.h>
# include<sys/types.h>
# include<unistd.h>
# include<errno.h>
# include<sys/wait.h>
#include<string.h>

void run_program(char *name)
{
    char cmd[1024] = "./";
    
    // Put path coresponding to your files
    char path[1024] = "./library/programs/client/";

    strcat(cmd, name);
    strcat(path, name);

    char *execve_argv[] = {cmd, NULL};
		execve(path, execve_argv, NULL);

}

int main(int argc, char* argv[])
{

    char opt;

    printf("Welcome to Telemetry!\n");
    printf("\nChoose one role:\n");
    printf("\t1. Publisher\n");
    printf("\t2. Subscriber\n");
    printf("\t3. Sublisher\n");
    printf("Your option: ");

	scanf("%c", &opt);
	
	if (opt == '1')
	{
		run_program("publisher");
	}
	else if (opt == '2')
	{
		run_program("subscriber");
	}
	else if (opt == '3')
	{
		run_program("sublisher");
	}
	else 
	{
		printf("Invalid option!");
	}
	
	
	return 0;
}
