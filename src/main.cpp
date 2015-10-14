#include<iostream>
using namespace std;

extern "C" {
    int yylex(void);
}

int main(int argc, char* argv [])
{
    cout << "Hello World!!" << endl;
    yylex();
    return 0;
}
