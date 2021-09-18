#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"

using namespace std;

struct variable_entry{
    int loc;
    string var_name;
};

vector<struct variable_entry*> var_table;
LexicalAnalyzer lexer;

int input_master(string inpt);
int error_code=0;
Token match();
struct InstructionNode* parse_program();
struct InstructionNode* parse_stmt();
struct InstructionNode* parse_stmt_list();
struct InstructionNode* parse_assign_stmt();
void parse_expression(struct InstructionNode* expression);

// stmtparse
struct InstructionNode* parse_while();
struct InstructionNode* parse_if();
struct InstructionNode* parse_switch();
struct InstructionNode* parse_case_list(struct InstructionNode* closing_noop, string curr_variable);
struct InstructionNode* parse_default_case(struct InstructionNode* def_inst);
struct InstructionNode* parse_case(struct InstructionNode* jmp_node, string curr_variable);
struct InstructionNode* parse_for();
struct InstructionNode* parse_input();
struct InstructionNode* parse_output();
struct InstructionNode* parse_body();
void parse_var_section();
void parse_id_list();
int parse_prim();
void parse_input_nums();
ArithmeticOperatorType get_operator_type();
ConditionalOperatorType parse_reloperator();
void parse_cond(struct InstructionNode* cond);

int check_if = 0;
int check_while = 0;
int check_switch = 0;
int check_for = 0;