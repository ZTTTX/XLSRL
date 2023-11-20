module xls_test(
  input wire clk,
  input wire [511:0] message,
  output wire [29:0] out
);
  // ===== Pipe stage 0:
  wire [29:0] p0_literal_51817_comb;
  assign p0_literal_51817_comb = 30'h0eb1_0b89;

  // Registers for pipe stage 0:
  reg [29:0] p1_literal_51817;
  always_ff @ (posedge clk) begin
    p1_literal_51817 <= p0_literal_51817_comb;
  end
  assign out = p1_literal_51817;
endmodule
