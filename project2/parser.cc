#include <iostream>
#include <vector>

#include "lexer.h"
#include "parser.h"

using namespace std;

int get_symbol_table_size()
{
    return symbol_table.size();
}

void syntax_error(int num)
{
    cout << "Syntax Error"<<endl;
    // cout << "Syntax Error: "<<num<<endl;
    exit(1); 
}

// vector<bool> did_lbrace_occur()
// {
        
// }

void add_glob_vector(string var_name, string var_scope, int var_instance)
{
    symbol_entry entry = {var_name, var_scope, var_instance};
    symbol_table.push_back(entry);
}

void delete_symbol_vector(string scope)
{
    int symtab_size = get_symbol_table_size();
    if(symtab_size > 0)
    {
        string variable_scope = symbol_table[symtab_size-1].var_scope;
        while(symtab_size > 0 && variable_scope == scope)
        {
            symbol_table.pop_back();
            symtab_size = get_symbol_table_size();
            if(symtab_size > 0) variable_scope = symbol_table[symtab_size-1].var_scope;
            else break;
        }
    }
}

string search_in_glob_vector(string variable_name, string variable_scope)
{
    string not_found = "?";
    int table_size = get_symbol_table_size();
    for(int i=table_size-1;i>=0;i--)
    {
        if(symbol_table[i].var_name == variable_name)
        {
            if(symbol_table[i].var_scope == variable_scope)
            {
                return symbol_table[i].var_scope;
            }
            else if(symbol_table[i].var_instance == 1)
            {
                return symbol_table[i].var_scope;
            }
            else if(symbol_table[i].var_instance == 0 )
            {
                return global_scope;
            }
        }
    }
    return not_found;
}

// Parser implementation

void parse_program()
{
    Token tok1 = lex.GetToken(), toknext;
    if(tok1.token_type == ID)
    {
        Token tok2 = lex.GetToken();
        if( tok2.token_type == COMMA )
        {
            lex.UngetToken(tok2);
            lex.UngetToken(tok1);
            parse_global_vars();
            parse_scope();
        }
        else if(tok2.token_type == SEMICOLON )
        {
            add_glob_vector(tok1.lexeme, global_scope,0);
            parse_scope();
            
        }
        else if(tok2.token_type == LBRACE)
        {
            lex.UngetToken(tok2);
            lex.UngetToken(tok1);
            parse_scope();
        }
        else
        {
            syntax_error(1);
        }
    }
    else
    {
        syntax_error(2);
    }
    // TODO: Handle EOF
    parse_eof();
}

void parse_eof()
{
    Token t1 = lex.GetToken();
    //if(t1 != RBRACE)
    // {
    //     Token t2 = lex.GetToken();
    //     if(t2 != END_OF_FILE)
    //         syntax_error();
    // }
    if(t1.token_type != END_OF_FILE)
    {
        syntax_error(15);
    }
}

void parse_global_vars()
{
    string global_scope = "::";
    Token tok1 = lex.GetToken();
    if(tok1.token_type == ID)
    {
        Token tok2 = lex.GetToken();
        if(tok2.token_type == COMMA)
        {
            lex.UngetToken(tok2);
            lex.UngetToken(tok1);
            vector<string> var_list = parse_var_list();
            for(int i = 0; i<=var_list.size()-1;i++)
            {
                add_glob_vector(var_list[i], global_scope, 0);
            }
            // print_symbol_vector();
        }
    }
    else
    {
        syntax_error(3);
    }
}

void parse_scope()
{
    Token tok1 = lex.GetToken();
    string curr_scope;
    if(tok1.token_type == ID)
    {
        Token tok2 = lex.GetToken();
        if(tok2.token_type == LBRACE)
        {
            curr_scope = tok1.lexeme;
            parse_public_vars(curr_scope); 
            parse_private_vars(curr_scope);
            parse_stmt_list(curr_scope);

            Token tok3 = lex.GetToken();
            if(tok3.token_type == RBRACE)
            {
                // print_symbol_vector();
                delete_symbol_vector(curr_scope);
                // print_symbol_vector();
            }
            else
            {
                syntax_error(4);
            }
        }
    }
}

