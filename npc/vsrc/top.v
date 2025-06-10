module top(
	input reg [1:0] X0,X1,X2,X3,
	input reg [1:0] Y,
	output reg [1:0] F
	);
	mux41 my_mux41(
		.X0(X0), .X1(X1), .X2(X2), .X3(X3),
		.Y(Y), .F(F)
	);
	endmodule

