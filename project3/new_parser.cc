#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "compiler.h"
#include "new_parser.h"

using namespace std;

void syntax_error()
{
    // cout << "SYNTAX ERROR\n"<<error_code++;
    cout << "SYNTAX ERROR\n";
    exit(1);
}

int index_master(string inp)
{
    for(int i=0; i< var_table.size(); i++)
    {
        if (var_table[i]->var_name == inp)
        {
            return var_table[i]->loc;
        } 
    }
    return -1;
}

Token match(TokenType req_tok_type)
{
    Token tok = lexer.GetToken();
    if(tok.token_type != req_tok_type) syntax_error();
    return tok;
}

ConditionalOperatorType parse_reloperator()
{
    Token tok1 = lexer.peek(1);
    // lexer.UngetToken(1);
    if(tok1.token_type == NOTEQUAL)
    {
        match(NOTEQUAL);
        return CONDITION_NOTEQUAL;
    }
    if(tok1.token_type == LESS)
    {
        match(LESS);
        return CONDITION_LESS;
    }
    if(tok1.token_type == GREATER)
    {
        match(GREATER);
        return CONDITION_GREATER;
    }
    syntax_error;
}

struct InstructionNode* parse_generate_intermediate_representation()
{
    struct InstructionNode* final_inst = new InstructionNode;
    parse_var_section();
	final_inst = parse_body();
	parse_input_nums();
	match(END_OF_FILE);
    return final_inst;
}

void parse_id_list()
{
    int first = next_available;
    next_available++;
    struct variable_entry* var_e = new struct variable_entry;
    Token tok1 = match(ID);
    var_e->var_name = tok1.lexeme;
    var_e->loc = first;
    var_table.push_back(var_e);
    Token tok2 = lexer.peek(1);
    if(tok2.token_type == COMMA)
    {
        match(COMMA);
        parse_id_list();
    }
    else if(tok2.token_type == SEMICOLON)
    {
        // continue;
        return;
    }
    else
    {
        syntax_error();
    }
}

struct InstructionNode* parse_stmt()
{
    struct InstructionNode* inst1;
    Token tok1 = lexer.peek(1);
    if(tok1.token_type == ID)
    {
        inst1 = parse_assign_stmt();
    }
    else if(tok1.token_type == SWITCH)
    {
        inst1 = parse_switch();
    }
    else if(tok1.token_type == FOR)
    {
        inst1 = parse_for();
    }
    else if(tok1.token_type == INPUT)
    {
        inst1 = parse_input();
    }
    else if(tok1.token_type == OUTPUT)
    {
        inst1 = parse_output();
    }
    else if(tok1.token_type == WHILE)
    {
        inst1 = parse_while();
    }
    else if(tok1.token_type == IF)
    {
        inst1 = parse_if();
    }
    else
    {
        syntax_error();
    }
    return inst1;
}

struct InstructionNode* parse_stmt_list()
{
    struct InstructionNode* inst1 =  parse_stmt();
    struct InstructionNode* head_tem;
    struct InstructionNode* inst2;

    Token tok1 = lexer.peek(1);
    if(tok1.token_type == ID || tok1.token_type == WHILE  || tok1.token_type == INPUT || tok1.token_type == OUTPUT || tok1.token_type == IF || tok1.token_type == SWITCH || tok1.token_type == FOR)
    {
        if(check_if == 1)
        {
            check_if = 0;
            inst1->cjmp_inst.target->next = parse_stmt_list();
        }
        else if(check_while == 1)
        {
            check_while = 0;
            inst1->cjmp_inst.target->next = parse_stmt_list();
        }
        else if(check_for == 1)
        {
            check_for = 0;
            inst1->next->cjmp_inst.target->next = parse_stmt_list();
        }
        else if(check_switch == 1)
        {
            check_switch = 0;
            inst2 = parse_stmt_list();
            head_tem = inst1;
            while(head_tem->next != NULL) head_tem = head_tem->next;
            head_tem->next = inst2;
        }
        else
        {
            inst1->next = parse_stmt_list();
        }
        return inst1;    
    }
    else if(tok1.token_type == RBRACE)
    {
        return inst1;
    }
    else 
    {
        syntax_error();
    }
    return nullptr;
}

