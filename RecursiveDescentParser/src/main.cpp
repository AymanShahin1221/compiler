#include <iostream>
#include <fstream>

#include "../include/parser.h"

#include "../include/line.h"

using namespace std;

int lineNumber = 1;

int main(int argc, char *argv[]) {

    istream *in = nullptr;
    ifstream file;

    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];

        if (in != nullptr)
        {
            cerr << "ONLY ONE FILE NAME ALLOWED" << endl;
            return 0;
        }
        else
        {
            file.open(arg.c_str());
            if (!file.is_open())
            {
                cerr << "CANNOT OPEN " << arg << endl;
                return 0;
            }
            in = &file;
        }
    }
    if (argc == 1)
    {
        cerr << "Missing File Name." << endl;
        return 0;
    }
//    int lineNumber = 1;
    bool status = Prog(*in, lineNumber);

    if (!status)
        cout << "Unsuccessful Parsing" << endl << "Number of Syntax Errors " << ErrCount() << endl;

    else
    {
        cout << "(DONE)" << endl;
        cout << "Successful Parsing" << endl;
    }

    return 0;
}
