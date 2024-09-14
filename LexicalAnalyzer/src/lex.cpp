#include <iostream>
#include <string>
#include "../include/lex.h"
#include "../include/errors.h"

using namespace std;

LexItem id_or_kw(string &lexeme, int linenum) {
    Token token;
    if(lexeme[0] == '$')
        token = NIDENT;
    else if(lexeme[0] == '@')
        token = SIDENT;
    else
        token = IDENT;

    return LexItem(token, lexeme, linenum);
}

string tokenToString(Token token) {
    map<Token, string> tokenMap = {
            {WRITELN, "WRITELN"},
            {IF, "IF"},
            {ELSE, "ELSE"},
            {IDENT, "IDENT"},
            {NIDENT, "NIDENT"},
            {SIDENT, "SIDENT"},
            {ICONST, "ICONST"},
            {RCONST, "RCONST"},
            {SCONST, "SCONST"},
            {PLUS, "PLUS"},
            {MINUS, "MINUS"},
            {MULT, "MULT"},
            {DIV, "DIV"},
            {EXPONENT, "EXPONENT"},
            {ASSOP, "ASSOP"},
            {LPAREN, "LPAREN"},
            {RPAREN, "RPAREN"},
            {LBRACES, "LBRACES"},
            {RBRACES, "RBRACES"},
            {NEQ, "NEQ"},
            {NGTHAN, "NGTHAN"},
            {NLTHAN, "NLTHAN"},
            {CAT, "CAT"},
            {SREPEAT, "SREPEAT"},
            {SEQ, "SEQ"},
            {SLTHAN, "SLTHAN"},
            {SGTHAN, "SGTHAN"},
            {COMMA, "COMMA"},
            {SEMICOL, "SEMICOL"},
            {ERR, "ERR"},
            {DONE, "DONE"}
    };
    auto it = tokenMap.find(token);
    if (it != tokenMap.end())
        return it->second;
}

bool isSpecialChar(char c) {
    string specialChars = "!@#%^&()=";
    for(auto &s : specialChars)
    {
        if(c == s)
            return true;
    }

    return false;
}

LexItem getNextToken(istream &in, int &linenum) {
    enum State{START, INDIV, ININT, INREAL, INID, INSTRING, INCOMMENT, INCOMPARE};

    char c;
    string lexeme;

    State state = START;
    while(in.get(c))
    {
        switch(state)
        {
            case START:
            {
                if (c == ' ' || c == '\t')
                    continue;

                else if (c == '=')
                {
                    if (in.peek() == '=')
                    {
                        in.get(c);
                        return LexItem(NEQ, "==", linenum);
                    }

                    else
                        return LexItem(ASSOP, "=", linenum);
                }
                else if (c == ';')
                    return LexItem(SEMICOL, ";", linenum);

                else if (c == '(')
                    return LexItem(LPAREN, "(", linenum);

                else if (c == ')')
                    return LexItem(RPAREN, ")", linenum);

                else if (c == '+')
                    return LexItem(PLUS, "+", linenum);

                else if (c == '-')
                {
                    char nextChar = tolower(in.peek());
                    if(nextChar == 'e' || nextChar == 'l' || nextChar == 'g')
                        state = INCOMPARE;
                    else
                        return LexItem(MINUS, "-", linenum);
                }

                else if (c == '*')
                {
                    if(in.peek() == '*')
                    {
                        in.get(c);
                        return LexItem(SREPEAT, "**", linenum);
                    }
                    return LexItem(MULT, "*", linenum);
                }

                else if (c == '/')
                    return LexItem(DIV, "/", linenum);

                else if (c == '^')
                    return LexItem(EXPONENT, "^", linenum);

                else if (c == '<')
                    return LexItem(NLTHAN, "<", linenum);

                else if (c == '>')
                    return LexItem(NGTHAN, ">", linenum);

                else if (c == '.')
                    return LexItem(CAT, ".", linenum);

                else if (c == '{')
                    return LexItem(LBRACES, "{", linenum);

                else if (c == '}')
                    return LexItem(RBRACES, "}", linenum);

                else if (c == ',')
                    return LexItem(COMMA, ",", linenum);

                else if (c == '#')
                {
                    state = INCOMMENT;
                    continue;
                }
                else if (c == '\n')
                {
                    linenum++;
                    state = START;
                }
                else if (isalpha(c) || c == '_' || c == '$' || c == '@')
                {
                    lexeme += c;
                    state = INID;
                    if (isSpecialChar(in.peek()))
                    {
                        string lex(1, c);
                        return id_or_kw(lex, linenum);
                    }
                }
                else if (isdigit(c))
                {
                    lexeme += c;
                    state = ININT;
                }
                else if (c == '\'')
                    state = INSTRING;

                else
                {
                    errorState = GEN_ERR;
                    string lex(1, c);
                    return LexItem(ERR, lex, linenum);
                }
                break;
            }

            case INID:
            {
                if(isalnum(c) && isSpecialChar(in.peek()))
                {
                    lexeme += c;
                    return id_or_kw(lexeme, linenum);
                }

                else if(isalnum(c) || c == '_')
                    lexeme += c;

                else
                {
                    if(isSpecialChar(c))
                    {
                        errorState = INID_ERR;
                        string err;
                        return LexItem(ERR, err += c, linenum);
                    }
                    else
                    {
                        in.putback(c);
                        return id_or_kw(lexeme, linenum);
                    }
                }
                break;
            }

            case ININT:
            {
                if(isdigit(c))
                    lexeme += c;

                else if(c == '.')
                {
                    lexeme += c;
                    state = INREAL;
                }
                else
                {
                    in.putback(c);
                    return LexItem(ICONST, lexeme, linenum);
                }
                break;
            }

            case INSTRING:
                if(c == '\n')
                {
                    errorState = INSTRING_ERR;
                    return LexItem(ERR, lexeme, linenum);
                }

                if(c == '\'')
                    return LexItem(SCONST, lexeme, linenum);

                lexeme += c;

                break;

            case INREAL:
                if(isdigit(c))
                    lexeme += c;

                else if(c == '.')
                {
                    errorState = INREAL_ERR;
                    return LexItem(ERR, lexeme + c, linenum);
                }
                else
                {
                    in.putback(c);
                    return LexItem(RCONST, lexeme, linenum);
                }
                break;

            case INCOMMENT:
            {
                if(c == '\n')
                {
                    linenum++;
                    state = START;
                }
                break;
            }
            case INCOMPARE:
            {
                if (tolower(c) == 'e' && tolower(in.peek()) == 'q')
                {
                    in.get(c);
                    return LexItem(SEQ, "-eq", linenum);
                }
                else if (tolower(c) == 'l' && tolower(in.peek()) == 't')
                {
                    in.get(c);
                    return LexItem(SLTHAN, "-lt", linenum);
                }
                else if (tolower(c) == 'g' && tolower(in.peek()) == 't')
                {
                    in.get(c);
                    return LexItem(SGTHAN, "-gt", linenum);
                }
                else
                    return LexItem(MINUS, "-", linenum);
            }
        }
    }
    return LexItem(DONE, "", linenum);
}

ostream &operator<<(ostream &out, const LexItem &lexItem) {
    out << tokenToString(lexItem.getToken()) << "(" << lexItem.getLexeme() << ")";
    return out;
}