void parse_var_section()
{
    parse_id_list();
    match(SEMICOLON);
}

struct InstructionNode* parse_body()
{
    match(LBRACE);
    struct InstructionNode* inst_set_body;
    inst_set_body = parse_stmt_list();
    match(RBRACE);
    return inst_set_body;
}

struct InstructionNode* parse_assign_stmt()
{
    int ans;
    struct InstructionNode* assn_inst1 = new InstructionNode;
    assn_inst1->type = ASSIGN;
    Token LHS = match(ID);
    assn_inst1->assign_inst.left_hand_side_index = index_master(LHS.lexeme);
    assn_inst1->next = NULL;
    match(EQUAL);
    Token tok1 = lexer.peek(1);
    Token tok2 = lexer.peek(2);
    if(tok1.token_type == ID || tok1.token_type == NUM)
    {
        if(tok2.token_type == PLUS || tok2.token_type == MINUS || tok2.token_type == MULT || tok2.token_type == DIV)
        {
            parse_expression(assn_inst1);
            match(SEMICOLON);
            return assn_inst1;
        }
        else if(tok2.token_type == SEMICOLON)
        {   
            ans = parse_prim();
            // assn_inst1->next = NULL;
            assn_inst1->assign_inst.operand2_index = 0;
            assn_inst1->assign_inst.operand1_index = ans;
            assn_inst1->assign_inst.op = OPERATOR_NONE;
            assn_inst1->next = NULL;

            match(SEMICOLON);
            return assn_inst1;
        }
        else
        {
            syntax_error();
        }
    }
    else
    {
        syntax_error();
    }
    return nullptr;
}

int parse_prim()
{
    Token tok1 = lexer.peek(1);
    Token tok2;
    if(tok1.token_type == ID)
    {
        tok2 = match(ID);
        string s = tok2.lexeme;
        // s = s+"";
        return(index_master(s));
    }
    else if(tok1.token_type == NUM)
    {
        struct variable_entry* var_e = new struct variable_entry;
        tok2 = match(NUM);
        int first = next_available;
        var_e->var_name = tok2.lexeme;
        var_e->loc = first;
        var_table.push_back(var_e);
        mem[first] = stoi(tok2.lexeme);
        // mem[next_available] = stoi(tok2.lexeme);
        next_available++;
        return(index_master(tok2.lexeme));
    }
    else
    {
        syntax_error();
    }
    return 0;
}

ArithmeticOperatorType get_operator_type()
{
    Token tok1 = lexer.peek(1);
    if(tok1.token_type == PLUS)
    {
        match(PLUS);
        return OPERATOR_PLUS;
    }
    else if(tok1.token_type == DIV)
    {
        match(DIV);
        return OPERATOR_DIV;
    }
    else if(tok1.token_type == MULT)
    {
        match(MULT);
        return OPERATOR_MULT;
    }
    else if(tok1.token_type == MINUS)
    {
        match(MINUS);
        return OPERATOR_MINUS;
    }
    else 
    {
        syntax_error();
    }
    return OPERATOR_DIV;
}

void parse_cond(InstructionNode* c)
{
    c->cjmp_inst.operand1_index = parse_prim();
    c->cjmp_inst.condition_op = parse_reloperator();
    c->cjmp_inst.operand1_index = parse_prim();
}

struct InstructionNode* parse_default_case(struct InstructionNode* def_inst)
{
    struct InstructionNode* def_inst1 = NULL;
    struct InstructionNode* def_inst2 = NULL;
    match(DEFAULT); match(COLON);
    def_inst1 = parse_body();
    def_inst2 = def_inst1;
    while(def_inst2->next != NULL) def_inst2 = def_inst2->next;
    def_inst2->next = def_inst;
    return def_inst1;
}

