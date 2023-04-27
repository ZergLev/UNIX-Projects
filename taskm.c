#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <wait.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sched.h>

#define MAX_LEN 100

#define STACK_SIZE (1024 * 1024)

char child_stack[STACK_SIZE];


int main(int argc, char* argv[]) {
  if (argc != 2) {
    printf("Wrong number of arguments.\n");
    exit(-1);
  }
  int max_commands = atoi(argv[1]);
  /*Main cycle of the program*/
  char input[MAX_LEN] = "";
  while (1) {
    /*Reading the command from standard input*/
    printf("Please, input the command.\n");
    char temp_str[200] = "";
    fgets(temp_str, 200, stdin);
    temp_str[strlen(temp_str) - 1] = '\0';
    strcpy(input, temp_str);
    int length = strlen(input);
    /*Checking for --list option*/
    int list_flag = 1;
    char list[200];
    strcpy(list, "");
    strcat(list,"--list");
    for (int k = 0; input[k] != '\0'; k++) {
      if (input[k] != list[k])
        list_flag = 0;
    }
    /*Counting child processes with "ps" system call.*/
    /*First, using the console command writing the results into a file*/
    int res = 0;
    res = fork();
    if (res == 0) {
      if (execl("/bin/bash", "/bin/bash", "-c", "ps -ef > res.txt", (char*)NULL) < 0) {
        printf("Failed to use ps.\n");
        exit(-1);
      }  
    }
    sleep(1);
    int fd;
    if ((fd = open("res.txt", 0666 | O_RDWR)) < 0) {
      printf(".txt file with ps results not found.\n");
      exit(-1);
    }
    /*Reading the results and parsing them with strtok.*/
    char text[5000] = "";
    if (read(fd, text, 5000) < 0) {
      printf("Reading from ps result file failed.\n");
      exit(-1);
    }
    char lines[200][200];
    char* token;
    int line_count = 0;
    token = strtok(text, "\n");
    while (token) {
      strcpy(lines[line_count], token);
      token = strtok(NULL, "\n");
      line_count++;
    }
    /*The actual counting begins.*/
    int kid_count = 0;
    int cur_pid = getpid();
    for (int i = 1; i < line_count; i++) {
      /*Skipping 2 words and all whitespaces after them, reading only PPID.*/
      int j = 0;
      char PPID[20];
      strcpy(PPID, "");
      for (; lines[i][j] != ' '; j++);
      for (; lines[i][j] == ' '; j++);
      for (; lines[i][j] != ' '; j++);
      for (; lines[i][j] == ' '; j++);
    
      for (; lines[i][j] != ' '; j++) {
        char symbol[2];
        symbol[0] = lines[i][j];
        symbol[1] = '\0';
        strcat(PPID, symbol);
      }
      /*We have to check if a process is a child of this process
        and if it is defunct.*/
      int parent = atoi(PPID);
      if (parent == cur_pid) {
	if (list_flag) {
	  char print[200];
	  strcpy(print, "");
	  int p = 0;
	  for (; lines[i][p] != '\0'; p++)
		  print[p] = lines[i][p];
	  p++;
	  print[p] = '\0';
	  printf("%s\n", print);
	}
        char last_word[200];
        strcpy(last_word, "");
        int k = strlen(lines[i]) - 1;
        for (; lines[i][k] == ' '; k--);
        for (; (lines[i][k] != '<') && (lines[i][k] != ' '); k--);
        for (; (lines[i][k] != '>') && (lines[i][k] != ' '); k++) {
          char symbol[2];
          symbol[0] = lines[i][k];
          symbol[1] = '\0';
          strcat(last_word, symbol);
        }
        char symbol[2];
        symbol[0] = lines[i][k];
        symbol[1] = '\0';
        strcat(last_word, symbol);
        
          char defunct[200];
          strcpy(defunct, "<defunct>");
          
    /*If it's defunct, delete it. If not, count it.*/
          if (strcmp(last_word, defunct))
            kid_count++;
          else {
            for (j = 0; lines[i][j] != ' '; j++);
            for (; lines[i][j] == ' '; j++);
            char PID[20];
            strcpy(PID, "");
            for (; lines[i][j] != ' '; j++) {
              symbol[0] = lines[i][j];
              symbol[1] = '\0';
              strcat(PID, symbol);
            }
            waitpid(atoi(PID), NULL, WNOHANG);
          }
      }
    }


    /*We need to count the "ps -ef > res.txt" process too,
     it's called every time and counted everytime since it is not defunct,
     so we need to subtract 1.*/
    kid_count--;
    /*Checking the number of running programs. If conditions aren't satisfied
      using continue to jump to next "while" cycle iteration.*/
    if (kid_count >= max_commands) {
      printf("Already executing %d commands, request denied.\n", kid_count);
      continue;
    }
    int kill_flag = 1;
    char kill[200];
    strcpy(kill, "");
    strcat(kill,"--kill");
    for (int k = 0; input[k] != ' '; k++) {
      if (input[k] != kill[k])
        kill_flag = 0;
    }
    int p = 0;
    if (kill_flag) {
	char com[200];
	strcpy(com, input);
	strcpy(input, "");
	for (int k = 2; com[k] != '\0'; k++){
	    input[p] = com[k];
	    p++;
	}
      input[p] = '\0';
    
    }
    /*Executing the required program if conditions met.*/
    if (!list_flag) {
      res = 0;
      res = fork();
      if (res == 0) {
        if (execl("/bin/bash", "/bin/bash", "-c", input, (char*)NULL) < 0) {
          printf("Failed to execute console command.\n");
          exit(-1);
        }
      }
    /*If EndOfFile is found, finishes program*/
      if (input[length] == EOF)
      break;
    }
  }
  printf("Finished program.\n");
  return 0;
}
