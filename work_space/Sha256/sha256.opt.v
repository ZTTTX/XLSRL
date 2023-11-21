module xls_test(
  input wire clk,
  input wire [511:0] message,
  output wire [29:0] out
);
  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [511:0] p0_message;
  always_ff @ (posedge clk) begin
    p0_message <= message;
  end

  // ===== Pipe stage 1:
  wire [29:0] p1_add_51670_comb;
  assign p1_add_51670_comb = p0_message[415:386] + 30'h0eb1_0b89;

  // Registers for pipe stage 1:
  reg [29:0] p1_add_51670;
  always_ff @ (posedge clk) begin
    p1_add_51670 <= p1_add_51670_comb;
  end
  assign out = p1_add_51670;
endmodule
