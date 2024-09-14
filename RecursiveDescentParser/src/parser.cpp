#include "../include/parser.h"

#include "../include/line.h"

map<string, bool> defVar;

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream &in, int &line) {
		if(pushed_back)
        {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem &t) {
		if(pushed_back)
			abort();

		pushed_back = true;
		pushed_token = t;
	}
}

LexItem skipToken(istream &in, int &line) {
    LexItem lexItem;
    for(int i = 0; i < 2; i++)
        lexItem = Parser::GetNextToken(in, line);

    return lexItem;
}

static int error_count = 0;

int ErrCount() {
    return error_count;
}

void ParseError(int line, const string &msg) {
	++error_count;
	cout << error_count << ". Line # " << line << ": " << msg << endl;
}

// Prog ::= StmtList
bool Prog(istream &in, int &line) {
    bool status = StmtList(in, line);
    if(!status)
    {
        ParseError(line, "Missing Program");
        return false;
    }
    return true;
}

// StmtList ::= Stmt; { Stmt; }
bool StmtList(istream &in, int &line) {

    bool status = Stmt(in, line);
    if(!status)
    {
        ParseError(line, "Syntactic error in Program Body.");
        return false;
    }

    LexItem lexItem = Parser::GetNextToken(in, line);
    if(lexItem.getToken() != SEMICOL)
    {
        ParseError(line, "Missing semicolon at end of Statement.");
        return false;
    }

    lexItem = Parser::GetNextToken(in, line);
    while (lexItem == IDENT || lexItem == SIDENT || lexItem == NIDENT || lexItem == WRITELN || lexItem == IF)
    {
        Parser::PushBackToken(lexItem);
        if (!Stmt(in, line))
        {
            ParseError(line, "Syntactic error in Program Body.");
            return false;
        }

        lexItem = Parser::GetNextToken(in, line);
        if (lexItem != SEMICOL)
        {
            ParseError(line, "Missing semicolon at end of Statement.");
            return false;
        }
        lexItem = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(lexItem);
    return true;
}

// Stmt ::= AssignStmt | WriteLnStmt | IfStmt
bool Stmt(istream &in, int &line) {
    LexItem lexItem = Parser::GetNextToken(in, line);
    bool status;
    if(lexItem == IDENT || lexItem == SIDENT || lexItem == NIDENT)
    {
        Parser::PushBackToken(lexItem);

        status = AssignStmt(in, line);
        if(!status)
        {
            ParseError(line, "Incorrect Assignment Statement.");
            return false;
        }
        else
            return true;
    }
    else if(lexItem == WRITELN)
    {
        Parser::PushBackToken(lexItem);

        status = WritelnStmt(in, line);
        if(!status)
        {
            ParseError(line, "Incorrect Writeln Statement.");
            return false;
        }
        else
            return true;
    }
    else if(lexItem == IF)
    {
        Parser::PushBackToken(lexItem);
        status = IfStmt(in, line);

        if(!status)
        {
            ParseError(line, "Incorrect If-Statement.");
            return false;
        }
        else
            return true;
    }
    else
    {
        ParseError(line, "Invalid Statement");
        return false;
    }
}

// WritelnStmt:= WRITELN (ExprList)
bool WritelnStmt(istream &in, int &line) {
    LexItem lexItem = skipToken(in, line);

    if(lexItem != LPAREN)
    {
        ParseError(line, "Missing Left Parenthesis of Writeln Statement");
        return false;
    }

    bool status = ExprList(in, line);

    if(!status)
    {
        ParseError(line, "Missing expression list after Print");
        return false;
    }

    lexItem = Parser::GetNextToken(in, line);
    if(lexItem != RPAREN)
    {
        ParseError(line, "Missing Right Parenthesis of Writeln Statement");
        return false;
    }
    return true;
}
// IfStmt ::= if (Expr) ‘{‘ StmtList ‘}’ [ else ‘{‘ StmtList ‘}’ ]
bool IfStmt(istream &in, int &line) {

    LexItem lexItem = skipToken(in, line);

    if(lexItem != LPAREN)
    {
        ParseError(line, "Missing left parenthesis in if statement");
        return false;
    }
    bool status = Expr(in, line);
    if(!status)
        return false;

    lexItem = Parser::GetNextToken(in, line);
    if(lexItem != RPAREN)
    {
        ParseError(line, "Missing Right Parenthesis of If condition");
        return false;
    }
    lexItem = Parser::GetNextToken(in, line);
    if(lexItem != LBRACES)
    {
        ParseError(line, "If Statement Syntax Error: Missing left brace");
        return false;
    }
    status = StmtList(in, line);
    if(!status)
        return false;

    lexItem = Parser::GetNextToken(in, line);
    if(lexItem != RBRACES)
    {
        ParseError(line, "Missing right brace.");
        ParseError(line, "Missing Statement for If-Stmt Clause");
        return false;
    }
    lexItem = Parser::GetNextToken(in, line);
    if(lexItem == ELSE)
    {
        lexItem = Parser::GetNextToken(in, line);
        if(lexItem != LBRACES)
        {
            ParseError(line, "Else Statement Syntax Error: Missing left brace");
            return false;
        }
        status = StmtList(in, line);
        if(!status)
            return false;

        lexItem = Parser::GetNextToken(in, line);
        if(lexItem != RBRACES)
        {
            ParseError(line, "Else Statement Syntax Error: Missing right brace.");
            return false;
        }
    }
    else
        Parser::PushBackToken(lexItem);

    return true;

}

// AssignStmt ::= Var = Expr
bool AssignStmt(istream &in, int &line) {
    bool status = Var(in, line);
    if(!status)
        return false;

    LexItem lexItem = Parser::GetNextToken(in, line);
    if(lexItem != ASSOP)
    {
        ParseError(line, "Missing Assignment Operator");
        return false;
    }
    status = Expr(in, line);
    if(!status)
    {
        ParseError(line, "Missing Expression in Assignment Statement");
        return false;
    }

    return true;
}

// Var ::= NIDENT | SIDENT
bool Var(istream &in, int &line) {
    LexItem lexItem = Parser::GetNextToken(in, line);

    if(lexItem == NIDENT || lexItem == SIDENT)
    {
        string identifier = lexItem.getLexeme();
        defVar[identifier] = true;
        return true;
    }
    else
    {
        ParseError(line, "Invalid variable name");
        return false;
    }
}


// ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status;
	status = Expr(in, line);
	if(!status)
    {
		ParseError(line, "Missing Expression");
		return false;
	}

	LexItem tok = Parser::GetNextToken(in, line);

	if (tok == COMMA)
		status = ExprList(in, line);

	else if(tok.getToken() == ERR)
    {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.getLexeme() << ")" << endl;
		return false;
	}
	else
    {
		Parser::PushBackToken(tok);
		return true;
	}
	return status;
}