struct InstructionNode* parse_case_list(struct InstructionNode* noop, string currv)
{
    struct InstructionNode* jmp_node = new InstructionNode();
    jmp_node->type = JMP;
    jmp_node->jmp_inst.target = noop;
    jmp_node->next = noop;
    struct InstructionNode* case_inst1 = parse_case(jmp_node, currv);
    struct InstructionNode* case_inst2 = NULL;
    Token tok1  = lexer.peek(1);
    if(tok1.token_type == CASE)
    {
        case_inst2 = parse_case_list(noop, currv);
        case_inst1->next = case_inst2;
        return case_inst1;
    }
    else if(tok1.token_type == RBRACE || tok1.token_type == DEFAULT) return case_inst1; // check this logic
    else syntax_error();
}

struct InstructionNode* parse_case(struct InstructionNode* jmp, string currv)
{
    int one = next_available;
    struct InstructionNode* case_inst = new InstructionNode();
    struct InstructionNode* temp;
    case_inst->next = NULL;
    case_inst->type = CJMP;
    match(CASE);
    Token num = match(NUM);
    struct variable_entry* var_e = new struct variable_entry;
    var_e->var_name = num.lexeme;
    var_e->loc = one;
    var_table.push_back(var_e);

    mem[next_available] = stoi(num.lexeme);
    next_available ++;
    case_inst->cjmp_inst.operand1_index = index_master(currv);
    case_inst->cjmp_inst.operand2_index = index_master(num.lexeme);
    case_inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    match(COLON);
    case_inst->cjmp_inst.target = parse_body();
    temp = case_inst->cjmp_inst.target;
    if(temp == NULL) case_inst->cjmp_inst.target = jmp; 
    else
    {
        while(temp->next != NULL) temp = temp->next;
        temp->next = jmp;
    }
    return case_inst;
}

struct InstructionNode* parse_if()
{
    match(IF);
    struct InstructionNode* if_inst1 = new InstructionNode;
    struct InstructionNode* if_inst = new InstructionNode;
    struct InstructionNode* if_inst_temp;
    if_inst->type = CJMP;
    if_inst1->next = NULL;
    if_inst1->type = NOOP;
    parse_cond(if_inst);

    //parse body
    if_inst->next = parse_body();
    // set old-> next to tmep
    if_inst_temp = if_inst->next;
    while(if_inst_temp->next != NULL)  if_inst_temp = if_inst_temp->next;
    if_inst_temp->next = if_inst1;
    if_inst->cjmp_inst.target = if_inst1;

    // set flag
    check_if = 1;
    
    return if_inst;
}

void parse_expression(struct InstructionNode* exprssn)
{
    exprssn->assign_inst.operand1_index = parse_prim();
    exprssn->assign_inst.op = get_operator_type();
    exprssn->assign_inst.operand2_index = parse_prim();
}

struct InstructionNode* parse_switch()
{
    Token tok1;
    struct InstructionNode* switch_inst = NULL;
    struct InstructionNode* jmp_node = new InstructionNode;
    struct InstructionNode* switch_inst1 = NULL;
    struct InstructionNode* no_op = new InstructionNode;
    struct InstructionNode* temp = NULL;
    jmp_node->type = JMP;
    jmp_node->jmp_inst.target = no_op;
    jmp_node->next = no_op;
    no_op->type = NOOP;
    no_op->next = NULL;
    match(SWITCH);
    tok1 = match(ID);
    match(LBRACE);
    switch_inst = parse_case_list(no_op, tok1.lexeme);
    Token tok2 = lexer.peek(1);
    if(tok2.token_type == DEFAULT)
    {
        switch_inst1 = parse_default_case(jmp_node);
        temp = switch_inst;
        while(temp->next != NULL) temp = temp->next;
        temp->next = switch_inst1;
        match(RBRACE);
        check_switch = 1;
        return switch_inst;
    }
    else if(tok2.token_type == RBRACE)
    {
        temp = switch_inst;
        while(temp->next != NULL) temp = temp->next;
        temp->next = jmp_node;
        match(RBRACE);
        check_switch = 1;
        return switch_inst;
    }
    else syntax_error();
    return NULL;
}

