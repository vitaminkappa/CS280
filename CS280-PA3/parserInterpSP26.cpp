#include "parserInterpSP26.h"
#include <map>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

map<string, bool> defVar;
map<string, bool> defConst;
map<string, Value> SymTable;
map<string, Token> SymType;

static int error_count = 0;
static string progName = "";
static int lastStmtStartLine = 1;
bool ExecFlag = true; // Controls whether statements actually execute

void ParseError(int line, string msg);

namespace Parser {
	bool pushed_back = false;
	LexItem pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if (pushed_back) {
			pushed_back = false;
			return pushed_token;
		}

		LexItem tok = getNextToken(in, line);

		if (tok.GetToken() == ERR) {
			ParseError(line, "Unrecognized Input Pattern.");
			cout << "(" << tok.GetLexeme() << ")" << endl;
		}

		return tok;
	}

	static void PushBackToken(LexItem& t) {
		if (pushed_back) abort();
		pushed_back = true;
		pushed_token = t;
	}
}

static bool IsTypeToken(Token t) {
	return t == INTEGER || t == REAL || t == BOOLEAN || t == CHAR || t == STRING;
}

static bool IsRelOp(Token t) {
	return t == EQ || t == LTHAN || t == GTHAN;
}

/* static bool IsExprStart(Token t) {
	return t == IDENT || t == ICONST || t == RCONST || t == SCONST ||
		   t == BCONST || t == CCONST || t == LPAREN ||
		   t == PLUS || t == MINUS || t == NOT;
} */

static bool IsStmtStart(Token t) {
	return t == IDENT || t == READLN || t == WRITELN || t == WRITE || t == IF || t == BEGIN;
}

/* static void PrintNames(const map<string, bool>& table) {
	bool first = true;
	for (map<string, bool>::const_iterator it = table.begin(); it != table.end(); ++it) {
		if (!first) cout << ", ";
		cout << it->first;
		first = false;
	}
	cout << endl;
}
*/
int ErrCount() {
	return error_count;
}

bool TypeCompatible(Token formal, Value& actual) {
    if (formal == INTEGER && actual.IsInt()) return true;
    if (formal == REAL && (actual.IsReal() || actual.IsInt())) return true;
    if (formal == BOOLEAN && actual.IsBool()) return true;
    if (formal == CHAR && actual.IsChar()) return true;
    if (formal == STRING && actual.IsString()) return true;
    return false;
}

// Helper to handle type coercion (e.g., Int to Real)
Value CoerceToType(Token formal, Value actual) {
    if (formal == REAL && actual.IsInt()) {
        return Value((float)actual.GetInt());
    }
    return actual;
}

void ParseError(int line, string msg) {
	++error_count;
	cout << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line) {
	error_count = 0;
	defVar.clear();
	defConst.clear();
    SymTable.clear(); // Reset for new run
    SymType.clear();
    ExecFlag = true;
	progName = "";
	Parser::pushed_back = false;
	lastStmtStartLine = 1;

	LexItem tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != PROGRAM) {
		ParseError(line, "Missing PROGRAM.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != IDENT) {
		ParseError(line, "Missing Program Name.");
		return false;
	}
	progName = tok.GetLexeme();

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != SEMICOL) {
		ParseError(line, "Missing semicolon after program name.");
		return false;
	}

	if (!Block(in, line)) return false;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != DOT) {
		ParseError(line, "Missing period at end of program.");
		return false;
	}

    // Done output remains the same
    
    std::cout << endl << endl << "DONE" << endl;
	return true;
}
bool Block(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);
	Parser::PushBackToken(tok);

	if (tok.GetToken() == CONST || tok.GetToken() == VAR) {
		if (!DeclPart(in, line)) {
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
	}

	if (!CompStmt(in, line)) {
		ParseError(line, "Incorrect Program Body.");
		return false;
	}

	return true;
}

bool DeclPart(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);
	Parser::PushBackToken(tok);

	if (tok.GetToken() == CONST) {
		if (!ConstPart(in, line)) {
			ParseError(line, "Incorrect Constant Definition Part.");
			return false;
		}
	}

	tok = Parser::GetNextToken(in, line);
	Parser::PushBackToken(tok);

	if (tok.GetToken() == VAR) {
		if (!VarPart(in, line)) {
			ParseError(line, "Incorrect Declaration Part.");
			return false;
		}
	}

	return true;
}

