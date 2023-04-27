#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h> 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

char* dir1;
char* dir2;

char* convert_path(char* pathname) {
  printf("convert_path called.\n");
  char path[200] = "";
  strcat(path, dir2);
  for (int k = strlen(dir1); k < strlen(pathname); k++)
    path[strlen(dir2) - strlen(dir1) + k] = pathname[k];
  char* result;
  result = path;
  printf("convert_path worked fine. New path = %s\n", path);
  return result;
}
void gzip(char* pathname){
  int res = 0;
  res = fork();
  if (res == 0)
    execl("/bin/gzip", "/bin/gzip", "-f", pathname);
  // Waiting to eliminate race condition
  return;
}
void copy_file(char* pathname) {
  printf("copy_file called.\n");
  char path[200] = "";
  strcpy(path, convert_path(pathname));
  // Counting slashes in the pathname
  int slash_count = 0;
  for (int i = 0; i < strlen(path); i++)
    if (path[i] == '/')
      slash_count++;
  int k = 0;
  // Making each directory on the path to the copied file.
  // Basically, I've chosen EAFP instead of LBYL.
  // (Easier to Ask Forgiveness than Permission) - the "mkdir" way
  // (Look Before You Leap) - the "stat" way
  char cur_path[200] = "";
  while (slash_count > 0) {
    if (path[k] == '/') {
      	slash_count--;
      	int res = 0;
      res = fork();
      if (res == 0) {
        printf("Tryin' to create a directory called %s\n", cur_path);
        if (execl("/bin/mkdir", "/bin/mkdir", cur_path) < 0)
          if (errno != EEXIST) {
            printf("Failed to create a directory at %s.\n", cur_path);
            exit(-1);
          }
        exit(-1);
    }
      // Waiting a bit before trying to create the next directory
      // to avoid "race condition" since my program
      // calls lots of different processes
      waitpid(res, NULL, WNOHANG);
    }
    char temp[2] = "";
    temp[1] = '\0';
    temp[0] = path[k];
    strcat(cur_path, temp);
    k++;
  }
  // Waiting for all the directories to get created
  // Not using sleep so as not to wait for hours on end each time
  sleep(1);
  // Copying the actual file
  int res = 0;
  res = fork();
  if (res == 0)
    if (execl("/bin/cp", "/bin/cp", pathname, path) < 0) {
      printf("Failed to copy file at %s\n", path);
      exit(-1);
    }
  // Waiting to eliminate race condition. Yes, again.
  waitpid(res, NULL, WNOHANG);
  // Calling gzip() to zip the copied file.
  printf("Trying to call gzip with path = %s\n", path);
  gzip(path);

  printf("Copy_file worked fine.\n");
}

int find_file(char* pathname) {
  printf("Find_file called.\n");
  struct stat stat_info;
  char path[200] = "";
  strcpy(path, convert_path(pathname));
  strcat(path, ".gz");
  // Using lstat() to find the file or learn that
  // it doesn't exist
  if (lstat(path, &stat_info) < 0) {
    if ((errno == ENOENT)||(errno == ENOTDIR))
      return 0;
    else if(errno == EACCES) {
      printf("No permission to access file %s\n", pathname);
    }
    else{
      printf("Failed to use lstat.\n");
      exit(-1);
    }
  }
  return 1;
}

int mtime_check(char* pathname) {
  printf("mtime_check called.\n");
  struct stat stat_info1;
  struct stat stat_info2;
  char* path = convert_path(pathname);
  strcat(path, ".gz");
  // Using lstat to check modify_time of the original and the copy
  // and compare them
  if (lstat(path, &stat_info1) < 0) {
    printf("Failed to use lstat at %s.\n", path);
    exit(-1);
  }
  if (lstat(pathname, &stat_info2) < 0) {
    printf("Failed to use lstat at %s.\n", pathname);
    exit(-1);
  }

  time_t mtime1 = stat_info1.st_mtime;
  time_t mtime2 = stat_info2.st_mtime;

  printf("mtime_check worked fine.\n");
  if (mtime2 > mtime1)
    return 0;
  return 1;
}

