# UNIX-Projects
Some of my basic UNIX Projects

"backup.c" makes a copy of each file modified since the last backup, then zips al the files into .gz files

To-do list: - Replace the "sleep(1)" command at directory creation with a waitpid() function.
            - Remove debug output.
            - Remove the checking for an existing directory function(The dir creation function can do it itself and not throw an error)

"taskm.c" lets the user run a number of commands in the command line. The number of commands allowed is specified in the command line arguments.
This program can actually run itself and .exe files just fine.

To-do list: Regulate permissions for --kill option.