bool ConstPart(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != CONST) {
		Parser::PushBackToken(tok);
		return false;
	}

	if (!ConstDef(in, line)) {
		ParseError(line, "Syntactic error in Constants Definitions Part.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != SEMICOL) {
		ParseError(line, "Missing semicolon.");
		return false;
	}

	while (true) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() != IDENT) {
			Parser::PushBackToken(tok);
			break;
		}

		Parser::PushBackToken(tok);

		if (!ConstDef(in, line)) {
			ParseError(line, "Syntactic error in Constants Definitions Part.");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() != SEMICOL) {
			ParseError(line, "Missing semicolon.");
			return false;
		}
	}

	return true;
}

bool VarPart(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != VAR) {
		Parser::PushBackToken(tok);
		return false;
	}

	if (!DeclStmt(in, line)) {
		ParseError(line, "Syntactic error in Declaration Block.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != SEMICOL) {
		ParseError(line, "Missing semicolon.");
		return false;
	}

	while (true) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() != IDENT) {
			Parser::PushBackToken(tok);
			break;
		}

		Parser::PushBackToken(tok);

		if (!DeclStmt(in, line)) {
			ParseError(line, "Syntactic error in Declaration Block.");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() != SEMICOL) {
			ParseError(line, "Missing semicolon.");
			return false;
		}
	}

	return true;
}

bool ConstDef(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != IDENT) {
		ParseError(line, "Incorrect constant definition syntax.");
		return false;
	}
	string name = tok.GetLexeme();

    // Original redefinition checks
	if (defConst.find(name) != defConst.end()) {
		ParseError(line, "Constant Redefinition: " + name);
		return false;
	}
	if (defVar.find(name) != defVar.end()) {
		ParseError(line, "Redefinition of identifier: " + name);
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != EQ) {
		ParseError(line, "Incorrect constant definition syntax.");
		return false;
	}

    Value val;
	if (!Expr(in, line, val)) {
		ParseError(line, "Incorrect constant definition syntax.");
		return false;
	}

   defConst[name] = true;
    SymTable[name] = val;
    if (val.IsInt()) SymType[name] = INTEGER;
    else if (val.IsReal()) SymType[name] = REAL;
    else if (val.IsBool()) SymType[name] = BOOLEAN;
    else if (val.IsChar()) SymType[name] = CHAR;
    else if (val.IsString()) SymType[name] = STRING;
	return true;
}

bool DeclStmt(istream& in, int& line) {
	vector<string> ids;
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != IDENT) {
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}

	string firstName = tok.GetLexeme();

    // Preserve the original double ParseError messages
	if (defConst.find(firstName) != defConst.end()) {
		ParseError(line, "Illegal use of a constant name as a variable: " + firstName);
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}
	if (defVar.find(firstName) != defVar.end()) {
		ParseError(line, "Variable Redefinition: " + firstName);
		ParseError(line, "Incorrect identifiers list in Declaration Statement.");
		return false;
	}

	ids.push_back(firstName);

	while (true) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == COLON) break;
		if (tok.GetToken() != COMMA) {
			ParseError(line, "Missing comma in declaration statement.");
			ParseError(line, "Incorrect identifiers list in Declaration Statement.");
			return false;
		}

		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() != IDENT) {
			ParseError(line, "Incorrect identifiers list in Declaration Statement.");
			return false;
		}

		string name = tok.GetLexeme();
		for (auto const& id : ids) {
			if (id == name) {
				ParseError(line, "Variable Redefinition: " + name);
				ParseError(line, "Incorrect identifiers list in Declaration Statement.");
				return false;
			}
		}
		if (defConst.find(name) != defConst.end()) {
			ParseError(line, "Illegal use of a constant name as a variable: " + name);
			ParseError(line, "Incorrect identifiers list in Declaration Statement.");
			return false;
		}
		if (defVar.find(name) != defVar.end()) {
			ParseError(line, "Variable Redefinition: " + name);
			ParseError(line, "Incorrect identifiers list in Declaration Statement.");
			return false;
		}
		ids.push_back(name);
	}

	tok = Parser::GetNextToken(in, line);
	if (!IsTypeToken(tok.GetToken())) {
		ParseError(line, "Incorrect Declaration Type: " + tok.GetLexeme());
		return false;
	}
    Token typeTok = tok.GetToken();

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ASSOP) {
        Value val;
		if (!Expr(in, line, val)) return false;
		if (ExecFlag) {
            if (!TypeCompatible(typeTok, val)) {
                ParseError(line, "Type Mismatch in Assignment");
                return false;
            }
            for (auto const& id : ids) {
                defVar[id] = true;
                SymType[id] = typeTok;
                SymTable[id] = CoerceToType(typeTok, val);
            }
        }
	}
	else {
		Parser::PushBackToken(tok);
		for (auto const& id : ids) {
            defVar[id] = true;
            SymType[id] = typeTok;
        }
	}
	return true;
}

