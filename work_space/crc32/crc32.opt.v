module xls_test(
  input wire clk,
  input wire [7:0] message,
  output wire [31:0] out
);
  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [7:0] p0_message;
  always_ff @ (posedge clk) begin
    p0_message <= message;
  end

  // ===== Pipe stage 1:
  wire [7:0] p1_not_481_comb;
  wire [31:0] p1_xor_491_comb;
  wire p1_bit_slice_492_comb;
  wire [30:0] p1_bit_slice_493_comb;
  assign p1_not_481_comb = ~p0_message;
  assign p1_xor_491_comb = {25'h0ff_ffff, p1_not_481_comb[7:1]} ^ -{31'h0000_0000, p1_not_481_comb[0]} & 32'hedb8_8320;
  assign p1_bit_slice_492_comb = p1_xor_491_comb[0];
  assign p1_bit_slice_493_comb = p1_xor_491_comb[31:1];

  // Registers for pipe stage 1:
  reg p1_bit_slice_492;
  reg [30:0] p1_bit_slice_493;
  always_ff @ (posedge clk) begin
    p1_bit_slice_492 <= p1_bit_slice_492_comb;
    p1_bit_slice_493 <= p1_bit_slice_493_comb;
  end

  // ===== Pipe stage 2:
  wire [31:0] p2_xor_505_comb;
  wire p2_bit_slice_506_comb;
  wire [30:0] p2_bit_slice_507_comb;
  assign p2_xor_505_comb = {1'h0, p1_bit_slice_493} ^ -{31'h0000_0000, p1_bit_slice_492} & 32'hedb8_8320;
  assign p2_bit_slice_506_comb = p2_xor_505_comb[0];
  assign p2_bit_slice_507_comb = p2_xor_505_comb[31:1];

  // Registers for pipe stage 2:
  reg p2_bit_slice_506;
  reg [30:0] p2_bit_slice_507;
  always_ff @ (posedge clk) begin
    p2_bit_slice_506 <= p2_bit_slice_506_comb;
    p2_bit_slice_507 <= p2_bit_slice_507_comb;
  end

  // ===== Pipe stage 3:
  wire [31:0] p3_xor_519_comb;
  wire p3_bit_slice_520_comb;
  wire [30:0] p3_bit_slice_521_comb;
  assign p3_xor_519_comb = {1'h0, p2_bit_slice_507} ^ -{31'h0000_0000, p2_bit_slice_506} & 32'hedb8_8320;
  assign p3_bit_slice_520_comb = p3_xor_519_comb[0];
  assign p3_bit_slice_521_comb = p3_xor_519_comb[31:1];

  // Registers for pipe stage 3:
  reg p3_bit_slice_520;
  reg [30:0] p3_bit_slice_521;
  always_ff @ (posedge clk) begin
    p3_bit_slice_520 <= p3_bit_slice_520_comb;
    p3_bit_slice_521 <= p3_bit_slice_521_comb;
  end

  // ===== Pipe stage 4:
  wire [31:0] p4_xor_533_comb;
  wire p4_bit_slice_534_comb;
  wire [30:0] p4_bit_slice_535_comb;
  assign p4_xor_533_comb = {1'h0, p3_bit_slice_521} ^ -{31'h0000_0000, p3_bit_slice_520} & 32'hedb8_8320;
  assign p4_bit_slice_534_comb = p4_xor_533_comb[0];
  assign p4_bit_slice_535_comb = p4_xor_533_comb[31:1];

  // Registers for pipe stage 4:
  reg p4_bit_slice_534;
  reg [30:0] p4_bit_slice_535;
  always_ff @ (posedge clk) begin
    p4_bit_slice_534 <= p4_bit_slice_534_comb;
    p4_bit_slice_535 <= p4_bit_slice_535_comb;
  end

  // ===== Pipe stage 5:
  wire [31:0] p5_xor_547_comb;
  wire p5_bit_slice_548_comb;
  wire [30:0] p5_bit_slice_549_comb;
  assign p5_xor_547_comb = {1'h0, p4_bit_slice_535} ^ -{31'h0000_0000, p4_bit_slice_534} & 32'hedb8_8320;
  assign p5_bit_slice_548_comb = p5_xor_547_comb[0];
  assign p5_bit_slice_549_comb = p5_xor_547_comb[31:1];

  // Registers for pipe stage 5:
  reg p5_bit_slice_548;
  reg [30:0] p5_bit_slice_549;
  always_ff @ (posedge clk) begin
    p5_bit_slice_548 <= p5_bit_slice_548_comb;
    p5_bit_slice_549 <= p5_bit_slice_549_comb;
  end

  // ===== Pipe stage 6:
  wire [31:0] p6_xor_561_comb;
  wire p6_bit_slice_562_comb;
  wire [30:0] p6_bit_slice_563_comb;
  assign p6_xor_561_comb = {1'h0, p5_bit_slice_549} ^ -{31'h0000_0000, p5_bit_slice_548} & 32'hedb8_8320;
  assign p6_bit_slice_562_comb = p6_xor_561_comb[0];
  assign p6_bit_slice_563_comb = p6_xor_561_comb[31:1];

  // Registers for pipe stage 6:
  reg p6_bit_slice_562;
  reg [30:0] p6_bit_slice_563;
  always_ff @ (posedge clk) begin
    p6_bit_slice_562 <= p6_bit_slice_562_comb;
    p6_bit_slice_563 <= p6_bit_slice_563_comb;
  end

  // ===== Pipe stage 7:
  wire [31:0] p7_xor_575_comb;
  wire p7_bit_slice_576_comb;
  wire [30:0] p7_bit_slice_577_comb;
  assign p7_xor_575_comb = {1'h0, p6_bit_slice_563} ^ -{31'h0000_0000, p6_bit_slice_562} & 32'hedb8_8320;
  assign p7_bit_slice_576_comb = p7_xor_575_comb[0];
  assign p7_bit_slice_577_comb = p7_xor_575_comb[31:1];

  // Registers for pipe stage 7:
  reg p7_bit_slice_576;
  reg [30:0] p7_bit_slice_577;
  always_ff @ (posedge clk) begin
    p7_bit_slice_576 <= p7_bit_slice_576_comb;
    p7_bit_slice_577 <= p7_bit_slice_577_comb;
  end

  // ===== Pipe stage 8:
  wire [31:0] p8_not_590_comb;
  assign p8_not_590_comb = ~({1'h0, p7_bit_slice_577} ^ -{31'h0000_0000, p7_bit_slice_576} & 32'hedb8_8320);

  // Registers for pipe stage 8:
  reg [31:0] p8_not_590;
  always_ff @ (posedge clk) begin
    p8_not_590 <= p8_not_590_comb;
  end
  assign out = p8_not_590;
endmodule
