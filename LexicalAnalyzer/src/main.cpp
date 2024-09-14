#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include "../include/lex.h"
#include "../include/errors.h"

bool isFileEmpty(const string &filename) {
    ifstream file(filename);
    return file.peek() == ifstream::traits_type::eof();
}

ErrorState errorState = NO_ERR;
int main(int argc, char **argv) {

// ============================================================================
//                      handle command line args
// ============================================================================

    int fileCount = 0;
    for(int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if(arg[0] != '-')
            fileCount++;
    }

    if(fileCount > 1)
    {
        cout << "ONLY ONE FILE NAME IS ALLOWED." << endl;
        exit(1);
    }

    string fname = argc >= 2 ? argv[1] : "";

    fstream file;
    file.open(fname, ios::in);

    if(fname.empty())
    {
        cout << "NO SPECIFIED INPUT FILE NAME." <<  endl;
        exit(1);
    }

    if(!file.is_open())
    {
        cout << "CANNOT OPEN THE FILE " << fname << endl;
        exit(1);
    }

    if(isFileEmpty(fname))
    {
        cout << "Lines: 0" << endl;
        cout << "Empty File." << endl;
        exit(1);
    }

    string validFlags[] = {"-v", "-nconst", "-sconst", "-ident"};
    vector<string> seenFlags;
    for(int i = 2; i < argc; i++)
    {
        bool isValid = false;
        for(const auto &flag : validFlags)
        {
            if(argv[i] == flag)
            {
                isValid = true;
                seenFlags.push_back(argv[i]);
                break;
            }
        }

        if(!isValid)
        {
            cout << "UNRECOGNIZED FLAG {" << argv[i] << "}" << endl;
            exit(1);
        }
    }

    bool _v = false;
    bool _nconst = false;
    bool _sconst = false;
    bool _ident = false;

    for(auto &flag : seenFlags)
    {
        if(flag == "-v")
            _v = true;
        else if(flag == "-nconst")
            _nconst = true;
        else if(flag == "-sconst")
            _sconst = true;
        else if(flag == "-ident")
            _ident = true;
    }

// ============================================================================
//                     parse input file
// ============================================================================

    int linenum = 1;
    int totalTokens = 0;

    set<string> identifiers;
    set<double> numbers;
    set<string> strings;

    LexItem lexItem;
    while(lexItem.getToken() != DONE)
    {
        lexItem = getNextToken(file, linenum);
//        cout << "Token: " << tokenToString(lexItem.getToken()) << " \t\tLexeme: " << lexItem.getLexeme() << "\t\tLine number: " << lexItem.getLinenum() << endl;

        Token tok = lexItem.getToken();
        if(tok == DONE)
        {
            linenum--;
            break;
        }
        if(_v)
        {
            if(tok == ASSOP || tok == SEMICOL|| tok == LPAREN|| tok == RPAREN || tok == LBRACES || tok == RBRACES || tok == COMMA || tok == MINUS || tok == CAT || tok == MULT || tok == PLUS ||
               tok == DIV || tok == EXPONENT || tok == NEQ || tok == SEQ || tok == NGTHAN || tok == NLTHAN || tok == SREPEAT || tok == SLTHAN || tok == SGTHAN)

                cout << tokenToString(tok) << endl;

            else if(tok == ERR)
            {
                if(errorState == INSTRING_ERR)
                    cout << "Error in line " << lexItem.getLinenum() << " ('" << lexItem.getLexeme() << ")" << endl;

                else if(errorState == GEN_ERR || errorState == INID_ERR || errorState == INREAL_ERR)
                    cout << "Error in line " << lexItem.getLinenum() << " (" << lexItem.getLexeme() << ")" << endl;

                exit(1);
            }
            else
                cout << lexItem << endl;
        }

        if(tok == IDENT || tok == NIDENT || tok == SIDENT)
            identifiers.insert(lexItem.getLexeme());

        if(tok == ICONST || tok == RCONST)
            numbers.insert(stod(lexItem.getLexeme()));

        if(tok == SCONST)
            strings.insert(lexItem.getLexeme());

        totalTokens++;
    }

// ============================================================================
//                      print summary
// ============================================================================

    if(_v)
        cout << endl;

    cout << "Lines: " << linenum << endl;
    cout << "Total Tokens: " << totalTokens << endl;
    cout << "Identifiers: " << identifiers.size() << endl;
    cout << "Numbers: " << numbers.size() << endl;
    cout << "Strings: " << strings.size() << endl;

    if(_ident)
    {
        cout << "IDENTIFIERS:" << endl;
        auto it = identifiers.begin();
        while (it != identifiers.end())
        {
            cout << *it;
            if (++it != identifiers.end())
                cout << ", ";
        }
        cout << endl;
    }

    if(_nconst)
    {
        cout << "NUMBERS:" << endl;
        auto it2 = numbers.begin();
        while (it2 != numbers.end())
        {
            cout << *it2 << endl;
            it2++;
        }
    }

    if(_sconst)
    {
        cout << "STRINGS:" << endl;
        auto it3 = strings.begin();
        while (it3 != strings.end())
        {
            cout << "\'" << *it3 << '\'' << endl;
            it3++;
        }
    }

    return 0;
}


