/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE",
    "IF", "WHILE", "DO", "THEN", "PRINT",
    "PLUS", "MINUS", "DIV", "MULT",
    "EQUAL", "COLON", "COMMA", "SEMICOLON",
    "LBRAC", "RBRAC", "LPAREN", "RPAREN",
    "NOTEQUAL", "GREATER", "LESS", "LTEQ", "GTEQ",
    "DOT", "NUM", "ID", "ERROR", // TODO: Add labels for new token types here
    "REALNUM", "BASE08NUM", "BASE16NUM"
};

#define KEYWORDS_COUNT 5
string keyword[] = { "IF", "WHILE", "DO", "THEN", "PRINT" };

bool digit_gte_8_occured = false;

void Token::Print()
{
    cout << "{" << this->lexeme << " , " << reserved[(int) this->token_type] << " , " << this->line_no << "}\n";
}

bool LexicalAnalyzer::check_dot() 
{
    char chr;
    input.GetChar(chr);
    if(input.EndOfInput() || chr != '.') 
    {
        if(!input.EndOfInput()) 
        {
            input.UngetChar(chr);
        }
        return false;
    }
    return true;
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::check_if_base(int a) 
{
    char ch;
    vector<string> lookup;
    lookup.push_back("x08");
    lookup.push_back("x16");
    int temp = 0;
    while(temp<3)
    {
        input.GetChar(ch);
        if(input.EndOfInput() || ch!=lookup[a][temp]) 
        {
            if(!input.EndOfInput()) 
            {
                input.UngetChar(ch);
            }
            for(int i=0; i<temp; i++)
            {
                input.UngetChar(lookup[a][temp-i-1]);
            }
            return false;
        }
        temp++;
    }
    return true;
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while(!input.EndOfInput() && isspace(c)) 
    {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if(!input.EndOfInput()) 
    {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for(int i = 0; i < KEYWORDS_COUNT; i++) 
    {
        if(s == keyword[i]) 
        {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for(int i = 0; i < KEYWORDS_COUNT; i++) 
    {
        if(s == keyword[i]) 
        {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

void LexicalAnalyzer::set_temp(string lex, TokenType tt)
{
    tmp.lexeme = lex;
    tmp.line_no = line_no; 
    tmp.token_type = tt;
}

Token LexicalAnalyzer::check_all_num_types() 
{
    string checked_token;
    bool exit = 0;

    checked_token = get_b16_num();
    if (checked_token.length() > 0) {
        set_temp(checked_token, BASE16NUM);
        return tmp;
    }
    checked_token = get_b08_num();
    if(checked_token.length() > 0) 
    {
        set_temp(checked_token, BASE08NUM);
        return tmp;
    }
    checked_token = get_real_num();
    if(checked_token.length() > 0) 
    {
        set_temp(checked_token, REALNUM);
        return tmp;
    }
    checked_token = get_num();
    if(checked_token.length() > 0) 
    {
        set_temp(checked_token, NUM);
        return tmp;
    }
    set_temp("", ERROR);
    return tmp;
}

string LexicalAnalyzer::get_num()
{
    char ch;
    string curr_lex = "";

    input.GetChar(ch);
    curr_lex += ch;
    if (ch == '0') 
    {
        return curr_lex;
    } 
    else if(ch>'0' && ch <= '9')
    {
        input.GetChar(ch);
        while (!input.EndOfInput() && ch >= '0' && ch <= '9') 
        {
            curr_lex += ch;
            input.GetChar(ch);
        }
        if (!input.EndOfInput()) 
        {
            input.UngetChar(ch);
        }
        return curr_lex;
    }
    input.UngetString(curr_lex);
    return "";
}

string LexicalAnalyzer::get_b16_num()
{
    char c; 
    string curr_lex = "";
    input.GetChar(c);
    curr_lex += c;
    int type = 1;   
    if((c >= '1' && c <= '9') || (c >= 'A' && c <= 'F')) 
    {
        input.GetChar(c);
        while(!input.EndOfInput() && (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) 
        {
            curr_lex += c;
            input.GetChar(c);
        }
        if(!input.EndOfInput()) 
        {
            input.UngetChar(c);
        }
        if(check_if_base(type)) 
        {
            curr_lex += "x16";
            return curr_lex;
        }
    }
    else if(c == '0') 
    {
        if(check_if_base(type)) 
        {
            curr_lex += "x16";
            return curr_lex;
        }
    } 
    input.UngetString(curr_lex);
    return "";
}


string LexicalAnalyzer::get_b08_num() 
{
    char ctr;
    string curr_lex = "";
    input.GetChar(ctr);
    curr_lex += ctr;
    int type = 0;
    if(ctr >= '1' && ctr <= '7') 
    {
        input.GetChar(ctr);
        while(!input.EndOfInput() && ctr >= '0' && ctr <= '7') 
        {
            curr_lex += ctr;
            input.GetChar(ctr);
        }
        if(!input.EndOfInput()) 
        {
            input.UngetChar(ctr);
        }
        if(check_if_base(type)) 
        {
            curr_lex += "x08";
            // cout << "This is it right here too "<< curr_lex<<endl;
            return curr_lex;
        }
    }
    else if(ctr == '0') 
    {
        if(check_if_base(type)) 
        {
            curr_lex += "x08";
            // cout << "This is it right here "<< curr_lex<<endl;
            return curr_lex; 
        }
    } 
    input.UngetString(curr_lex);
    return "";
}


Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);

    if(isalpha(c)) 
    {
        tmp.lexeme = "";
        while(!input.EndOfInput() && isalnum(c)) 
        {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        if(!input.EndOfInput()) 
        {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        if(IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } 
    else 
    {
        if(!input.EndOfInput()) 
        {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

// you should unget tokens in the reverse order in which they
// are obtained. If you execute
//
//    t1 = lexer.GetToken();
//    t2 = lexer.GetToken();
//    t3 = lexer.GetToken();
//
// in this order, you should execute
//
//    lexer.UngetToken(t3);
//    lexer.UngetToken(t2);
//    lexer.UngetToken(t1);
//
// if you want to unget all three tokens. Note that it does not
// make sense to unget t1 without first ungetting t2 and t3
//
TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

string LexicalAnalyzer::get_real_num() 
{
    char cr;
    string curr_lex = "";
    input.GetChar(cr);
    curr_lex += cr;
    bool pdig_after_dec = false;
    int num_digits_after_dec = 0;
    int last_occured_pdig = 0;
    if(cr == '0') 
    {
        if(check_dot()) 
        {
            curr_lex += '.';
            input.GetChar(cr);
            if(!input.EndOfInput() && cr>='0'&&cr<='9') 
            {
                while(!input.EndOfInput() && cr>='0'&&cr<='9') 
                {
                    curr_lex += cr; 
                    input.GetChar(cr);
                }
                if(!input.EndOfInput()) 
                {
                    input.UngetChar(cr);
                }
                return curr_lex;
            }
        }
    }
    
    else if(cr >= '1' && cr <= '9') 
    {
        input.GetChar(cr);
        while(!input.EndOfInput() && cr >= '0' && cr <= '9') 
        {
            curr_lex += cr;
            input.GetChar(cr);
        }
        if(!input.EndOfInput()) {
            input.UngetChar(cr);
        }
        if(check_dot()) 
        {
            curr_lex += '.';
            pdig_after_dec = false;
            input.GetChar(cr);
            while(!input.EndOfInput() && cr >= '0' && cr <= '9') 
            {
                if(cr>'0'&&cr<=9)
                {
                    pdig_after_dec = true;
                    last_occured_pdig = num_digits_after_dec;
                }
                num_digits_after_dec++;
                curr_lex += cr;
                input.GetChar(cr);
            }
            if(!input.EndOfInput()) 
            {
                input.UngetChar(cr);
            }
            if(pdig_after_dec)
            {
                return curr_lex;
            }
        }
    }
    input.UngetString(curr_lex);
    return "";
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if(!tokens.empty()) 
    {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    // tmp.lexeme += c;
    switch (c) {
        case '.':
            tmp.token_type = DOT;
            // tmp.lexeme += c;
            return tmp;
        case '+':
            tmp.token_type = PLUS;
            // tmp.lexeme += c;
            return tmp;
        case '-':
            tmp.token_type = MINUS;
            // tmp.lexeme += c;
            return tmp;
        case '/':
            tmp.token_type = DIV;
            // tmp.lexeme += c;
            return tmp;
        case '*':
            tmp.token_type = MULT;
            // tmp.lexeme += c;
            return tmp;
        case '=':
            tmp.token_type = EQUAL;
            // tmp.lexeme += c;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            // tmp.lexeme += c;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            // tmp.lexeme += c;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            // tmp.lexeme += c;
            return tmp;
        case '[':
            tmp.token_type = LBRAC;
            // tmp.lexeme += c;
            return tmp;
        case ']':
            tmp.token_type = RBRAC;
            // tmp.lexeme += c;
            return tmp;
        case '(':
            tmp.token_type = LPAREN;
            // tmp.lexeme += c;
            return tmp;
        case ')':
            tmp.token_type = RPAREN;
            return tmp;
        case '<':
            input.GetChar(c);
            if(c == '=') {
                tmp.token_type = LTEQ;
            } else if(c == '>') {
                tmp.token_type = NOTEQUAL;
            } else {
                if(!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = LESS;
            }
            return tmp;
        case '>':
            input.GetChar(c);
            if(c == '=') {
                tmp.token_type = GTEQ;
            } else {
                if(!input.EndOfInput()) {
                    input.UngetChar(c);
                }
                tmp.token_type = GREATER;
            }
            return tmp;
        default:
            if(c >= '0' && c <= '9') {
                input.UngetChar(c);
                return check_all_num_types();
            } else if(isalpha(c)) {
                input.UngetChar(c);
                return ScanIdOrKeyword();
            } else if(input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

int main()
{
    LexicalAnalyzer lexer;
    Token token;

    token = lexer.GetToken();
    token.Print();
    while(token.token_type != END_OF_FILE)
    {
        token = lexer.GetToken();
        token.Print();
    }
}
