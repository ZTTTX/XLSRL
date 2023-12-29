module xls_test(
  input wire clk,
  input wire [31:0] x1,
  input wire [31:0] y1,
  input wire [31:0] z1,
  output wire [31:0] out
);
  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [31:0] p0_x1;
  reg [31:0] p0_y1;
  reg [31:0] p0_z1;
  always_ff @ (posedge clk) begin
    p0_x1 <= x1;
    p0_y1 <= y1;
    p0_z1 <= z1;
  end

  // ===== Pipe stage 1:
  wire [31:0] p1_add_190_comb;
  assign p1_add_190_comb = p0_x1 + p0_y1;

  // Registers for pipe stage 1:
  reg [31:0] p1_z1;
  reg [31:0] p1_add_190;
  always_ff @ (posedge clk) begin
    p1_z1 <= p0_z1;
    p1_add_190 <= p1_add_190_comb;
  end

  // ===== Pipe stage 2:
  wire [31:0] p2_add_195_comb;
  assign p2_add_195_comb = p1_add_190 + p1_z1;

  // Registers for pipe stage 2:
  reg [31:0] p2_add_195;
  always_ff @ (posedge clk) begin
    p2_add_195 <= p2_add_195_comb;
  end
  assign out = p2_add_195;
endmodule