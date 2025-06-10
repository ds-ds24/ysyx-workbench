#include "Vtop.h"
#include "verilated.h"
#include <nvboard.h>

static TOP_NAME dut;
void nvboard_bind_all_pins(TOP_NAME* top) ;
//static void single_cycle() {
  //dut.clk = 0; dut.eval();
  //dut.clk = 1; dut.eval();}
signed main(){
        nvboard_bind_all_pins(&dut);
        nvboard_init();
        //VerilatedContext* contextp = new VerilatedContext;
       // Vtop* top=new Vtop(contextp);
       // VerilatedVcdC* tfp = new VerilatedVcdC;
       // contextp->traceEverOn(true);
       // top->trace(tfp,0);
       // tfp->open("wave.vcd");
        while(1){
                nvboard_update();
		dut.eval();
                //single_cycle();
        }
        //tfp->close();
        //return 0;
}


