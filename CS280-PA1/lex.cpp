#include "lex.h"
#include <map>
#include <string>
#include <cctype>

using namespace std;

int tokenCount = 0, idCount = 0, keywordCount = 0, numberCount = 0, booleanCount = 0, stringCount = 0;

static string lowerCopy(const string& s) {
    string out = s;
    for (size_t i = 0; i < out.size(); i++)
        out[i] = static_cast<char>(tolower(static_cast<unsigned char>(out[i])));
    return out;
}

static string tokenName(Token t) {
    switch (t) {
    case IF: return "IF";
    case ELSE: return "ELSE";
    case WRITELN: return "WRITELN";
    case WRITE: return "WRITE";
    case READLN: return "READLN";
    case INTEGER: return "INTEGER";
    case REAL: return "REAL";
    case BOOLEAN: return "BOOLEAN";
    case CHAR: return "CHAR";
    case STRING: return "STRING";
    case BEGIN: return "BEGIN";
    case END: return "END";
    case VAR: return "VAR";
    case CONST: return "CONST";
    case THEN: return "THEN";
    case PROGRAM: return "PROGRAM";
    case IDENT: return "IDENT";
    case TRUE: return "TRUE";
    case FALSE: return "FALSE";
    case ICONST: return "ICONST";
    case RCONST: return "RCONST";
    case SCONST: return "SCONST";
    case BCONST: return "BCONST";
    case CCONST: return "CCONST";
    case PLUS: return "PLUS";
    case MINUS: return "MINUS";
    case MULT: return "MULT";
    case DIV: return "DIV";
    case IDIV: return "IDIV";
    case MOD: return "MOD";
    case ASSOP: return "ASSOP";
    case EQ: return "EQ";
    case GTHAN: return "GTHAN";
    case LTHAN: return "LTHAN";
    case AND: return "AND";
    case OR: return "OR";
    case NOT: return "NOT";
    case COMMA: return "COMMA";
    case SEMICOL: return "SEMICOL";
    case LPAREN: return "LPAREN";
    case RPAREN: return "RPAREN";
    case LBRACE: return "LBRACE";
    case RBRACE: return "RBRACE";
    case DOT: return "DOT";
    case COLON: return "COLON";
    case ERR: return "ERR";
    case DONE: return "DONE";
    default: return "";
    }
}

ostream& operator<<(ostream& out, const LexItem& tok) {
    Token t = tok.GetToken();

    if (t == ERR) {
        out << "ERR: Error in line (" << tok.GetLinenum() << ") " << tok.GetLexeme() << endl;
        return out;
    }

    out << tokenName(t);

    if (t == ICONST || t == RCONST || t == BCONST)
        out << ": (" << tok.GetLexeme() << ")";
    else if (t == IDENT)
        out << ": <" << tok.GetLexeme() << ">";
    else if (t == SCONST || t == CCONST)
        out << ": '" << tok.GetLexeme() << "'";
    else if (t != DONE)
        out << ": \"" << tok.GetLexeme() << "\"";

    out << endl;
    return out;
}

map<string, Token> keyword_map = {
    { "and", AND },
    { "begin", BEGIN },
    { "boolean", BOOLEAN },
    { "char", CHAR },
    { "const", CONST },
    { "div", IDIV },
    { "end", END },
    { "else", ELSE },
    { "false", FALSE },
    { "if", IF },
    { "integer", INTEGER },
    { "mod", MOD },
    { "not", NOT },
    { "or", OR },
    { "program", PROGRAM },
    { "readln", READLN },
    { "real", REAL },
    { "string", STRING },
    { "then", THEN },
    { "true", TRUE },
    { "var", VAR },
    { "write", WRITE },
    { "writeln", WRITELN }
};

map<string, int> idTable;
map<string, int> kwTable;
map<string, int> stringTable;
map<string, int> intTable;
map<string, int> realTable;

LexItem id_or_kw(const string& lexeme, int linenum) {
    string key = lowerCopy(lexeme);
    map<string, Token>::iterator it = keyword_map.find(key);

    tokenCount++;

    if (it != keyword_map.end()) {
        Token t = it->second;

        if (t == TRUE || t == FALSE) {
            booleanCount++;
            return LexItem(BCONST, key, linenum);
        }

        keywordCount++;
        kwTable[key]++;
        return LexItem(t, key, linenum);
    }

    idCount++;
    idTable[lexeme]++;
    return LexItem(IDENT, lexeme, linenum);
}

static LexItem makeErr(const string& msg, int linenum) {
    return LexItem(ERR, msg, linenum);
}

static LexItem scanComment(istream& in, int& linenum, const string& opener) {
    char c;

    while (in.get(c)) {
        if (c == '\n') {
            linenum++;
            continue;
        }

        if (opener == "{" && c == '{')
            return makeErr("Invalid nesting of comments \"{{\"", linenum);

        if (opener == "(*" && c == '(' && in.peek() == '*') {
            in.get(c);
            return makeErr("Invalid nesting of comments \"(*(*\"", linenum);
        }

        if (opener == "{" && c == '}')
            return LexItem(DONE, "COMMENT_OK", linenum);

        if (opener == "(*" && c == '*' && in.peek() == ')') {
            in.get(c);
            return LexItem(DONE, "COMMENT_OK", linenum);
        }
    }

    return makeErr("Missing closing symbol(s) for a comment \"" + opener + "\"", linenum);
}

