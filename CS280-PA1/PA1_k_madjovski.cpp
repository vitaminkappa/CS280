#include "lex.h"
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <cctype>

using namespace std;

extern int tokenCount;
extern int idCount;
extern int keywordCount;
extern int numberCount;
extern int booleanCount;
extern int stringCount;

extern map<string, int> stringTable;
extern map<string, int> intTable;
extern map<string, int> realTable;
extern map<string, int> idTable;
extern map<string, int> kwTable;

static bool isWhitespaceOnly(const string& s) {
    for (size_t i = 0; i < s.size(); i++) {
        if (!isspace(static_cast<unsigned char>(s[i])))
            return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    int lineNumber = 1;
    int summaryLines = 0;
    bool printAll = false;
    bool printNum = false;
    bool printStr = false;
    bool printIds = false;
    int idkwCount = 0;
    int fileCount = 0;
    string fileName = "";
    LexItem tok;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (!arg.empty() && arg[0] == '-') {
            if (arg == "-all")
                printAll = true;
            else if (arg == "-num")
                printNum = true;
            else if (arg == "-str")
                printStr = true;
            else if (arg == "-ids")
                printIds = true;
            else {
                cout << "UNRECOGNIZED FLAG {" << arg << "}" << endl;
                return 0;
            }
        }
        else {
            fileName = arg;
            fileCount++;
        }
    }

    if (fileCount == 0) {
        cout << "NO SPECIFIED INPUT FILE." << endl;
        return 0;
    }

    if (fileCount > 1) {
        cout << "ONLY ONE FILE NAME IS ALLOWED." << endl;
        return 0;
    }

    ifstream infile(fileName.c_str());
    if (!infile.is_open()) {
        cout << "CANNOT OPEN THE FILE " << fileName << endl;
        return 0;
    }

    if (infile.peek() == EOF) {
        cout << "Empty File." << endl;
        return 0;
    }

    {
        ifstream countIn(fileName.c_str(), ios::binary);
        string tmp, lastLine = "";

        while (getline(countIn, tmp)) {
            summaryLines++;
            lastLine = tmp;
        }

        countIn.clear();
        countIn.seekg(0, ios::end);
        streamoff len = countIn.tellg();

        if (len > 0) {
            countIn.seekg(len - 1);
            char lastChar;
            countIn.get(lastChar);

            if (lastChar != '\n' && lastChar != '\r' && isWhitespaceOnly(lastLine))
                summaryLines--;
        }
    }

    while (true) {
        tok = getNextToken(infile, lineNumber);

        if (tok.GetToken() == ERR) {
            cout << tok;
            return 0;
        }

        if (tok.GetToken() == DONE)
            break;

        if (printAll)
            cout << tok;
    }

    idkwCount = idCount + keywordCount;

    cout << '\n';
    cout << "Lines: " << summaryLines << endl;
    cout << "Total Tokens: " << tokenCount << endl;
    cout << "Identifiers & Keywords: " << idkwCount << endl;
    cout << "Numbers: " << numberCount << endl;
    cout << "Booleans: " << booleanCount << endl;
    cout << "Strings: " << stringCount << endl;

    if (printIds) {
        if (!idTable.empty()) {
            cout << "IDENTIFIERS:" << endl;

            int count = 0;
            int size = (int)idTable.size();
            map<string, int>::iterator it;

            for (it = idTable.begin(); it != idTable.end(); ++it) {
                cout << it->first << " (" << it->second << ")";
                count++;
                if (count < size)
                    cout << ", ";
            }
            cout << endl;
        }

        if (!kwTable.empty()) {
            if (idTable.empty())
                cout << endl;

            cout << "KEYWORDS:" << endl;

            int count = 0;
            int size = (int)kwTable.size();
            map<string, int>::iterator it;

            for (it = kwTable.begin(); it != kwTable.end(); ++it) {
                cout << it->first << " (" << it->second << ")";
                count++;
                if (count < size)
                    cout << ", ";
            }
            cout << endl;
        }
    }

    if (printNum) {
        if (!intTable.empty()) {
            cout << "INTEGERS:" << endl;

            int count = 0;
            int size = (int)intTable.size();
            map<string, int>::iterator it;

            for (it = intTable.begin(); it != intTable.end(); ++it) {
                cout << it->first << " (" << it->second << ")";
                count++;
                if (count < size)
                    cout << ", ";
            }
            cout << endl;
        }

        if (!realTable.empty()) {
            cout << "REALS:" << endl;

            int count = 0;
            int size = (int)realTable.size();
            map<string, int>::iterator it;

            for (it = realTable.begin(); it != realTable.end(); ++it) {
                cout << it->first << " (" << it->second << ")";
                count++;
                if (count < size)
                    cout << ", ";
            }
            cout << endl;
        }
    }

    if (printStr) {
        if (!stringTable.empty()) {
            cout << "STRINGS:" << endl;

            int count = 0;
            int size = (int)stringTable.size();
            map<string, int>::iterator it;

            for (it = stringTable.begin(); it != stringTable.end(); ++it) {
                cout << "'" << it->first << "'" << " (" << it->second << ")";
                count++;
                if (count < size)
                    cout << ", ";
            }
            cout << endl;
        }
    }

    return 0;
}