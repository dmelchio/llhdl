#ifndef __LLHDL_STRUCTURE_H
#define __LLHDL_STRUCTURE_H

#include <gmp.h>

struct llhdl_node;
struct llhdl_module;

struct llhdl_node_constant {
	int sign;
	int vectorsize;
	mpz_t value;
};

enum {
	LLHDL_SIGNAL_INTERNAL,
	LLHDL_SIGNAL_PORT_OUT,
	LLHDL_SIGNAL_PORT_IN
};

struct llhdl_node_signal {
	int type;
	int sign;
	int vectorsize;
	struct llhdl_node *source;
	struct llhdl_node *next;
	char name[];
};

enum {
	LLHDL_LOGIC_NOT,
	LLHDL_LOGIC_AND,
	LLHDL_LOGIC_OR,
	LLHDL_LOGIC_XOR
};

struct llhdl_node_logic {
	int op;
	struct llhdl_node *operands[];
};

struct llhdl_node_mux {
	int nsources;
	struct llhdl_node *select;
	struct llhdl_node *sources[];
};

struct llhdl_node_fd {
	struct llhdl_node *clock;
	struct llhdl_node *data;
};

struct llhdl_slice {
	struct llhdl_node *source;
	int start;
	int end;
};

struct llhdl_node_vect {
	int sign;
	int nslices;
	struct llhdl_slice slices[];
};

enum {
	LLHDL_ARITH_ADD,
	LLHDL_ARITH_SUB,
	LLHDL_ARITH_MUL
};

struct llhdl_node_arith {
	int op;
	struct llhdl_node *a;
	struct llhdl_node *b;
};

enum {
	LLHDL_NODE_CONSTANT,
	LLHDL_NODE_SIGNAL,
	LLHDL_NODE_LOGIC,
	LLHDL_NODE_MUX,
	LLHDL_NODE_FD,
	LLHDL_NODE_VECT,
	LLHDL_NODE_ARITH
};

struct llhdl_node {
	int type;
	union {
		struct llhdl_node_constant constant;
		struct llhdl_node_signal signal;
		struct llhdl_node_logic logic;
		struct llhdl_node_mux mux;
		struct llhdl_node_fd fd;
		struct llhdl_node_vect vect;
		struct llhdl_node_arith arith;
	} p;
};

struct llhdl_module {
	char *name;
	struct llhdl_node *head;
};

int llhdl_get_logic_arity(int op);

struct llhdl_module *llhdl_new_module();
void llhdl_free_module(struct llhdl_module *m);
void llhdl_set_module_name(struct llhdl_module *m, const char *name);

struct llhdl_node *llhdl_create_constant(mpz_t value, int sign, int vectorsize);
struct llhdl_node *llhdl_create_signal(struct llhdl_module *m, int type, const char *name, int sign, int vectorsize);
struct llhdl_node *llhdl_create_logic(int op, struct llhdl_node **operands);
struct llhdl_node *llhdl_create_mux(int nsources, struct llhdl_node *select, struct llhdl_node **sources);
struct llhdl_node *llhdl_create_fd(struct llhdl_node *clock, struct llhdl_node *data);
struct llhdl_node *llhdl_create_vect(int sign, int nslices, struct llhdl_slice *slices);
struct llhdl_node *llhdl_create_arith(int op, struct llhdl_node *a, struct llhdl_node *b);
void llhdl_free_node(struct llhdl_node *n); /* < does not free signals */
void llhdl_free_signal(struct llhdl_node *n);

struct llhdl_node *llhdl_find_signal(struct llhdl_module *m, const char *name);

#endif /* __LLHDL_STRUCTURE_H */