void parse_private_vars(string curr_scope)
{
    Token tok1 = lex.GetToken();
    if(tok1.token_type == PRIVATE)
    {
        Token tok2 = lex.GetToken();
        if(tok2.token_type == COLON)
        {
            vector<string> var_list = parse_var_list();
            for(int i=0;i<=var_list.size()-1;i++)
            {
                add_glob_vector(var_list[i], curr_scope, 2);
            }
        }
        else
        {
            syntax_error(5);
        }
    }
    else
    {
        // syntax_error(5);
        lex.UngetToken(tok1);
    }
}

void parse_public_vars(string curr_scope)
{
    Token tok1 = lex.GetToken();
    if(tok1.token_type == PUBLIC)
    {
        Token tok2 = lex.GetToken();
        if(tok2.token_type == COLON)
        {
            vector<string> var_list = parse_var_list();
            for(int i=0;i<=var_list.size()-1;i++)
            {
                add_glob_vector(var_list[i], curr_scope, 1);
            }
        }
        else
        {
            syntax_error(6);
        }
    }
    else
    {
        // syntax_error(7);
        lex.UngetToken(tok1);
    }
}

vector<string> parse_var_list()
{
    vector<string> var_list;
    Token tok1 = lex.GetToken();
    if(tok1.token_type == SEMICOLON)
        return var_list;
    else if(tok1.token_type == COMMA)
        return parse_var_list();
    else if(tok1.token_type == ID)
    {
        var_list.push_back(tok1.lexeme);
        vector<string> rspnse = parse_var_list();
        // for(int i =0;i<parsed.size();i++)
        var_list.insert(var_list.end(), rspnse.begin(), rspnse.end());
        return var_list;
    }
    else
    {
        syntax_error(8);
    }
    vector<string> a;
    return a;
}

void parse_stmt(string curr_scope)
{
    Token tok1 = lex.GetToken();
    if(tok1.token_type == ID)
    {
        Token tok2 = lex.GetToken();
        if(tok2.token_type == EQUAL)
        {
            Token tok3 = lex.GetToken();
            Token tok4 = lex.GetToken();
            if(tok3.token_type == ID && tok4.token_type == SEMICOLON)
            {
                string left_scop = search_in_glob_vector(tok1.lexeme, curr_scope); 
                string rihgt_scop = search_in_glob_vector(tok3.lexeme, curr_scope); 
                if(left_scop.length()>0 && rihgt_scop.length()>0)
                {
                    string imp;
                    if(left_scop == global_scope && rihgt_scop == global_scope) imp = (left_scop+tok1.lexeme+" = "+rihgt_scop+tok3.lexeme);
                    else if(left_scop == global_scope && rihgt_scop != global_scope) imp = (left_scop+tok1.lexeme+" = "+rihgt_scop+"."+tok3.lexeme);
                    else if(left_scop != global_scope && rihgt_scop == global_scope) imp = (left_scop+"."+tok1.lexeme+" = "+rihgt_scop+tok3.lexeme);
                    else imp = (left_scop+"."+tok1.lexeme+" = "+rihgt_scop+"."+tok3.lexeme);
                    // print_symbol_vector();
                    print_this.push_back(imp);
                }
            }
            else
            {
                syntax_error(9);
            }
        }
        else if(tok2.token_type == LBRACE)
        {
            lex.UngetToken(tok2);
            lex.UngetToken(tok1);
            parse_scope();
        }
        else
        {
            syntax_error(10);
        }
    }
    else
    {
        // syntax_error(11);
        lex.UngetToken(tok1);
    }
}

void parse_stmt_list(string curr_scope)
{
    Token tok1 = lex.GetToken();
    if(tok1.token_type == ID)
    {
        while(tok1.token_type == ID)
        {
            lex.UngetToken(tok1);
            parse_stmt(curr_scope);
            tok1 = lex.GetToken();
        }

        if(tok1.token_type != RBRACE)
        {
            syntax_error(12);
        }
        else{
            lex.UngetToken(tok1);
        }
    }
    else
    {
        // cout << tok1.lexeme << endl;
        syntax_error(13);
    }
}

void print_symbol_vector()
{
    int tab_size = get_symbol_table_size();
    for(int i=0;i<=tab_size-1;i++)
    {
        cout << "here\t" << symbol_table[i].var_name << "\t" << symbol_table[i].var_scope <<endl;
    }
}

int main()
{
    parse_program();
    parse_eof();
    for(int i=0;i<=print_this.size()-1;i++)
    {
        cout << print_this[i]<<endl;
    }
}