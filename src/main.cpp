#include "cmd.h"

int main(int argc, char* argv [])
{
    if (argc >= 2) {
        cmd_readfile(argv[1]);
    } else {
        cmd_readline();
    }

    return 0;
}

