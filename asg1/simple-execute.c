// CSCI3150 Asg 1
// Name: YUAN Lin
// SID: 1155141399
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAX_ARG_NUM  30
#define MAX_SEG_NUM  4

//YUAN Lin



int shell_execute(char ** args, int argc)
{
	int num_pipes = 0;
	int child_pid;
	int segment_index = 0;
	char* argvlist[MAX_SEG_NUM+1][MAX_ARG_NUM+1];

	if (strcmp(args[0], "EXIT") == 0)
		return -1; 
	// EXIT to exit the program

	if(argc == 1){
		return 0;// if only has the NULL as argument, directly return without any execution

	}


	//process the argv segments
	int pos = 0;
	int count = 0;
	// why c doesn't have strings but need to use so many pointers
	// I spend 2 nights on it I hate pointers
	for(count=0; count<MAX_SEG_NUM; count++){
		if(pos==(argc-1)){
            break;// if reach the end of the command list, break
        }
		for(int j=0; j<MAX_ARG_NUM; j++){

            if((args[pos]==NULL)|| args[pos][0] == '\0'||(strcmp(args[pos],"|")==0) ){    //if reach the end of command list or reach a pipe sign, have a NULL in the array row
			//here also use the short-circuit OR to avoid strcmp(NULL,"|")
            argvlist[count][j]=NULL; 
            pos++; //next iteration start to read at the next position
            break; //go to the next row in the array
            }

            argvlist[count][j] = args[pos]; // if a normal argv to read, read it
            pos++; //next iteration start to read at the next position

        }

	}




	// count how many pipes/child processes do we need
	for (int i = 0; i < (argc-1); i++) {
        if (strcmp(args[i], "|") == 0)
		num_pipes++; 
    }
	

	// the parent-child piping

	// the first n-1 argument segments
	// create a pipe
	// fork a child to execute current argv segment
	// in parent, save the read end of pipe to a temp fd for next round's child to use; need to close the old temp fd in parent process starting from the 2nd round



	int prev_in=0;
	for (int i=0; i<num_pipes; i++){
		int fd[2];
		pipe(fd);

		if((child_pid=fork())<0){
			//error
			printf("fork() error \n");
		}
		else if(child_pid==0){
			//child
			close(fd[0]);//close read end of current pipe
			
			if(i>0){
				close(0);//close stdin
				dup(prev_in);//redirect stdin to prev_in to get the output of last pipe			
			}	
			close(1);//close stdout
			dup(fd[1]);//redirect stdout to writing end of current pipe
			//execvp
			if(fork()==0){
				if(execvp(argvlist[segment_index][0],argvlist[segment_index])<0){
					printf("execvp() error \n");
					exit(-1);
				}
			}
			else{
				wait(NULL);
			}
			exit(0);
			
		}
		else{
			//parent
			close(fd[1]);//close write end of current pipe
			if(i!=0){
				close(prev_in); //if it is not the 1st pipe argument segment, close the previous fd;
			}
			prev_in=fd[0];//save the reading end to prev_fd for next round's process to get
			//maybe no need to close fd[0], cuz the same file table entry is pointed by prev_in? and the while iteration will reset the value of fd[0]
			wait(NULL); //wait for the current child to terminate
			segment_index++;
		}
		

	}

	// the last argument segment
	// print out in the console
	// no redirection of stdout && no need to save the current pipe reading end for next iteration
	// need to store the stdin first and restore it after argv segment execution
	int temp_in = dup(0); // store the stdin temporarily	
	close(0);
	dup(prev_in); // redirect the stdin to writing end of last pipe
	
	//execvp
	if((child_pid=fork())<0){
		//error
			printf("fork() error \n");		
	}
	else if(child_pid==0){
		if(execvp(argvlist[num_pipes][0],argvlist[num_pipes])<0){
					printf("execvp() error \n");
					exit(-1);
				}

	}
	else{
		wait(NULL);
	}
	
	close(prev_in);
	close(0);
	dup(temp_in); //restore the stdin


// old codes provided by TA

/*

	if((child_pid = fork()) < 0)
	{
		printf("fork() error \n");
	}
	else if (child_pid == 0)
	{
		if ( execvp(args[0], args) < 0)
		{ 
			printf("execvp() error \n");
			exit(-1);
		}
	}
	else
	{
		if ((wait_return = wait(&status)) < 0)
			printf("wait() error \n"); 
	}

*/



	return 0;

}
