Build TinySQL:
    Type “make” under the project folder “tinysql/” to build the project.
    The executable file “tinysql” will be in “bin/” folder after compilation.

    -----------------------------------------------------------
    shell> cd tinysql
    shell> make
    -----------------------------------------------------------

Execute TinySQL:
    Interactive Mode:
        To execute TinySQL in interactive mode, type “./bin/tinysql ” in
        folder “./tinysql”. The prompt “tinysql>” indicates that you can start using
        TinySQL now. If you want to stop TinySQL, use command “quit” to exit.

        -----------------------------------------------------------
        shell> ./bin/tinysql
        tinysql>
        ...
        tinysql> quit
        Bye
        -----------------------------------------------------------

    Batch Mode:
        To execute TinySQL in batch mode, type “./bin/tinysql” and follow by the
        name of the file, in which are commands you want to execute. TinySQL
        will exit automatically after encounting errors or finishing execution.

        -----------------------------------------------------------
        shell> cd tinysql
        shell> ./bin/tinysql $FILE
        -----------------------------------------------------------

    Else:
        All the results will be display on screen by standard output. If you want
        to dump the results to a file, please redirect the output.

        -----------------------------------------------------------
        shell> ./bin/tinysql $FILE > $OUT_FILE
        -----------------------------------------------------------

        To run multiple commands without leaving, use the “source” command in interactive mode.

        -----------------------------------------------------------
        shell> ./bin/tinysql
        tinysql> source $FILE
        ...
        tinysql>
        -----------------------------------------------------------
    
