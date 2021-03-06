#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

#include <netlist/net.h>

struct netlist_instance *netlist_instantiate(unsigned int uid, struct netlist_primitive *p)
{
	struct netlist_instance *new;
	int i;

	new = alloc_type(struct netlist_instance);
	
	new->uid = uid;
	new->dont_touch = 0;
	new->p = p;

	if(p->attribute_count > 0) {
		new->attributes = alloc_size(p->attribute_count*sizeof(void *));
		for(i=0;i<p->attribute_count;i++)
			new->attributes[i] = stralloc(p->default_attributes[i]);
	} else
		new->attributes = NULL;

	new->next = NULL;

	return new;
}

void netlist_free_instance(struct netlist_instance *inst)
{
	int i;

	for(i=0;i<inst->p->attribute_count;i++)
		free(inst->attributes[i]);
	free(inst->attributes);
	
	free(inst);
}

static int find_attribute(struct netlist_instance *inst, const char *attr)
{
	int i;

	for(i=0;i<inst->p->attribute_count;i++)
		if(strcmp(inst->p->attribute_names[i], attr) == 0) return i;
	return -1;
}

void netlist_set_attribute(struct netlist_instance *inst, const char *attr, const char *value)
{
	int a;

	a = find_attribute(inst, attr);
	assert(a != -1);
	
	free(inst->attributes[a]);
	if(value == NULL)
		inst->attributes[a] = stralloc(inst->p->default_attributes[a]);
	else
		inst->attributes[a] = stralloc(value);
}

struct netlist_net *netlist_create_net(unsigned int uid)
{
	struct netlist_net *net;

	net = alloc_type(struct netlist_net);
	net->uid = uid;
	net->head = NULL;
	net->joined = NULL;
	net->next = NULL;
	return net;
}

struct netlist_net *netlist_resolve_joined(struct netlist_net *net)
{
	while(net->joined != NULL)
		net = net->joined;
	return net;
}

void netlist_join(struct netlist_net *resulting, struct netlist_net *tomerge)
{
	struct netlist_branch *last;
	
	if((resulting == NULL) || (tomerge == NULL)) return;
	resulting = netlist_resolve_joined(resulting);
	tomerge = netlist_resolve_joined(tomerge);
	if(resulting == tomerge) return;
	
	if(resulting->head != NULL) {
		last = resulting->head;
		while(last->next != NULL)
			last = last->next;
		last->next = tomerge->head;
	} else
		resulting->head = tomerge->head;
	
	tomerge->head = NULL;
	tomerge->joined = resulting;
}

void netlist_add_branch(struct netlist_net *net, struct netlist_instance *inst, int output, int pin_index)
{
	struct netlist_branch *branch;

	net = netlist_resolve_joined(net);

	if(output)
		assert(pin_index < inst->p->outputs);
	else
		assert(pin_index < inst->p->inputs);
	
	branch = alloc_type(struct netlist_branch);
	branch->inst = inst;
	branch->output = output;
	branch->pin_index = pin_index;
	branch->next = net->head;
	net->head = branch;
}

void netlist_disconnect_instance(struct netlist_net *net, struct netlist_instance *inst)
{
	struct netlist_branch *b1, *b2, *prev;
	
	if(net->head == NULL) return;
	prev = NULL;
	b1 = net->head;
	while(b1 != NULL) {
		if(b1->inst == inst) {
			b2 = b1->next;
			free(b1);
			if(prev != NULL)
				prev->next = b2;
			else
				net->head = b2;
			b1 = b2;
		} else {
			prev = b1;
			b1 = b1->next;
		}
	}
}

void netlist_free_net(struct netlist_net *net)
{
	struct netlist_branch *b1, *b2;

	b1 = net->head;
	while(b1 != NULL) {
		b2 = b1->next;
		free(b1);
		b1 = b2;
	}
	free(net);
}
