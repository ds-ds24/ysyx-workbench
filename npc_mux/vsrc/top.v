module mux41(
    input [1:0] X0,
    input [1:0] X1,
    input [1:0] X2,
    input [1:0] X3,
    input [1:0] Y,
    output reg [1:0] F

);
	always@(*) begin
		case(Y)
			2'b00: F = X0;
			2'b01: F = X1;
			2'b10: F = X2;
			2'b11: F = X3;
			default: F = 2'b00;
		endcase
	end
endmodule

module top(
	input  [1:0] X0,X1,X2,X3,
	input  [1:0] Y,
	output reg [1:0] F
	);
	mux41 mux41_module(
		.X0(X0), .X1(X1), .X2(X2), .X3(X3),
		.Y(Y), .F(F)
	);
	endmodule