bool Stmt(istream& in, int& line) {
	lastStmtStartLine = line;

	LexItem tok = Parser::GetNextToken(in, line);
	Parser::PushBackToken(tok);

	if (tok.GetToken() == IDENT || tok.GetToken() == READLN ||
		tok.GetToken() == WRITELN || tok.GetToken() == WRITE) {
		if (!SimpleStmt(in, line)) {
			ParseError(line, "Syntactic error in the statement.");
			return false;
		}
		return true;
	}

	if (tok.GetToken() == IF || tok.GetToken() == BEGIN) {
		if (!StructuredStmt(in, line)) {
			ParseError(line, "Syntactic error in the statement.");
			return false;
		}
		return true;
	}

	if (tok.GetToken() == ELSE) {
		ParseError(line, "Illegal Else-clause.");
		return false;
	}

	return false;
}

bool StructuredStmt(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);
	Parser::PushBackToken(tok);

	if (tok.GetToken() == IF) {
		if (!IfStmt(in, line)) {
			ParseError(line, "Incorrect Structured Statement.");
			return false;
		}
		return true;
	}

	if (tok.GetToken() == BEGIN) {
		if (!CompStmt(in, line)) {
			ParseError(line, "Incorrect Structured Statement.");
			return false;
		}
		return true;
	}

	return false;
}

bool CompStmt(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != BEGIN) {
		Parser::PushBackToken(tok);
		return false;
	}

	if (!Stmt(in, line)) {
		ParseError(line, "Incorrect Program Body.");
		return false;
	}

	while (true) {
		tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == END) return true;

		if (tok.GetToken() == DONE) {
			ParseError(line, "Missing end of compound statement.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}

		if (tok.GetToken() != SEMICOL) {
			ParseError(lastStmtStartLine, "Missing semicolon.");
			ParseError(lastStmtStartLine, "Incorrect Program Body.");
			return false;
		}

		LexItem next = Parser::GetNextToken(in, line);
		Parser::PushBackToken(next);

		if (next.GetToken() == END) {
			tok = Parser::GetNextToken(in, line);
			if (tok.GetToken() == END) return true;
			ParseError(line, "Incorrect Program Body.");
			return false;
		}

		if (next.GetToken() == DONE) {
			ParseError(line, "Missing end of compound statement.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}

		if (next.GetToken() == ELSE) {
			ParseError(line, "Illegal Else-clause.");
			ParseError(line, "Incorrect Program Body.");
			return false;
		}

		if (!Stmt(in, line)) {
			ParseError(line, "Incorrect Program Body.");
			return false;
		}
	}
}

bool SimpleStmt(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);
	Parser::PushBackToken(tok);

	if (tok.GetToken() == IDENT) {
		if (!AssignStmt(in, line)) {
			ParseError(line, "Incorrect Simple Statement.");
			return false;
		}
		return true;
	}

	if (tok.GetToken() == READLN) {
		if (!ReadLnStmt(in, line)) {
			ParseError(line, "Incorrect Simple Statement.");
			return false;
		}
		return true;
	}

	if (tok.GetToken() == WRITELN) {
		if (!WriteLnStmt(in, line)) {
			ParseError(line, "Incorrect Simple Statement.");
			return false;
		}
		return true;
	}

	if (tok.GetToken() == WRITE) {
		if (!WriteStmt(in, line)) {
			ParseError(line, "Incorrect Simple Statement.");
			return false;
		}
		return true;
	}

	return false;
}

bool WriteLnStmt(istream& in, int& line) {
	Parser::GetNextToken(in, line); // Skip WRITELN
	LexItem tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != LPAREN) {
		ParseError(line, "Missing left parenthesis after WRITELN.");
		return false;
	}

    // Logic for ExprList with printing
    // (Simplifying ExprList call here—ensure your ExprList uses ExecFlag to print)
	if (!ExprList(in, line)) return false;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != RPAREN) {
		ParseError(line, "Missing right parenthesis after WRITELN.");
		return false;
	}

    if (ExecFlag) cout << endl;
	return true;
}

