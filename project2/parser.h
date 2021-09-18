#include <iostream>
#include <vector>

#include "lexer.h"

using namespace std;
struct symbol_entry{
    string var_name;
    string var_scope;
    int var_instance;
};

LexicalAnalyzer lex;
string global_scope = "::";
vector<struct symbol_entry> symbol_table;
vector<string> print_this;

// Scope functions 
int get_symbol_table_size();
void add_glob_vector(string var_name, string var_scope, int var_instance);
string search_in_glob_vector(string variable_name, string variable_scope);
void delete_symbol_vector(string closed_scope);

// Parse functions
void parse_program();
void parse_global_vars();
vector<string> parse_var_list();
void parse_scope();
void parse_public_vars(string curr_scope);
void parse_private_vars(string curr_scope);
void parse_stmt(string curr_scope);
void parse_stmt_list(string curr_scope);
void parse_eof();