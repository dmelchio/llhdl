%include {
	#include <assert.h>
	#include <string.h>
	#include <stdlib.h>
	#include "verilog.h"
}

%start_symbol module
%extra_argument {struct verilog_module *outm}

%token_type {void *}
%token_destructor { free($$); }

%syntax_error {
	fprintf(stderr, "Syntax error in Verilog input\n");
	exit(EXIT_FAILURE);
}

%type constant {struct verilog_constant *}
%destructor constant { verilog_free_constant($$); }

constant(C) ::= TOK_VCON(V). {
	C = verilog_new_constant_str(V);
	free(V);
}

constant(C) ::= TOK_PINT(V). {
	C = verilog_new_constant_str(V);
	free(V);
}

%type width {int}

width(W) ::= . {
	W = 1;
}

width(W) ::= TOK_LBRACKET TOK_PINT(H) TOK_COLON TOK_PINT(L) TOK_RBRACKET . {
	int h, l;
	h = atoi(H);
	l = atoi(L);
	free(H);
	free(L);
	if(l != 0) {
		fprintf(stderr, "Right half of width specifier must be 0\n");
		exit(EXIT_FAILURE);
	}
	W = h - l + 1;
}

%include {
	struct slice {
		int start;
		int end;
	};
}

%type slice {struct slice}

slice(C) ::= TOK_LBRACKET TOK_PINT(H) TOK_COLON TOK_PINT(L) TOK_RBRACKET . {
	C.start = atoi(L);
	C.end = atoi(H);
	free(H);
	free(L);
}

slice(C) ::= TOK_LBRACKET TOK_PINT(B) TOK_RBRACKET . {
	C.start = C.end = atoi(B);
	free(B);
}

%type sgn {int}

sgn(N) ::= . {
	N = 0;
}

sgn(N) ::= TOK_SIGNED . {
	N = 1;
}

%type newsignal {struct verilog_signal *}
%destructor newsignal { verilog_free_signal(outm, $$); }

newsignal(S) ::= TOK_REGWIRE sgn(N) width(W) TOK_ID(I). {
	S = verilog_new_update_signal(outm, VERILOG_SIGNAL_REGWIRE, I, W, N);
	free(I);
}

newsignal(S) ::= TOK_OUTPUT sgn(N) width(W) TOK_ID(I). {
	S = verilog_new_update_signal(outm, VERILOG_SIGNAL_OUTPUT, I, W, N);
	free(I);
}

newsignal(S) ::= TOK_INPUT sgn(N) width(W) TOK_ID(I). {
	S = verilog_new_update_signal(outm, VERILOG_SIGNAL_INPUT, I, W, N);
	free(I);
}

newsignal(S) ::= TOK_OUTPUT TOK_REGWIRE sgn(N) width(W) TOK_ID(I). {
	S = verilog_new_update_signal(outm, VERILOG_SIGNAL_OUTPUT, I, W, N);
	free(I);
}

newsignal(S) ::= TOK_INPUT TOK_REGWIRE sgn(N) width(W) TOK_ID(I). {
	S = verilog_new_update_signal(outm, VERILOG_SIGNAL_INPUT, I, W, N);
	free(I);
}

%type signal {struct verilog_signal *}

signal(S) ::= TOK_ID(I). {
	S = verilog_find_signal(outm, I);
	if(S == NULL) {
		fprintf(stderr, "Signal not found: %s\n", I);
		exit(EXIT_FAILURE);
	}
	free(I);
}

%type node {struct verilog_node *}
%destructor node { verilog_free_node($$); }

node(N) ::= constant(C). {
	N = verilog_new_op_node(VERILOG_NODE_CONSTANT);
	N->branches[0] = C;
}

node(N) ::= signal(S). {
	N = verilog_new_op_node(VERILOG_NODE_SIGNAL);
	N->branches[0] = S;
}

node(N) ::= signal(S) slice(C). {
	struct verilog_node *sig;
	sig = verilog_new_op_node(VERILOG_NODE_SIGNAL);
	sig->branches[0] = S;
	N = verilog_new_op_node(VERILOG_NODE_SLICE);
	N->branches[0] = sig;
	N->branches[1] = (void *)C.start;
	N->branches[2] = (void *)C.end;
}

%type lnode {struct verilog_node *}
%destructor lnode { verilog_free_node($$); }

lnode(N) ::= node(A) TOK_COMMA node(B). {
	N = verilog_new_op_node(VERILOG_NODE_CAT);
	N->branches[0] = A;
	N->branches[1] = B;
}

lnode(N) ::= lnode(A) TOK_COMMA node(B). {
	struct verilog_node *nn;
	nn = verilog_new_op_node(VERILOG_NODE_CAT);
	nn->branches[0] = A->branches[1];
	nn->branches[1] = B;
	A->branches[1] = nn;
	N = A;
}

node(N) ::= TOK_LCBRACKET node(A) TOK_RCBRACKET. {
	N = A;
}

node(N) ::= TOK_LCBRACKET lnode(A) TOK_RCBRACKET. {
	N = A;
}

%left TOK_QUESTION TOK_COLON.
%left TOK_OR.
%left TOK_XOR.
%left TOK_AND.
%left TOK_EQL TOK_NEQ.
%left TOK_PLUS TOK_MINUS.
%left TOK_STAR.
%left TOK_TILDE.