bool WriteStmt(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != WRITE) {
		Parser::PushBackToken(tok);
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != LPAREN) {
		ParseError(line, "Missing left parenthesis after WRITE.");
		return false;
	}

	if (!ExprList(in, line)) return false;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != RPAREN) {
		ParseError(line, "Missing right parenthesis after WRITE.");
		return false;
	}

	return true;
}

bool ReadLnStmt(istream& in, int& line) {
	Parser::GetNextToken(in, line); // Skip READLN
	LexItem tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != LPAREN) {
		ParseError(line, "Missing left parenthesis after READLN.");
		return false;
	}

    vector<string> varNames;
    // Assuming VarList is updated to populate varNames
	if (!VarList(in, line, varNames)) return false;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != RPAREN) {
		ParseError(line, "Missing right parenthesis after READLN.");
		return false;
	}

    if (ExecFlag) {
        for (string name : varNames) {
    string input;
    if (SymType[name] == STRING) {
        getline(cin >> ws, input);
        SymTable[name] = Value(input);
    } else {
        cin >> input;
        if (SymType[name] == INTEGER) SymTable[name] = Value(stoi(input));
        else if (SymType[name] == REAL) SymTable[name] = Value(stof(input));
        else if (SymType[name] == BOOLEAN) SymTable[name] = Value(input == "true" || input == "True" || input == "TRUE");
        else if (SymType[name] == CHAR) SymTable[name] = Value(input[0]);
    }
}
    }
	return true;
}
bool IfStmt(istream& in, int& line) {
	Parser::GetNextToken(in, line); // Skip IF
    Value cond;
	if (!Expr(in, line, cond)) {
		ParseError(line, "Missing if statement Logic Expression.");
		return false;
	}

    if (!cond.IsBool()) {
        ParseError(line, "Run-Time Error: If condition is not boolean");
        return false;
    }

	LexItem tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != THEN) {
        // Original specific error checks
		if (IsStmtStart(tok.GetToken()) || tok.GetToken() == ELSE) {
			ParseError(line, "If Statement Syntax Error.");
		} else {
			ParseError(line, "Missing then keyword in if statement");
		}
		return false;
	}

    bool prevExec = ExecFlag;
    ExecFlag = prevExec && cond.GetBool();
    if (!Stmt(in, line)) return false;
        ExecFlag = prevExec;
        tok = Parser::GetNextToken(in, line);
    
    if (tok.GetToken() == ELSE) {
        ExecFlag = prevExec && !cond.GetBool();
        if (!Stmt(in, line)) return false;
        ExecFlag = prevExec;
	}
	else {
		Parser::PushBackToken(tok);
	}
	return true;
}

bool AssignStmt(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);
	string name = tok.GetLexeme();

	if (defConst.find(name) != defConst.end()) {
		ParseError(line, "Illegal use of a Constant as a Variable");
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement.");
		return false;
	}
	if (defVar.find(name) == defVar.end()) {
		ParseError(line, "Undeclared Variable: " + name);
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != ASSOP) {
		ParseError(line, "Missing Assignment Operator.");
		return false;
	}

    Value val;
	if (!Expr(in, line, val)) {
		ParseError(line, "Missing Expression in Assignment Statement.");
		return false;
	}

	if (ExecFlag) {
        if (!TypeCompatible(SymType[name], val)) {
            ParseError(line, "Type Mismatch in Assignment");
            return false;
        }
        SymTable[name] = CoerceToType(SymType[name], val);
    }
	return true;
}

