#include "parserSP26.h"
#include <map>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

map<string, bool> defVar;
map<string, bool> defConst;

static int error_count = 0;
static string progName = "";
static int lastStmtStartLine = 1;

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

static bool IsExprStart(Token t) {
	return t == IDENT || t == ICONST || t == RCONST || t == SCONST ||
		   t == BCONST || t == CCONST || t == LPAREN ||
		   t == PLUS || t == MINUS || t == NOT;
}

static bool IsStmtStart(Token t) {
	return t == IDENT || t == READLN || t == WRITELN || t == WRITE || t == IF || t == BEGIN;
}

static void PrintNames(const map<string, bool>& table) {
	bool first = true;
	for (map<string, bool>::const_iterator it = table.begin(); it != table.end(); ++it) {
		if (!first) cout << ", ";
		cout << it->first;
		first = false;
	}
	cout << endl;
}

int ErrCount() {
	return error_count;
}

void ParseError(int line, string msg) {
	++error_count;
	cout << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line) {
	error_count = 0;
	defVar.clear();
	defConst.clear();
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

	if (!Block(in, line)) {
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != DOT) {
		ParseError(line, "Missing period at end of program.");
		return false;
	}

	cout << "Program Name: " << progName << endl;
	cout << "Declared Variables:" << endl;
	PrintNames(defVar);
	cout << endl;
	cout << "Defined Constants:" << endl;
	PrintNames(defConst);
	cout << endl;
	cout << "DONE" << endl;

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

	if (!Expr(in, line)) {
		ParseError(line, "Incorrect constant definition syntax.");
		return false;
	}

	defConst[name] = true;
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

		for (int i = 0; i < (int)ids.size(); i++) {
			if (ids[i] == name) {
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

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ASSOP) {
		if (!Expr(in, line)) return false;
		for (int i = 0; i < (int)ids.size(); i++) defVar[ids[i]] = true;
	}
	else {
		Parser::PushBackToken(tok);
		for (int i = 0; i < (int)ids.size(); i++) defVar[ids[i]] = false;
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
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != WRITELN) {
		Parser::PushBackToken(tok);
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != LPAREN) {
		ParseError(line, "Missing left parenthesis after WRITELN.");
		return false;
	}

	if (!ExprList(in, line)) return false;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != RPAREN) {
		ParseError(line, "Missing right parenthesis after WRITELN.");
		return false;
	}

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
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != READLN) {
		Parser::PushBackToken(tok);
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != LPAREN) {
		ParseError(line, "Missing left parenthesis after READLN.");
		return false;
	}

	if (!VarList(in, line)) return false;

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != RPAREN) {
		ParseError(line, "Missing right parenthesis after READLN.");
		return false;
	}

	return true;
}

bool IfStmt(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != IF) {
		Parser::PushBackToken(tok);
		return false;
	}

	if (!Expr(in, line)) {
		ParseError(line, "Missing if statement Logic Expression.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() != THEN) {
		if (IsStmtStart(tok.GetToken()) || tok.GetToken() == ELSE) {
			ParseError(line, "If Statement Syntax Error.");
		}
		else {
			ParseError(line, "Missing then keyword in if statement");
		}
		return false;
	}

	LexItem next = Parser::GetNextToken(in, line);
	Parser::PushBackToken(next);

	if (next.GetToken() == IDENT || next.GetToken() == READLN ||
		next.GetToken() == WRITELN || next.GetToken() == WRITE) {
		if (!SimpleStmt(in, line)) {
			ParseError(line, "Missing Statement for If-Then-Part.");
			return false;
		}
	}
	else if (next.GetToken() == IF || next.GetToken() == BEGIN) {
		if (!StructuredStmt(in, line)) {
			ParseError(line, "Missing Statement for If-Then-Part.");
			return false;
		}
	}
	else {
		ParseError(line, "Missing Statement for If-Then-Part.");
		return false;
	}

	tok = Parser::GetNextToken(in, line);
	if (tok.GetToken() == ELSE) {
		LexItem next2 = Parser::GetNextToken(in, line);
		Parser::PushBackToken(next2);

		if (next2.GetToken() == IDENT || next2.GetToken() == READLN ||
			next2.GetToken() == WRITELN || next2.GetToken() == WRITE) {
			if (!SimpleStmt(in, line)) {
				ParseError(line, "Missing Statement for Else-Part.");
				return false;
			}
		}
		else if (next2.GetToken() == IF || next2.GetToken() == BEGIN) {
			if (!StructuredStmt(in, line)) {
				ParseError(line, "Missing Statement for Else-Part.");
				return false;
			}
		}
		else {
			ParseError(line, "Missing Statement for Else-Part.");
			return false;
		}
	}
	else {
		Parser::PushBackToken(tok);
	}

	return true;
}