void rec_search(char* pathname){
  // Making the argument for execl()
  char argh[200] = "ls -l ";
  strcat(argh, pathname);
  strcat(argh, " > ls.txt");
  printf("%s\n", argh);
  printf("Seg fault not at strcat.\n");
  int res = 0;
  res = fork();
  if (res == 0) {
    if (execl("/bin/bash", "/bin/bash", "-c", argh, (char*)NULL) < 0) {
      printf("Failed to use ls.\n");
      exit(-1);
    }
  }
  if (res == 0)
    printf("Seg fault not at /bin/bash.\n");
  waitpid(res, NULL, WNOHANG);
  //Using "ls -l > ls.txt" to get directory contents.
  //Then reading the file for said contents.
  int fd3 = 0;
  if ((fd3 = open("ls.txt", O_RDONLY, 0666)) < 0) {
    printf("Failed to open ls result file.\n");
    exit(-1);
  }
  printf("Seg fault not at open.\n");
  //Reading from the file into a buffer
  char buf[5000];
  if (read(fd3, buf, 5000) < 0) {
    printf("Failed to read from ls result file.\n");
    exit(-1);
  }
  printf("Seg fault not at read.\n");
  //Closing the file, so that we can write into it
  //for the next directory
  if (close(fd3) < 0) {
    printf("Failed to close ls result file.\n");
    exit(-1);
  }
  //Parsing ls results with strtok()
  //(into 200 lines max of 100 characters max)
  char lines[200][100];
  char* token;
  int line_count = 0;
  printf("Seg fault not at lines creation.\n");
  token = strtok(buf, "\n");
  while (token != NULL) {
    strcpy(lines[line_count], token);
    token = strtok(NULL, "\n");
    line_count++;
  }
  
  printf("Seg fault not at parsing.\n");
  // Making a list of all directory/file pathnames found
  // Parsing them myself this time because I could
  char dirs[200][200];
  int dir_count = 0;
  char files[200][200];
  int file_count = 0;
  for (int i = 1; i < line_count; i++) {
    if (lines[i][0] == 'd') {
      char path[200] = "";
      int length = strlen(lines[i]);
      int k = length - 1;
      for (; lines[i][k] != ' '; k--);
      k++;
      int j = 0;
      for (; k < length; k++) {
        path[j] = lines[i][k];
        j++;
      }
      strcpy(dirs[dir_count], path);
      dir_count++;
    }
    else {
      char path[200] = "";
      int length = strlen(lines[i]);
      int k = length - 1;
      for (; lines[i][k] != ' '; k--);
      k++;
      int j = 0;
      for (; k < length; k++) {
        path[j] = lines[i][k];
        j++;
      }
      strcpy(files[file_count], path);
      file_count++;
    }
  }
  printf("Seg fault not at big parsing section.\n");
  //Yet another debug check
  printf("dir_count = %d.\n",dir_count);
  for (int i = 0; i < dir_count; i++)
    printf("Dir: %s\n", dirs[i]);
  printf("file_count = %d.\n", file_count);
  for (int i = 0; i < file_count; i++)
    printf("File: %s\n", files[i]);
  printf("line_count = %d.\n", line_count);
  for (int i = 0; i < line_count; i++)
    printf("Line: %s\n", lines[i]);
  //Working with the files
  for (int i = 0; i < file_count; i++) {
    char new_path[200] = "";
    strcpy(new_path, pathname);
    strcat(new_path, "/");
    strcat(new_path, files[i]);
    printf("Checking file path = %s...\n", new_path);
    if (find_file(new_path) == 0){
      copy_file(new_path);
      printf("File didn't exist. Copied file.\n");
    }
    else if(mtime_check(new_path) == 0){
      copy_file(new_path);
      printf("File wasn't up to date. Copied file.\n");
    }
  }
  printf("Copied all files in the directory.\n");
  //RECURSION BLOCK
  //Calling recursive search function for every directory in
  // the current one
  for (int i = 0; i < dir_count; i++){
    char path[200] = "";
    strcpy(path, pathname);
    strcat(path, "/");
    strcat(path, dirs[i]);
    printf("%s\n", path);
    rec_search(path);
  }
  //RECURSION BLOCK

}

int main(int argc, char* argv[]){
  //Checking the amount of arguments
  if (argc != 3) {
    printf("Wrong number of arguments.\n");
    exit(-1);
  }
  dir1 = argv[1];
  dir2 = argv[2];
  //Calling the main function of the program
  rec_search(dir1);
  printf("Backed up succesfully.\n");

  return 0;
}