bool Variable(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != IDENT) {
		Parser::PushBackToken(tok);
		return false;
	}

	if (defConst.find(tok.GetLexeme()) != defConst.end()) {
		ParseError(line, "Illegal use of a Constant as a Variable");
		return false;
	}

	if (defVar.find(tok.GetLexeme()) == defVar.end()) {
		ParseError(line, "Undeclared Variable: " + tok.GetLexeme());
		return false;
	}

	return true;
}

bool ExprList(istream& in, int& line) {
    Value val;
    // Every ExprList starts with at least one expression
    if (!Expr(in, line, val)) {
        ParseError(line, "Missing Expression");
        return false;
    }
    
    // If we are actually executing, print the first value
    if (ExecFlag) cout << val;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok.GetToken() == COMMA) {
        if (!Expr(in, line, val)) {
            ParseError(line, "Missing Expression after comma");
            return false;
        }
        if (ExecFlag) cout << val;
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

bool Expr(istream& in, int& line, Value & retVal) {
    Value v1, v2;
    if (!SimpleExpr(in, line, v1)) return false;

    LexItem tok = Parser::GetNextToken(in, line);
    if (IsRelOp(tok.GetToken())) {
    Token op = tok.GetToken();
    if (!SimpleExpr(in, line, v2)) return false;

    if (op == EQ) retVal = (v1 == v2);
    else if (op == LTHAN) retVal = (v1 < v2);
    else if (op == GTHAN) retVal = (v1 > v2);

    LexItem next = Parser::GetNextToken(in, line);
    if (IsRelOp(next.GetToken())) {
        ParseError(line, "Illegal Relational Expression.");
        return false;
    }
    Parser::PushBackToken(next);
} else {
    Parser::PushBackToken(tok);
    retVal = v1;
}
    return true;
}

bool IdentList(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != IDENT) {
		ParseError(line, "Missing identifier.");
		return false;
	}

	while (true) {
		tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() != COMMA) {
			Parser::PushBackToken(tok);
			break;
		}

		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() != IDENT) {
			ParseError(line, "Missing identifier after comma.");
			return false;
		}
	}

	return true;
}