bool AssignStmt(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() != IDENT) {
		Parser::PushBackToken(tok);
		return false;
	}

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

	if (!Expr(in, line)) {
		ParseError(line, "Missing Expression in Assignment Statement.");
		return false;
	}

	defVar[name] = true;
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
	if (!Expr(in, line)) return false;

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == ERR) return false;

		if (tok.GetToken() != COMMA) {
			Parser::PushBackToken(tok);
			break;
		}

		if (!Expr(in, line)) {
			ParseError(line, "Missing expression after comma");
			return false;
		}
	}

	return true;
}

bool Expr(istream& in, int& line) {
	if (!SimpleExpr(in, line)) return false;

	LexItem tok = Parser::GetNextToken(in, line);

	if (tok.GetToken() == ERR) return false;

	if (IsRelOp(tok.GetToken())) {
		if (!SimpleExpr(in, line)) return false;

		LexItem next = Parser::GetNextToken(in, line);
		if (IsRelOp(next.GetToken())) {
			ParseError(line, "Illegal Relational Expression.");
			return false;
		}
		Parser::PushBackToken(next);
	}
	else {
		Parser::PushBackToken(tok);
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

bool VarList(istream& in, int& line) {
	if (!Variable(in, line)) return false;

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == ERR) return false;

		if (tok.GetToken() != COMMA) {
			Parser::PushBackToken(tok);
			break;
		}

		if (!Variable(in, line)) {
			ParseError(line, "Missing variable after comma");
			return false;
		}
	}

	return true;
}

bool SimpleExpr(istream& in, int& line) {
	if (!Term(in, line)) return false;

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == ERR) return false;

		if (tok.GetToken() != PLUS && tok.GetToken() != MINUS && tok.GetToken() != OR) {
			Parser::PushBackToken(tok);
			break;
		}

		LexItem next = Parser::GetNextToken(in, line);
		Parser::PushBackToken(next);

		if (!IsExprStart(next.GetToken())) {
			ParseError(line, "Missing operand after operator.");
			return false;
		}

		if (!Term(in, line)) {
			ParseError(line, "Missing operand after operator.");
			return false;
		}
	}

	return true;
}

bool Term(istream& in, int& line) {
	if (!SFactor(in, line)) return false;

	while (true) {
		LexItem tok = Parser::GetNextToken(in, line);

		if (tok.GetToken() == ERR) return false;

		if (tok.GetToken() != MULT &&
			tok.GetToken() != DIV &&
			tok.GetToken() != IDIV &&
			tok.GetToken() != MOD &&
			tok.GetToken() != AND) {
			Parser::PushBackToken(tok);
			break;
		}

		LexItem next = Parser::GetNextToken(in, line);
		Parser::PushBackToken(next);

		if (!IsExprStart(next.GetToken())) {
			ParseError(line, "Missing operand after operator.");
			return false;
		}

		if (!SFactor(in, line)) {
			ParseError(line, "Missing operand after operator.");
			return false;
		}
	}

	return true;
}

bool SFactor(istream& in, int& line) {
	LexItem tok = Parser::GetNextToken(in, line);
	bool unary = false;
	int sign = 1;

	if (tok.GetToken() == MINUS) {
		unary = true;
		sign = -1;
	}
	else if (tok.GetToken() == PLUS) {
		unary = true;
		sign = 1;
	}
	else if (tok.GetToken() == NOT) {
		unary = true;
		sign = 0;
	}
	else {
		Parser::PushBackToken(tok);
	}

	if (!Factor(in, line, sign)) {
		if (unary) ParseError(line, "Missing factor after unary operator");
		return false;
	}

	return true;
}

bool Factor(istream& in, int& line, int sign) {
	LexItem tok = Parser::GetNextToken(in, line);

	switch (tok.GetToken()) {
		case IDENT:
			if (defVar.find(tok.GetLexeme()) == defVar.end() &&
				defConst.find(tok.GetLexeme()) == defConst.end()) {
				ParseError(line, "Undeclared Variable: " + tok.GetLexeme());
				return false;
			}
			return true;

		case ICONST:
		case RCONST:
		case SCONST:
		case BCONST:
		case CCONST:
			return true;

		case LPAREN:
			if (!Expr(in, line)) {
				ParseError(line, "Missing expression after Left Parenthesis.");
				return false;
			}
			tok = Parser::GetNextToken(in, line);
			if (tok.GetToken() != RPAREN) {
				ParseError(line, "Missing ending parenthesis");
				return false;
			}
			return true;

		case ERR:
			return false;

		default:
			Parser::PushBackToken(tok);
			return false;
	}
}