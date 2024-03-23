#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <dlfcn.h>

using namespace std;

// Token 종류
enum TokenType {
    TEXT,
    VAR_DECL,
    VAR_ASSIGN,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    END,
    IF,
    ELSE,
    FOR,
    ELSEIF,
    IMPORT // 새로운 토큰 추가: IMPORT
};

// Token 구조체
struct Token {
    TokenType type;
    string value;
};

// Lexer 클래스
class Lexer {
public:
    Lexer(const string& input) : input(input), position(0) {}

    Token getNextToken() {
        if (position >= input.length()) {
            return {END, ""};
        }

        char currentChar = input[position++];

        if (currentChar == '&') {
            return {VAR_DECL, "&"};
        } else if (currentChar == ':') {
            return {VAR_ASSIGN, ":"};
        } else if (currentChar == '+') {
            return {ADD, "+"};
        } else if (currentChar == '-') {
            return {SUB, "-"};
        } else if (currentChar == '*') {
            return {MUL, "*"};
        } else if (currentChar == '/') {
            return {DIV, "/"};
        } else if (currentChar == '%') {
            return {MOD, "%"};
        } else if (isspace(currentChar)) {
            // 공백 무시
            return getNextToken();
        } else if (currentChar == 'i') {
            string keyword = "if";
            for (char c : keyword) {
                if (position >= input.length() || input[position++] != c) {
                    throw invalid_argument("Unexpected token.");
                }
            }
            return {IF, keyword};
        } else if (currentChar == 'e') {
            string keyword = "else";
            for (char c : keyword) {
                if (position >= input.length() || input[position++] != c) {
                    throw invalid_argument("Unexpected token.");
                }
            }
            return {ELSE, keyword};
        } else if (currentChar == 'f') {
            string keyword = "for";
            for (char c : keyword) {
                if (position >= input.length() || input[position++] != c) {
                    throw invalid_argument("Unexpected token.");
                }
            }
            return {FOR, keyword};
        } else if (currentChar == 'e') {
            string keyword = "elseif";
            for (char c : keyword) {
                if (position >= input.length() || input[position++] != c) {
                    throw invalid_argument("Unexpected token.");
                }
            }
            return {ELSEIF, keyword};
        } else if (currentChar == 'i') {
            string keyword = "import";
            for (char c : keyword) {
                if (position >= input.length() || input[position++] != c) {
                    throw invalid_argument("Unexpected token.");
                }
            }
            return {IMPORT, keyword};
        } else {
            // TEXT 토큰 수집
            string textValue(1, currentChar);
            while (position < input.length() && isalnum(input[position])) {
                textValue += input[position++];
            }
            return {TEXT, textValue};
        }
    }

private:
    string input;
    size_t position;
};

// 컴파일러 클래스
class Compiler {
public:
    Compiler(const string& sourceFilename, const string& outputFilename) 
        : sourceFilename(sourceFilename), outputFilename(outputFilename) {}

    void compile() {
        ifstream inputFile(sourceFilename);
        if (!inputFile) {
            cerr << "파일을 열 수 없습니다." << endl;
            return;
        }

        ofstream outputFile(outputFilename, ios::binary);
        if (!outputFile) {
            cerr << "출력 파일을 열 수 없습니다." << endl;
            return;
        }

        Lexer lexer(readFile(inputFile));
        Token token;
        bool isFirstStatement = true;
        while ((token = lexer.getNextToken()).type != END) {
            if (isFirstStatement) {
                cout << "welcome to 태백 compiler!" << endl;
                isFirstStatement = false;
            }

            switch (token.type) {
                case TEXT:
                    // TEXT 토큰 처리
                    outputFile << token.value;
                    break;
                case VAR_DECL:
                    // 변수 선언
                    processVarDecl(lexer, outputFile);
                    break;
                case VAR_ASSIGN:
                    // 변수 할당
                    processVarAssign(lexer, outputFile);
                    break;
                case ADD:
                case SUB:
                case MUL:
                case DIV:
                case MOD:
                    // 산술 연산
                    processArithmetic(token, lexer, outputFile);
                    break;
                case IF:
                    // IF 문 처리
                    processIfStatement(lexer, outputFile);
                    break;
                case ELSE:
                    // ELSE 문 처리
                    processElseStatement(lexer, outputFile);
                    break;
                case FOR:
                    // FOR 루프 처리
                    processForLoop(lexer, outputFile);
                    break;
                case ELSEIF:
                    // ELSEIF 함수 처리
                    processElseifFunction(lexer, outputFile);
                    break;
                case IMPORT:
                    // IMPORT 문 처리
                    processImport(lexer, outputFile);
                    break;
                default:
                    cerr << "알 수 없는 토큰입니다." << endl;
                    return;
            }
        }

        cout << "컴파일이 완료되었습니다." << endl;
    }

private:
    string sourceFilename;
    string outputFilename;
    unordered_map<string, int> variables;

    // 파일 읽기 함수
    string readFile(ifstream& file) {
        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // 변수 선언 처리 함수
    void processVarDecl(Lexer& lexer, ofstream& outputFile) {
        Token token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "변수 이름이 올바르지 않습니다." << endl;
            return;
        }
        variables[token.value] = 0; // 변수 맵에 추가
    }

    // 변수 할당 처리 함수
    void processVarAssign(Lexer& lexer, ofstream& outputFile) {
        Token token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "변수 이름이 올바르지 않습니다." << endl;
            return;
        }
        string varName = token.value;