struct InstructionNode* parse_while()
{
    struct InstructionNode* whl_inst1;
    struct InstructionNode* whl_inst = new InstructionNode;
    struct InstructionNode* whl_inst2 = new InstructionNode;
    struct InstructionNode* jmp_inst = new InstructionNode;
    match(WHILE);
    whl_inst2->type = NOOP;
    whl_inst->type = CJMP;
    whl_inst2->next = NULL;
    jmp_inst->type = JMP;
    jmp_inst->jmp_inst.target = whl_inst;
    jmp_inst->next = whl_inst2;
    parse_cond(whl_inst);
    whl_inst->next = parse_body();
    if(whl_inst->next == NULL)
    {
        // make while instruction ->next = jmp
        whl_inst->next = jmp_inst;
    }
    else
    {
        // while1 = oledwhile->next
        whl_inst1 = whl_inst->next;
        while(whl_inst1->next != NULL)
        {
            whl_inst1 = whl_inst1->next;
        }
        whl_inst1->next = jmp_inst;
    }
    check_while = 1;
    // set target to 2
    whl_inst->cjmp_inst.target = whl_inst2;
    // set while flag to 1
    return whl_inst;
}



struct InstructionNode* parse_for()
{
    // token t1 = lex.GetToken();
    // t1.token_type == FOR
    match(FOR);
    struct InstructionNode* jmp_inst = new InstructionNode;
    struct InstructionNode* for_inst = new InstructionNode;
    struct InstructionNode* for_inst2 = new InstructionNode;
    struct InstructionNode* temp_while;
    struct InstructionNode* assignment;
    struct InstructionNode* while_stmt = new InstructionNode;
    for_inst2->type = NOOP;
    for_inst2->next = NULL;
    while_stmt->type = CJMP;
    jmp_inst->type = JMP;
    jmp_inst->jmp_inst.target = while_stmt;
    jmp_inst->next = for_inst2;
    match(LPAREN);
    for_inst = parse_assign_stmt();
    parse_cond(while_stmt);
    match(SEMICOLON);
    assignment = parse_assign_stmt();
    assignment->next = jmp_inst;
    match(RPAREN);
    while_stmt->next = parse_body();
    while_stmt->cjmp_inst.target = for_inst2;
    for_inst->next = while_stmt;
    temp_while = while_stmt->next;
    while(temp_while->next != NULL)
    {
        temp_while = temp_while->next;
    }
    temp_while->next = assignment;
    check_for = 1;
    return for_inst;
}

struct InstructionNode* parse_input()
{
    struct InstructionNode* inp_inst = new InstructionNode;
    match(INPUT);
    Token tok1;
    tok1 = match(ID);
    inp_inst->type = IN;
    inp_inst->next = NULL;
    inp_inst->input_inst.var_index = index_master(tok1.lexeme);
    match(SEMICOLON);
    return inp_inst;
}

struct InstructionNode* parse_output()
{
    struct InstructionNode* out_inst = new InstructionNode;
    match(OUTPUT);
    Token tok1;
    tok1 = match(ID);
    out_inst->type = OUT;
    out_inst->next = NULL;
    out_inst->output_inst.var_index = index_master(tok1.lexeme);
    match(SEMICOLON);
    return out_inst;
}

void parse_input_nums()
{
    Token tok1;
    tok1 = lexer.peek(1);
    if(tok1.token_type == NUM)
    {
        match(NUM);
        int a = stoi(tok1.lexeme);
        inputs.push_back(a);
        parse_input_nums();
    }
    else if(tok1.token_type == END_OF_FILE)
    {
        return;
    }
    else
    {
        syntax_error();
    }
}