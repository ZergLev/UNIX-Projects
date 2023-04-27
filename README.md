# UNIX-Projects
Some of my basic UNIX Projects

"backup.c" makes a copy of each file modified since the last backup, then zips al the files into .gz files

To-do list: Replace the "sleep(1)" commands with waitpid() function. Remove debug output.

"taskm.c" lets the user run a number of commands in the command line. The number of commands allowed is specified in the command line arguments.
This program can actually run itself and .exe files just fine.

To-do list: Add --list and --kill commands. --list should be done by reusing previous code that checks for child processes. --kill could just be done with the "kill" system call. But in order to  secure permissions to kill the processes it could be done by the program itself.