bool VarList(istream& in, int& line, vector<string> & IdList) {
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok.GetToken() != IDENT) {
        ParseError(line, "Missing Variable");
        return false;
    }

    string name = tok.GetLexeme();
    if (defVar.find(name) == defVar.end()) {
        ParseError(line, "Undeclared Variable: " + name);
        return false;
    }
    IdList.push_back(name);

    tok = Parser::GetNextToken(in, line);
    while (tok.GetToken() == COMMA) {
        tok = Parser::GetNextToken(in, line);
        if (tok.GetToken() != IDENT) {
            ParseError(line, "Missing Variable after comma");
            return false;
        }
        name = tok.GetLexeme();
        if (defVar.find(name) == defVar.end()) {
            ParseError(line, "Undeclared Variable: " + name);
            return false;
        }
        IdList.push_back(name);
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

bool SimpleExpr(istream& in, int& line, Value & retVal) {
    Value v1, v2;
    if (!Term(in, line, v1)) return false;

    LexItem tok = Parser::GetNextToken(in, line);
    while (tok.GetToken() == PLUS || tok.GetToken() == MINUS || tok.GetToken() == OR) {
        Token op = tok.GetToken();
        if (!Term(in, line, v2)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }

        if (ExecFlag) {
            if (op == PLUS) v1 = v1 + v2;
            else if (op == MINUS) v1 = v1 - v2;
            else if (op == OR) v1 = v1 || v2;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    retVal = v1;
    return true;
}

bool Term(istream& in, int& line, Value & retVal) {
    Value v1, v2;
    if (!SFactor(in, line, v1)) return false;

    LexItem tok = Parser::GetNextToken(in, line);
       while (tok.GetToken() == MULT || tok.GetToken() == DIV || tok.GetToken() == IDIV || tok.GetToken() == MOD || tok.GetToken() == AND) {

        Token op = tok.GetToken();
        if (!SFactor(in, line, v2)) {
            ParseError(line, "Missing operand after operator.");
            return false;
        }

        if (ExecFlag) {
            if (op == MULT) v1 = v1 * v2;
            else if (op == DIV) {
                // Basic division check
                if ((v2.IsInt() && v2.GetInt() == 0) || (v2.IsReal() && v2.GetReal() == 0)) {
                    ParseError(line, "Run-Time Error: Division by Zero");
                    return false;
                }
                v1 = v1 / v2;
            }
            else if (op == AND) v1 = v1 && v2;
            else if (op == MOD) v1 = v1 % v2;
            else if (op == IDIV) {
                if ((v2.IsInt() && v2.GetInt() == 0) || (v2.IsReal() && v2.GetReal() == 0)) {
                    ParseError(line, "Run-Time Error: Division by Zero");
                    return false;
                }
                v1 = v1.idiv(v2);
            }
         }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    retVal = v1;
    return true;
}

bool SFactor(istream& in, int& line, Value& retVal) {
    LexItem tok = Parser::GetNextToken(in, line);
    Token op = tok.GetToken();
    bool hasUnary = true;

    if (op != PLUS && op != MINUS && op != NOT) {
        Parser::PushBackToken(tok);
        hasUnary = false;
    }

    if (!Factor(in, line, 1, retVal)) {
        if (hasUnary) ParseError(line, "Missing factor after unary operator");
        return false;
    }

    if (!hasUnary || !ExecFlag) return true;

    if (op == PLUS) {
        if (!retVal.IsInt() && !retVal.IsReal()) {
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;
        }
    }
    else if (op == MINUS) {
        if (retVal.IsInt() || retVal.IsReal()) {
            retVal = -retVal;
        } else if (retVal.IsString()) {
            retVal = retVal.Trim();
        } else {
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;
        }
        if (retVal.IsErr()) {
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;
        }
    }
    else if (op == NOT) {
        if (!retVal.IsBool()) {
            ParseError(line, "Illegal Operand Type for Sign/NOT Operator.");
            return false;
        }
        retVal = !retVal;
    }
    return true;
}

bool Factor(istream& in, int& line, int sign, Value & retVal) {
    LexItem tok = Parser::GetNextToken(in, line);

    if (tok.GetToken() == IDENT) {
        string name = tok.GetLexeme();
        // Check if the variable/constant is defined
        if (defVar.find(name) == defVar.end() && defConst.find(name) == defConst.end()) {
            ParseError(line, "Undeclared Variable: " + name);
            return false;
        }

        // INTERPRETER LOGIC: Fetch the value from the Symbol Table
        if (ExecFlag) {
    if (SymTable.find(name) == SymTable.end()) {
        ParseError(line, "Using Uninitialized Variable");
        return false;
    }
    retVal = SymTable[name];
    if (sign == -1) {
        if (retVal.IsInt()) retVal = Value(retVal.GetInt() * -1);
        else if (retVal.IsReal()) retVal = Value(retVal.GetReal() * -1.0f);
        else {
            ParseError(line, "Run-Time Error: Invalid sign for non-numeric type.");
            return false;
        }
    }
} else {
    retVal = Value(0);
}
return true;
    } 
    else if (tok.GetToken() == ICONST) {
        retVal = Value(stoi(tok.GetLexeme()) * sign);
        return true;
    } 
    else if (tok.GetToken() == RCONST) {
        retVal = Value(stof(tok.GetLexeme()) * (float)sign);
        return true;
    } 
    else if (tok.GetToken() == SCONST) {
        retVal = Value(tok.GetLexeme());
        return true;
    } 
    else if (tok.GetToken() == BCONST) {
        retVal = Value(tok.GetLexeme() == "true");
        return true;
    }
    else if (tok.GetToken() == CCONST) {
        retVal = Value(tok.GetLexeme()[0]);
        return true;
    }
    else if (tok.GetToken() == LPAREN) {
        // Handle ( Expression )
        if (!Expr(in, line, retVal)) {
            ParseError(line, "Missing expression after (");
            return false;
        }
        if (Parser::GetNextToken(in, line).GetToken() != RPAREN) {
            ParseError(line, "Missing ) after expression");
            return false;
        }
        // Apply sign to the parenthesized result
        if (sign == -1 && ExecFlag) {
            if (retVal.IsInt()) retVal = Value(retVal.GetInt() * -1);
            else if (retVal.IsReal()) retVal = Value(retVal.GetReal() * -1.0f);
        }
        return true;
    }

    ParseError(line, "Unrecognized input in Factor");
    return false;
}