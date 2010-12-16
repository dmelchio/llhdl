#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <llhdl/structure.h>

struct llhdl_module *llhdl_new_module()
{
	struct llhdl_module *m;

	m = malloc(sizeof(struct llhdl_module));
	assert(m != NULL);
	m->name = NULL;
	m->head = NULL;

	return m;
}

void llhdl_free_module(struct llhdl_module *m)
{
	struct llhdl_node *n1, *n2;
	
	free(m->name);

	n1 = m->head;
	while(n1 != NULL) {
		assert(n1->type == LLHDL_NODE_SIGNAL);
		n2 = n1->p.signal.next;
		llhdl_free_node(n1);
		n1 = n2;
	}
}

void llhdl_set_module_name(struct llhdl_module *m, const char *name)
{
	free(m->name);
	if(name == NULL)
		m->name = NULL;
	else {
		m->name = strdup(name);
		assert(m->name != NULL);
	}
}

static struct llhdl_node *alloc_base_node(int payload_size, int type)
{
	struct llhdl_node *n;

	n = malloc(sizeof(int)+payload_size);
	assert(n != NULL);
	n->type = type;
	return n;
}

struct llhdl_node *llhdl_create_boolean(int value)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_boolean), LLHDL_NODE_BOOLEAN);
	n->p.boolean.value = value;
	return n;
}

struct llhdl_node *llhdl_create_signal(struct llhdl_module *m, int type, int sign, const char *name, int vectorsize)
{
	struct llhdl_node *n;
	int len;

	len = strlen(name);
	n = alloc_base_node(sizeof(struct llhdl_node_signal)+len+1, LLHDL_NODE_SIGNAL);
	n->p.signal.type = type;
	n->p.signal.sign = sign;
	n->p.signal.source = NULL;
	n->p.signal.vectorsize = vectorsize;
	n->p.signal.next = m->head;
	memcpy(n->p.signal.name, name, len);
	m->head = n;
	return n;
}

struct llhdl_node *llhdl_create_mux(struct llhdl_node *sel, struct llhdl_node *negative, struct llhdl_node *positive)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_mux), LLHDL_NODE_MUX);
	n->p.mux.sel = sel;
	n->p.mux.negative = negative;
	n->p.mux.positive = positive;
	return n;
}

struct llhdl_node *llhdl_create_fd(struct llhdl_node *clock, struct llhdl_node *data)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_mux), LLHDL_NODE_FD);
	n->p.fd.clock = clock;
	n->p.fd.data = data;
	return n;
}

struct llhdl_node *llhdl_create_integer(long long int value)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_integer), LLHDL_NODE_INTEGER);
	n->p.integer.value = value;
	return n;
}

struct llhdl_node *llhdl_create_slice(struct llhdl_node *source, int start, int end)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_slice), LLHDL_NODE_SLICE);
	n->p.slice.source = source;
	n->p.slice.start = start;
	n->p.slice.end = end;
	return n;
}

struct llhdl_node *llhdl_create_cat(struct llhdl_node *msb, struct llhdl_node *lsb)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_cat), LLHDL_NODE_CAT);
	n->p.cat.msb = msb;
	n->p.cat.lsb = lsb;
	return n;
}

struct llhdl_node *llhdl_create_sign(struct llhdl_node *source, int sign)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_sign), LLHDL_NODE_SIGN);
	n->p.sign.source = source;
	n->p.sign.sign = sign;
	return n;
}

struct llhdl_node *llhdl_create_arith(int op, struct llhdl_node *a, struct llhdl_node *b)
{
	struct llhdl_node *n;

	n = alloc_base_node(sizeof(struct llhdl_node_cat), LLHDL_NODE_ARITH);
	n->p.arith.a = a;
	n->p.arith.b = b;
	return n;
}

void llhdl_free_node(struct llhdl_node *n)
{
	if(n == NULL)
		return;

	switch(n->type) {
		case LLHDL_NODE_BOOLEAN:
			break;
		case LLHDL_NODE_SIGNAL:
			llhdl_free_node(n->p.signal.source);
			break;
		case LLHDL_NODE_MUX:
			llhdl_free_node(n->p.mux.sel);
			llhdl_free_node(n->p.mux.negative);
			llhdl_free_node(n->p.mux.positive);
			break;
		case LLHDL_NODE_FD:
			llhdl_free_node(n->p.fd.clock);
			llhdl_free_node(n->p.fd.data);
			break;
		case LLHDL_NODE_INTEGER:
			break;
		case LLHDL_NODE_SLICE:
			llhdl_free_node(n->p.slice.source);
			break;
		case LLHDL_NODE_CAT:
			llhdl_free_node(n->p.cat.msb);
			llhdl_free_node(n->p.cat.lsb);
			break;
		case LLHDL_NODE_SIGN:
			llhdl_free_node(n->p.sign.source);
			break;
		case LLHDL_NODE_ARITH:
			llhdl_free_node(n->p.arith.a);
			llhdl_free_node(n->p.arith.b);
			break;
		default:
			assert(0);
			break;
	}
	free(n);
}