node(N) ::= node(A) TOK_EQL node(B). {
	N = verilog_new_op_node(VERILOG_NODE_EQL);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= node(A) TOK_NEQ node(B). {
	N = verilog_new_op_node(VERILOG_NODE_NEQ);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= node(A) TOK_OR node(B). {
	N = verilog_new_op_node(VERILOG_NODE_OR);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= node(A) TOK_AND node(B). {
	N = verilog_new_op_node(VERILOG_NODE_AND);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= TOK_TILDE node(A). {
	N = verilog_new_op_node(VERILOG_NODE_TILDE);
	N->branches[0] = A;
}

node(N) ::= node(A) TOK_XOR node(B). {
	N = verilog_new_op_node(VERILOG_NODE_XOR);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= node(A) TOK_PLUS node(B). {
	N = verilog_new_op_node(VERILOG_NODE_ADD);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= node(A) TOK_MINUS node(B). {
	N = verilog_new_op_node(VERILOG_NODE_SUB);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= node(A) TOK_STAR node(B). {
	N = verilog_new_op_node(VERILOG_NODE_MUL);
	N->branches[0] = A;
	N->branches[1] = B;
}

node(N) ::= node(A) TOK_QUESTION node(B) TOK_COLON node(C). {
	N = verilog_new_op_node(VERILOG_NODE_ALT);
	N->branches[0] = A;
	N->branches[1] = B;
	N->branches[2] = C;
}

node(N) ::= TOK_LPAREN node(A) TOK_RPAREN. {
	N = A;
}

%type singleassignment {struct verilog_statement *}
%destructor singleassignment { verilog_free_statement_list($$); }

singleassignment(A) ::= signal(D) TOK_BASSIGN node(S). {
	A = verilog_new_assignment(D, 1, S);
}

singleassignment(A) ::= signal(D) TOK_NBASSIGN node(S). {
	A = verilog_new_assignment(D, 0, S);
}

/* See http://marvin.cs.uidaho.edu/~heckendo/CS445/danglingElse.html */

%type statement {struct verilog_statement *}
%destructor statement { verilog_free_statement_list($$); }
%type matched {struct verilog_statement *}
%destructor matched { verilog_free_statement_list($$); }
%type unmatched {struct verilog_statement *}
%destructor unmatched { verilog_free_statement_list($$); }

matched(C) ::= TOK_IF TOK_LPAREN node(E) TOK_RPAREN matched(P) TOK_ELSE matched(N). {
	C = verilog_new_condition(E, N, P);
}

unmatched(C) ::= TOK_IF TOK_LPAREN node(E) TOK_RPAREN matched(S). {
	C = verilog_new_condition(E, NULL, S);
}

unmatched(C) ::= TOK_IF TOK_LPAREN node(E) TOK_RPAREN unmatched(S). {
	C = verilog_new_condition(E, NULL, S);
}

unmatched(C) ::= TOK_IF TOK_LPAREN node(E) TOK_RPAREN matched(P) TOK_ELSE unmatched(N). {
	C = verilog_new_condition(E, N, P);
}

matched(S) ::= singleassignment(A) TOK_SEMICOLON. { S = A; }

statement(S) ::= matched(M). { S = M; }
statement(S) ::= unmatched(U). { S = U; }

%type statementlist {struct verilog_statement *}
%destructor statementlist { verilog_free_statement_list($$); }

statementlist(L) ::= statementlist(B) statement(S). {
	if(B != NULL) {
		struct verilog_statement *s;
		s = B;
		while(s->next != NULL)
			s = s->next;
		s->next = S;
		L = B;
	} else
		L = S;
}
statementlist(L) ::= TOK_SEMICOLON. { L = NULL; }
statementlist(L) ::= . { L = NULL; }

matched(S) ::= TOK_BEGIN statementlist(L) TOK_END. { S = L; }

%type process {struct verilog_process *}
%destructor process { verilog_free_process(outm, $$); }

process(P) ::= TOK_ASSIGN singleassignment(A) TOK_SEMICOLON. {
	P = verilog_new_process(outm, NULL, A);
}

process(P) ::= TOK_ALWAYS TOK_AT TOK_LPAREN TOK_STAR TOK_RPAREN statement(S). {
	P = verilog_new_process(outm, NULL, S);
}

process(P) ::= TOK_ALWAYS TOK_AT TOK_LPAREN TOK_POSEDGE signal(C) TOK_RPAREN statement(S). {
	P = verilog_new_process(outm, C, S);
}

/* FIXME: this accepts wire/reg declarations in the module header */

/* FIXME: add destructors for io and body */

io(I) ::= io TOK_COMMA newsignal(S). { I = S; }
io(I) ::= newsignal(S). { I = S; }
io ::= .

body(B) ::= body newsignal(N) TOK_SEMICOLON. { B = N; }
body(B) ::= body process(P). { B = P; }
body ::= .

module ::= TOK_MODULE TOK_ID(N) TOK_LPAREN io TOK_RPAREN TOK_SEMICOLON body TOK_ENDMODULE. {
	verilog_set_module_name(outm, N);
	free(N);
}