// Expr ::= RelExpr [(-eq|==) RelExpr ]
bool Expr(istream &in, int &line) {
    bool status = RelExpr(in, line);
    if(!status)
        return false;

    LexItem lexItem = Parser::GetNextToken(in, line);
    if(lexItem == NEQ || lexItem == SEQ)
    {
        status = RelExpr(in, line);
        if(!status)
            return false;
    }
    else
        Parser::PushBackToken(lexItem);

    return true;
}

// RelExpr ::= AddExpr [ ( -lt | -gt | < | > ) AddExpr ]
bool RelExpr(istream &in, int &line) {
    bool status = AddExpr(in, line);
    if(!status)
        return false;

    LexItem lexItem = Parser::GetNextToken(in, line);
    if(lexItem == NLTHAN || lexItem == NGTHAN || lexItem == SLTHAN || lexItem == SGTHAN)
    {
        status = AddExpr(in, line);
        if(!status)
            return false;
    }
    else
        Parser::PushBackToken(lexItem);

    return true;
}

// AddExpr :: MultExpr { ( + | - | .) MultExpr }
bool AddExpr(istream &in, int &line) {
    bool status = MultExpr(in, line);
    if(!status)
        return false;

    LexItem lexItem = Parser::GetNextToken(in, line);
    if(lexItem == PLUS || lexItem == MINUS || lexItem == CAT)
    {
        status = MultExpr(in, line);
        if(!status)
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
    }
    else
        Parser::PushBackToken(lexItem);

    return true;
}

// MultExpr ::= ExponExpr { ( * | / | **) ExponExpr }
bool MultExpr(istream &in, int &line) {
    bool status = ExponExpr(in, line);
    if(!status)
        return false;

    LexItem lexItem = Parser::GetNextToken(in, line);
    if(lexItem == MULT || lexItem == DIV || lexItem == SREPEAT)
    {
        status = ExponExpr(in, line);
        if(!status)
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
    }
    else
        Parser::PushBackToken(lexItem);

    return true;
}

// ExponExpr ::= UnaryExpr { ^ UnaryExpr }
bool ExponExpr(istream &in, int &line) {
    bool status = UnaryExpr(in, line);
    if(!status)
        return false;

    LexItem lexItem = Parser::GetNextToken(in, line);
    if(lexItem == EXPONENT)
    {
        status = UnaryExpr(in, line);
        if(!status)
            return false;
    }
    else
        Parser::PushBackToken(lexItem);

    return true;
}

// UnaryExpr ::= [( - | + )] PrimaryExpr
bool UnaryExpr(istream &in, int &line) {
    LexItem lexItem = Parser::GetNextToken(in, line);

    int sign = 1;
    if(lexItem == MINUS || lexItem == PLUS)
    {
        if(lexItem == MINUS)
            sign = -1;
    }
    else
        Parser::PushBackToken(lexItem);

    bool status = PrimaryExpr(in, line, sign);
    if(!status)
        return false;

    return true;
}

// PrimaryExpr ::= IDENT | SIDENT | NIDENT | ICONST | RCONST | SCONST | (Expr)
bool PrimaryExpr(istream& in, int& line, int sign) {
    LexItem lexItem = Parser::GetNextToken(in, line);

    if (lexItem == IDENT || lexItem == SIDENT || lexItem == NIDENT ||
        lexItem == ICONST || lexItem == RCONST || lexItem == SCONST ||
        lexItem == LPAREN)
    {
        if (lexItem == LPAREN)
        {
            bool status = Expr(in, line);
            if (!status)
            {
                ParseError(line, "Missing expression after Left Parenthesis");
                return false;
            }

            lexItem = Parser::GetNextToken(in, line);
            if (lexItem != RPAREN)
            {
                ParseError(line, "Missing right Parenthesis after expression");
                return false;
            }
        }
//        if (sign == -1 && (lexItem == ICONST || lexItem == RCONST))
//            cout << "Unary minus encountered. This operation is not yet implemented." << endl;

        if(lexItem == IDENT || lexItem == SIDENT || lexItem == NIDENT)
        {
            if(defVar.find(lexItem.getLexeme()) == defVar.end())
            {
                ParseError(line, "Using Undefined Variable");
                return false;
            }
        }
        return true;
    }
    else
        return false;
}