LexItem getNextToken(istream& in, int& linenum) {
    char c;

    while (in.get(c)) {
        if (isspace(static_cast<unsigned char>(c))) {
            if (c == '\n')
                linenum++;
            continue;
        }

        if (c == '{') {
            LexItem res = scanComment(in, linenum, "{");
            if (res == DONE && res.GetLexeme() == "COMMENT_OK")
                continue;
            return res;
        }

        if (c == '(' && in.peek() == '*') {
            in.get(c);
            LexItem res = scanComment(in, linenum, "(*");
            if (res == DONE && res.GetLexeme() == "COMMENT_OK")
                continue;
            return res;
        }

        if (isalpha(static_cast<unsigned char>(c))) {
            string lexeme(1, c);

            while (in.peek() != EOF) {
                char p = static_cast<char>(in.peek());
                if (isalnum(static_cast<unsigned char>(p)) || p == '_' || p == '$') {
                    in.get(p);
                    lexeme += p;
                }
                else
                    break;
            }

            return id_or_kw(lexeme, linenum);
        }

        if (isdigit(static_cast<unsigned char>(c))) {
            string lexeme(1, c);

            while (isdigit(in.peek()))
                lexeme += static_cast<char>(in.get());

            if (in.peek() == '.') {
                in.get(c);

                if (isdigit(in.peek())) {
                    lexeme += '.';

                    while (isdigit(in.peek()))
                        lexeme += static_cast<char>(in.get());

                    if (in.peek() == 'e' || in.peek() == 'E') {
                        lexeme += static_cast<char>(in.get());

                        if (in.peek() == 'e' || in.peek() == 'E') {
                            lexeme += static_cast<char>(in.get());
                            return makeErr("Invalid exponent for a floating-point constant \"" + lexeme + "\"", linenum);
                        }

                        if (in.peek() == '+' || in.peek() == '-') {
                            lexeme += static_cast<char>(in.get());

                            if (!isdigit(in.peek())) {
                                if (in.peek() != EOF && !isspace(in.peek()))
                                    lexeme += static_cast<char>(in.get());
                                return makeErr("Invalid exponent for floating-point constant \"" + lexeme + "\"", linenum);
                            }
                        }
                        else if (!isdigit(in.peek())) {
                            return makeErr("Invalid exponent for a floating-point constant \"" + lexeme + "\"", linenum);
                        }

                        while (isdigit(in.peek()))
                            lexeme += static_cast<char>(in.get());

                        if (isalpha(in.peek())) {
                            lexeme += static_cast<char>(in.get());
                            return makeErr("Invalid exponent for floating-point constant \"" + lexeme + "\"", linenum);
                        }
                    }

                    if (in.peek() == '.') {
                        lexeme += static_cast<char>(in.get());
                        if (isdigit(in.peek()))
                            lexeme += static_cast<char>(in.get());
                        return makeErr("Invalid floating-point constant \"" + lexeme + "\"", linenum);
                    }

                    tokenCount++;
                    numberCount++;
                    realTable[lexeme]++;
                    return LexItem(RCONST, lexeme, linenum);
                }
                else {
                    in.putback(c);
                    tokenCount++;
                    numberCount++;
                    intTable[lexeme]++;
                    return LexItem(ICONST, lexeme, linenum);
                }
            }

            tokenCount++;
            numberCount++;
            intTable[lexeme]++;
            return LexItem(ICONST, lexeme, linenum);
        }

        if (c == '\'') {
            string lexeme;
            int startLine = linenum;

            while (in.get(c)) {
                if (c == '\n') {
                    linenum++;
                    return makeErr("New line is not allowed within string literal \"'" + lexeme + "\"", startLine);
                }

                if (c == '\'') {
                    tokenCount++;
                    stringCount++;
                    if (lexeme.size() == 1)
                        return LexItem(CCONST, lexeme, linenum);

                    stringTable[lexeme]++;
                    return LexItem(SCONST, lexeme, linenum);
                }

                lexeme += c;
            }

            return makeErr("New line is not allowed within string literal \"'" + lexeme + "\"", startLine);
        }

        if (c == '"')
            return makeErr("Invalid character for starting a token \"" + string(1, c) + "\"", linenum);

        tokenCount++;

        switch (c) {
        case '+': return LexItem(PLUS, "+", linenum);
        case '-': return LexItem(MINUS, "-", linenum);
        case '*': return LexItem(MULT, "*", linenum);
        case '/': return LexItem(DIV, "/", linenum);
        case '=': return LexItem(EQ, "=", linenum);
        case '<': return LexItem(LTHAN, "<", linenum);
        case '>': return LexItem(GTHAN, ">", linenum);
        case ',': return LexItem(COMMA, ",", linenum);
        case ';': return LexItem(SEMICOL, ";", linenum);
        case '(': return LexItem(LPAREN, "(", linenum);
        case ')': return LexItem(RPAREN, ")", linenum);
        case '.': return LexItem(DOT, ".", linenum);
        case ':':
            if (in.peek() == '=') {
                in.get(c);
                return LexItem(ASSOP, ":=", linenum);
            }
            return LexItem(COLON, ":", linenum);
        default:
            tokenCount--;
            return makeErr("Invalid character for starting a token \"" + string(1, c) + "\"", linenum);
        }
    }

    return LexItem(DONE, "", linenum);
}