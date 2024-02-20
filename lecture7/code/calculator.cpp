#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <map>

// Token “kind” values
char const number = '8';    // a floating-point number
char const quit = 'q';      // an exit command
char const print = ';';     // a print command
char const name = 'a';      // a variable name
char const assign = '=';    // an assignment command

std::map<std::string, double> variables;    // table of variables

class token {
    char kind_;
    double value_;
    std::string name_;

public:
    token(char ch) : kind_(ch), value_(0) {}
    token(double val) : kind_(number), value_(val) {}
    token(char ch, std::string n) : kind_(ch), value_(0), name_(n) {}

    char kind() const { return kind_; }
    double value() const { return value_; }
    std::string name() const { return name_; }
};

std::string const prompt = "> ";
std::string const result = "= ";

class token_stream {
    bool full;
    token buffer;

public:
    token_stream() : full(false), buffer('\0') {}

    token get();
    void putback(token t);
    void ignore(char c);
};

token_stream ts;

void token_stream::putback(token t) {
    if (full) {
        throw std::runtime_error("putback() into a full buffer");
    }
    buffer = t;
    full = true;
}

token token_stream::get() {
    if (full) {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> ch;

    switch (ch) {
        case print:
        case quit:
        case '(':
        case ')':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case assign:
            return token(ch);
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            std::cin.putback(ch);
            double val;
            std::cin >> val;
            return token(val);
        }
        default:
            if (isalpha(ch)) {
                std::string s;
                s += ch;
                while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) {
                    s += ch;
                }
                std::cin.putback(ch);
                if (s == "let") { // Handling 'let' for variable declarations (optional)
                    return token(name, s);
                } 
                else {
                    return token(name, s);  // Return a token with the variable name
                }
            }
            throw std::runtime_error("Bad token");
    }
}

void token_stream::ignore(char c) {
    if (full && c == buffer.kind()) {
        full = false;
        return;
    }
    full = false;

    char ch = 0;
    while (std::cin >> ch){
        if (ch == c) {
            return;
        }
    }
}

double expression(); // Forward declaration

double primary() {
    token t = ts.get();
    switch (t.kind()) {
        case '(': { //Parentheses handling
            double d = expression();
            t = ts.get();
            if (t.kind() != ')') throw std::runtime_error("')' expected");
            return d;
        }
        case number:
            return t.value();
        case name: {
            // Check if there is an assignment to the variable
            token next = ts.get();
            if (next.kind() == assign) {
                double d = expression();
                variables[t.name()] = d;  // Assign the value to the variable
                return d;
            } 
            else {
                ts.putback(next);
                return variables[t.name()]; // Return the value of the variable
            }
        }
        case '-': {
            return -primary();
        }
        default:{
            throw std::runtime_error("Primary expected");
        }
    }
}

double term() {
    double left = primary();
    token t = ts.get();

    while (true) {
        switch (t.kind()) {
            case '*':{ //Multiplication
                left *= primary();
                t = ts.get();
                break;
            }
            case '/': { //Division
                double d = primary();
                if (d == 0) throw std::runtime_error("Divide by zero");
                left /= d;
                t = ts.get();
                break;
            }
            case '%': {
                double d = primary();
                if (d == 0) throw std::runtime_error("Divide by zero"); //Divide by zero issue
                left = std::fmod(left, d); // Use fmod to get the remainder of the division
                t = ts.get();
                break;
            }
            default: {
                ts.putback(t);
                return left;
            }
        }
    }
}

double expression() {
    double left = term();
    token t = ts.get();

    while (true) {
        switch (t.kind()) {
            case '+':{ //Unary plus
                left += term();
                t = ts.get();
                break;
            }
            case '-': { //Unary minus
                left -= term();
                t = ts.get();
                break;
            }
            default: {
                ts.putback(t);
                return left;
            }
        }
    }
}

void calculate() {
    while (std::cin) {
        try {
            std::cout << prompt;
            token t = ts.get();
            while (t.kind() == print) t = ts.get();  // Remove “print” tokens
            if (t.kind() == quit) return;
            ts.putback(t); //Put back t
            std::cout << result << expression() << std::endl; // Print the result
        } 
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            ts.ignore(print);
        }
    }
}

int main() {
    variables["pi"] = 3.14159;
    variables["e"] = 2.71828;
    variables["abszero"] = -273.15;
    variables["k"] = 1000;
    variables["itoc"] = 2.54;
    variables["ftom"] = 0.3048;
    variables["days"] = 365.25;
    variables["hours"] = 24;
    variables["minutes"] = 60;
    variables["seconds"] = 60;
    variables["months"] = 12;
    variables["golden"] = 1.6180339887;
    variables["light"] = 299792458;
    try {
        calculate();
        return 0;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}