        token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "값이 올바르지 않습니다." << endl;
            return;
        }
        variables[varName] = stoi(token.value); // 변수에 값 할당
    }

    // 산술 연산 처리 함수
    void processArithmetic(Token opToken, Lexer& lexer, ofstream& outputFile) {
        int result
        int result = 0;
        string varName;
        Token token = lexer.getNextToken();
        if (token.type == TEXT) {
            varName = token.value;
            if (variables.find(varName) == variables.end()) {
                cerr << "변수가 선언되지 않았습니다." << endl;
                return;
            }
            result = variables[varName];
        }
        else {
            cerr << "값이 올바르지 않습니다." << endl;
            return;
        }

        token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "값이 올바르지 않습니다." << endl;
            return;
        }
        int operand = stoi(token.value);

        switch (opToken.type) {
            case ADD:
                result += operand;
                break;
            case SUB:
                result -= operand;
                break;
            case MUL:
                result *= operand;
                break;
            case DIV:
                if (operand != 0)
                    result /= operand;
                else {
                    cerr << "0으로 나눌 수 없습니다." << endl;
                    return;
                }
                break;
            case MOD:
                if (operand != 0)
                    result %= operand;
                else {
                    cerr << "0으로 나눌 수 없습니다." << endl;
                    return;
                }
                break;
            default:
                cerr << "올바르지 않은 연산자입니다." << endl;
                return;
        }

        // 결과를 변수에 다시 할당
        variables[varName] = result;
    }

    // IF 문 처리 함수
    void processIfStatement(Lexer& lexer, ofstream& outputFile) {
        Token token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "IF 조건이 올바르지 않습니다." << endl;
            return;
        }
        string varName = token.value;
        if (variables.find(varName) == variables.end()) {
            cerr << "변수가 선언되지 않았습니다." << endl;
            return;
        }

        // 변수 값에 따라 처리
        if (variables[varName] != 0) {
            // 조건이 참이면 다음 문장 실행
            compile();
        } else {
            // 거짓이면 ELSE 문을 찾아 건너뛰기
            skipToElse(lexer);
        }
    }

    // ELSE 문 처리 함수
    void processElseStatement(Lexer& lexer, ofstream& outputFile) {
        // ELSE 문을 찾아 건너뛰기
        skipToElse(lexer);
    }

    // ELSE 문을 건너뛰는 함수
    void skipToElse(Lexer& lexer) {
        Token token;
        while ((token = lexer.getNextToken()).type != END) {
            if (token.type == ELSE) {
                // ELSE 문을 발견하면 루프 종료
                return;
            }
        }
        cerr << "ELSE 문이 없습니다." << endl;
    }

    // FOR 루프 처리 함수
    void processForLoop(Lexer& lexer, ofstream& outputFile) {
        Token token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "FOR 루프 형식이 올바르지 않습니다." << endl;
            return;
        }
        string varName = token.value;
        if (variables.find(varName) == variables.end()) {
            cerr << "변수가 선언되지 않았습니다." << endl;
            return;
        }

        token = lexer.getNextToken();
        if (token.type != VAR_ASSIGN) {
            cerr << "FOR 루프 형식이 올바르지 않습니다." << endl;
            return;
        }

        token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "FOR 루프 형식이 올바르지 않습니다." << endl;
            return;
        }
        int initialValue = stoi(token.value);

        token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "FOR 루프 형식이 올바르지 않습니다." << endl;
            return;
        }
        int endValue = stoi(token.value);

        // FOR 루프 실행
        for (int i = initialValue; i <= endValue; ++i) {
            variables[varName] = i;
            compile();
        }
    }

    // ELSEIF 함수 처리 함수
    void processElseifFunction(Lexer& lexer, ofstream& outputFile) {
        Token token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "ELSEIF 함수 형식이 올바르지 않습니다." << endl;
            return;
        }
        string varName = token.value;
        if (variables.find(varName) == variables.end()) {
            cerr << "변수가 선언되지 않았습니다." << endl;
            return;
        }

        // 변수 값이 0이 아니면 조건이 참
        if (variables[varName] != 0) {
            compile(); // 조건이 참이면 다음 문장 실행
        } else {
            // 조건이 거짓이면 ELSE 문을 찾아 건너뛰기
            skipToElse(lexer);
        }
    }

    // IMPORT 문 처리 함수
    void processImport(Lexer& lexer, ofstream& outputFile) {
        Token token = lexer.getNextToken();
        if (token.type != TEXT) {
            cerr << "IMPORT 문 형식이 올바르지 않습니다." << endl;
            return;
        }
        string libName = token.value + ".so"; // 라이브러리 이름에 확장자 추가
        void* handle = dlopen(libName.c_str(), RTLD_LAZY); // 라이브러리 로드
        if (!handle) {
            cerr << "라이브러리를 로드할 수 없습니다: " << dlerror() << endl;
            return;
        }
        cout << "라이브러리 " << libName << "를 성공적으로 로드했습니다." << endl;

        // 라이브러리에서 함수를 사용하는 코드 작성
        typedef void (*FunctionType)();
        FunctionType function = (FunctionType)dlsym(handle, "some_function");
        if (!function) {
            cerr << "라이브러리에서 함수를 찾을 수 없습니다." << endl;
            dlclose(handle);
            return;
        }
        cout << "라이브러리에서 함수를 성공적으로 불러왔습니다." << endl;
                function(); // 함수 호출

        // 라이브러리 핸들 해제
        dlclose(handle);
    }

    int main() {
        // 컴파일할 소스 파일 이름과 컴파일된 파일 이름
        string sourceFilename = "source.tb";
        string outputFilename = "output.bin";

        // 컴파일러 객체 생성 및 컴파일 실행
        Compiler compiler(sourceFilename, outputFilename);
        compiler.compile();

        return 0;
    }
};
