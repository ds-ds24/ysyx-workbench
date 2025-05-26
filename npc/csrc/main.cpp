#include <stdio.h>
#include "Vexample.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;

int main(int argc,char** argv){
	Vexample *top =new Vexample;
	Verilated::traceEverOn(true);
	VerilatedVcdC *m_trace = new VerilatedVcdC;
	top->trace(m_trace,10);
	m_trace->open("wavefrom.vcd");
	while(sim_time < MAX_SIM_TIME) {
		top->clk ^= 1;
		top->a = sim_time / 3;
		top->b = sim_time / 2;
		top->eval();
		m_trace->dump(sim_time);
		sim_time++;
	}

	m_trace->close();
	delete top;

	return 0;
}

