#include <stdio.h>

#include <netlist/net.h>
#include <netlist/manager.h>
#include <netlist/edif.h>
#include <netlist/io.h>

static char *attribute_names[] = {"MYATTR"};
static char *attrs[] = {"42"};
static char *input_names[] = {"I"};
static char *output_names[] = {"O"};

static struct netlist_primitive test = {
	.type = NETLIST_PRIMITIVE_INTERNAL,
	.name = "TEST",
	.attribute_count = 1,
	.attribute_names = attribute_names,
	.default_attributes = attrs,
	.inputs = 1,
	.input_names = input_names,
	.outputs = 1,
	.output_names = output_names
};

static struct edif_param edif_param = {
	.flavor = EDIF_FLAVOR_XILINX,
	.design_name = "ledblinker",
	.cell_library = "UNISIMS",
	.part = "xc6slx45-fgg484-2",
	.manufacturer = "Xilinx"
};

int main(int argc, char *argv[])
{
	struct netlist_primitive *my_in, *my_out;
	struct netlist_manager *m;
	struct netlist_instance *i1, *i2;
	struct netlist_instance *inp, *outp;

	my_in = netlist_create_io_primitive(NETLIST_PRIMITIVE_PORT_IN, "toto_i");
	my_out = netlist_create_io_primitive(NETLIST_PRIMITIVE_PORT_OUT, "tata_o");
	
	m = netlist_m_new();

	i1 = netlist_m_instantiate(m, &test);
	i2 = netlist_m_instantiate(m, &test);
	netlist_m_connect(m, i1, 0, i2, 0);
	inp = netlist_m_instantiate(m, my_in);
	outp = netlist_m_instantiate(m, my_out);
	netlist_m_connect(m, inp, 0, i1, 0);
	netlist_m_connect(m, i2, 0, outp, 0);
	
	netlist_m_edif_fd(m, stdout, &edif_param);
	
	netlist_m_free(m);

	netlist_free_io_primitive(my_in);
	netlist_free_io_primitive(my_out);

	return 0;
}
