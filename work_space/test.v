module xls_test_unroll(
  input wire clk,
  input wire [31:0] x,
  output wire [31:0] out
);
  // lint_off MULTIPLY
  function automatic [31:0] umul32b_32b_x_2b (input reg [31:0] lhs, input reg [1:0] rhs);
    begin
      umul32b_32b_x_2b = lhs * rhs;
    end
  endfunction
  // lint_on MULTIPLY
  // lint_off MULTIPLY
  function automatic [31:0] umul32b_32b_x_3b (input reg [31:0] lhs, input reg [2:0] rhs);
    begin
      umul32b_32b_x_3b = lhs * rhs;
    end
  endfunction
  // lint_on MULTIPLY
  // lint_off MULTIPLY
  function automatic [31:0] umul32b_32b_x_4b (input reg [31:0] lhs, input reg [3:0] rhs);
    begin
      umul32b_32b_x_4b = lhs * rhs;
    end
  endfunction
  // lint_on MULTIPLY
  // lint_off MULTIPLY
  function automatic [31:0] umul32b_32b_x_5b (input reg [31:0] lhs, input reg [4:0] rhs);
    begin
      umul32b_32b_x_5b = lhs * rhs;
    end
  endfunction
  // lint_on MULTIPLY

  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [31:0] p0_x;
  always_ff @ (posedge clk) begin
    p0_x <= x;
  end

  // ===== Pipe stage 1:

  // Registers for pipe stage 1:
  reg [31:0] p1_x;
  always_ff @ (posedge clk) begin
    p1_x <= p0_x;
  end

  // ===== Pipe stage 2:
  wire [31:0] p2_umul_1689_comb;
  wire [31:0] p2_umul_1690_comb;
  wire [31:0] p2_umul_1691_comb;
  wire [31:0] p2_umul_1692_comb;
  wire [31:0] p2_umul_1693_comb;
  wire [31:0] p2_umul_1694_comb;
  wire [31:0] p2_umul_1695_comb;
  wire [31:0] p2_umul_1696_comb;
  wire [31:0] p2_umul_1697_comb;
  wire [31:0] p2_umul_1698_comb;
  wire [31:0] p2_umul_1699_comb;
  wire [29:0] p2_bit_slice_1700_comb;
  wire [30:0] p2_bit_slice_1701_comb;
  wire [30:0] p2_bit_slice_1702_comb;
  wire [28:0] p2_bit_slice_1703_comb;
  wire [30:0] p2_bit_slice_1704_comb;
  wire [30:0] p2_bit_slice_1705_comb;
  wire [29:0] p2_bit_slice_1706_comb;
  wire [29:0] p2_bit_slice_1707_comb;
  wire [30:0] p2_bit_slice_1708_comb;
  wire [30:0] p2_bit_slice_1709_comb;
  wire [27:0] p2_bit_slice_1710_comb;
  wire [1:0] p2_bit_slice_1711_comb;
  wire p2_bit_slice_1712_comb;
  wire [2:0] p2_bit_slice_1713_comb;
  wire p2_bit_slice_1714_comb;
  wire [1:0] p2_bit_slice_1715_comb;
  wire p2_bit_slice_1716_comb;
  wire [3:0] p2_bit_slice_1717_comb;
  assign p2_umul_1689_comb = umul32b_32b_x_2b(p1_x, 2'h3);
  assign p2_umul_1690_comb = umul32b_32b_x_3b(p1_x, 3'h5);
  assign p2_umul_1691_comb = umul32b_32b_x_3b(p1_x, 3'h6);
  assign p2_umul_1692_comb = umul32b_32b_x_3b(p1_x, 3'h7);
  assign p2_umul_1693_comb = umul32b_32b_x_4b(p1_x, 4'h9);
  assign p2_umul_1694_comb = umul32b_32b_x_4b(p1_x, 4'ha);
  assign p2_umul_1695_comb = umul32b_32b_x_4b(p1_x, 4'hb);
  assign p2_umul_1696_comb = umul32b_32b_x_4b(p1_x, 4'hc);
  assign p2_umul_1697_comb = umul32b_32b_x_4b(p1_x, 4'hd);
  assign p2_umul_1698_comb = umul32b_32b_x_4b(p1_x, 4'he);
  assign p2_umul_1699_comb = umul32b_32b_x_4b(p1_x, 4'hf);
  assign p2_bit_slice_1700_comb = p2_umul_1689_comb[31:2];
  assign p2_bit_slice_1701_comb = p2_umul_1690_comb[31:1];
  assign p2_bit_slice_1702_comb = p2_umul_1691_comb[31:1];
  assign p2_bit_slice_1703_comb = p2_umul_1692_comb[31:3];
  assign p2_bit_slice_1704_comb = p2_umul_1693_comb[31:1];
  assign p2_bit_slice_1705_comb = p2_umul_1694_comb[31:1];
  assign p2_bit_slice_1706_comb = p2_umul_1695_comb[31:2];
  assign p2_bit_slice_1707_comb = p2_umul_1696_comb[31:2];
  assign p2_bit_slice_1708_comb = p2_umul_1697_comb[31:1];
  assign p2_bit_slice_1709_comb = p2_umul_1698_comb[31:1];
  assign p2_bit_slice_1710_comb = p2_umul_1699_comb[31:4];
  assign p2_bit_slice_1711_comb = p2_umul_1689_comb[1:0];
  assign p2_bit_slice_1712_comb = p2_umul_1690_comb[0];
  assign p2_bit_slice_1713_comb = p2_umul_1692_comb[2:0];
  assign p2_bit_slice_1714_comb = p2_umul_1693_comb[0];
  assign p2_bit_slice_1715_comb = p2_umul_1695_comb[1:0];
  assign p2_bit_slice_1716_comb = p2_umul_1697_comb[0];
  assign p2_bit_slice_1717_comb = p2_umul_1699_comb[3:0];

  // Registers for pipe stage 2:
  reg [31:0] p2_x;
  reg [29:0] p2_bit_slice_1700;
  reg [30:0] p2_bit_slice_1701;
  reg [30:0] p2_bit_slice_1702;
  reg [28:0] p2_bit_slice_1703;
  reg [30:0] p2_bit_slice_1704;
  reg [30:0] p2_bit_slice_1705;
  reg [29:0] p2_bit_slice_1706;
  reg [29:0] p2_bit_slice_1707;
  reg [30:0] p2_bit_slice_1708;
  reg [30:0] p2_bit_slice_1709;
  reg [27:0] p2_bit_slice_1710;
  reg [1:0] p2_bit_slice_1711;
  reg p2_bit_slice_1712;
  reg [2:0] p2_bit_slice_1713;
  reg p2_bit_slice_1714;
  reg [1:0] p2_bit_slice_1715;
  reg p2_bit_slice_1716;
  reg [3:0] p2_bit_slice_1717;
  always_ff @ (posedge clk) begin
    p2_x <= p1_x;
    p2_bit_slice_1700 <= p2_bit_slice_1700_comb;
    p2_bit_slice_1701 <= p2_bit_slice_1701_comb;
    p2_bit_slice_1702 <= p2_bit_slice_1702_comb;
    p2_bit_slice_1703 <= p2_bit_slice_1703_comb;
    p2_bit_slice_1704 <= p2_bit_slice_1704_comb;
    p2_bit_slice_1705 <= p2_bit_slice_1705_comb;
    p2_bit_slice_1706 <= p2_bit_slice_1706_comb;
    p2_bit_slice_1707 <= p2_bit_slice_1707_comb;
    p2_bit_slice_1708 <= p2_bit_slice_1708_comb;
    p2_bit_slice_1709 <= p2_bit_slice_1709_comb;
    p2_bit_slice_1710 <= p2_bit_slice_1710_comb;
    p2_bit_slice_1711 <= p2_bit_slice_1711_comb;
    p2_bit_slice_1712 <= p2_bit_slice_1712_comb;
    p2_bit_slice_1713 <= p2_bit_slice_1713_comb;
    p2_bit_slice_1714 <= p2_bit_slice_1714_comb;
    p2_bit_slice_1715 <= p2_bit_slice_1715_comb;
    p2_bit_slice_1716 <= p2_bit_slice_1716_comb;
    p2_bit_slice_1717 <= p2_bit_slice_1717_comb;
  end

  // ===== Pipe stage 3:
  wire [30:0] p3_add_1803_comb;
  wire [29:0] p3_add_1805_comb;
  wire [30:0] p3_add_1806_comb;
  wire [28:0] p3_add_1807_comb;
  wire [30:0] p3_add_1808_comb;
  wire [29:0] p3_add_1809_comb;
  wire [30:0] p3_add_1810_comb;
  wire [27:0] p3_add_1811_comb;
  wire [31:0] p3_umul_1770_comb;
  wire [31:0] p3_umul_1771_comb;
  wire [31:0] p3_umul_1772_comb;
  wire [31:0] p3_umul_1773_comb;
  wire [31:0] p3_umul_1774_comb;
  wire [31:0] p3_umul_1775_comb;
  wire [31:0] p3_umul_1776_comb;
  wire [31:0] p3_umul_1777_comb;
  wire [31:0] p3_umul_1778_comb;
  wire [31:0] p3_umul_1779_comb;
  wire [31:0] p3_umul_1780_comb;
  wire [31:0] p3_umul_1781_comb;
  wire [31:0] p3_umul_1782_comb;
  wire [31:0] p3_umul_1783_comb;
  wire [31:0] p3_add_1829_comb;
  wire [31:0] p3_add_1830_comb;
  wire [31:0] p3_add_1831_comb;
  wire [31:0] p3_add_1832_comb;
  wire [30:0] p3_bit_slice_1789_comb;
  wire [30:0] p3_bit_slice_1790_comb;
  wire [29:0] p3_bit_slice_1791_comb;
  wire [29:0] p3_bit_slice_1792_comb;
  wire [30:0] p3_bit_slice_1793_comb;
  wire [30:0] p3_bit_slice_1794_comb;
  wire [28:0] p3_bit_slice_1795_comb;
  wire [28:0] p3_bit_slice_1796_comb;
  wire [30:0] p3_bit_slice_1797_comb;
  wire [30:0] p3_bit_slice_1798_comb;
  wire [29:0] p3_bit_slice_1799_comb;
  wire [29:0] p3_bit_slice_1800_comb;
  wire [30:0] p3_bit_slice_1801_comb;
  wire [30:0] p3_bit_slice_1802_comb;
  wire p3_bit_slice_1812_comb;
  wire [1:0] p3_bit_slice_1813_comb;
  wire p3_bit_slice_1814_comb;
  wire [2:0] p3_bit_slice_1815_comb;
  wire p3_bit_slice_1816_comb;
  wire [1:0] p3_bit_slice_1817_comb;
  wire p3_bit_slice_1818_comb;
  wire [31:0] p3_umul_1828_comb;
  wire [31:0] p3_add_1833_comb;
  wire [31:0] p3_add_1834_comb;
  assign p3_add_1803_comb = p2_x[31:1] + p2_x[30:0];
  assign p3_add_1805_comb = p2_bit_slice_1700 + p2_x[29:0];
  assign p3_add_1806_comb = p2_bit_slice_1701 + p2_bit_slice_1702;
  assign p3_add_1807_comb = p2_bit_slice_1703 + p2_x[28:0];
  assign p3_add_1808_comb = p2_bit_slice_1704 + p2_bit_slice_1705;
  assign p3_add_1809_comb = p2_bit_slice_1706 + p2_bit_slice_1707;
  assign p3_add_1810_comb = p2_bit_slice_1708 + p2_bit_slice_1709;
  assign p3_add_1811_comb = p2_bit_slice_1710 + p2_x[27:0];
  assign p3_umul_1770_comb = umul32b_32b_x_5b(p2_x, 5'h11);
  assign p3_umul_1771_comb = umul32b_32b_x_5b(p2_x, 5'h12);
  assign p3_umul_1772_comb = umul32b_32b_x_5b(p2_x, 5'h13);
  assign p3_umul_1773_comb = umul32b_32b_x_5b(p2_x, 5'h14);
  assign p3_umul_1774_comb = umul32b_32b_x_5b(p2_x, 5'h15);
  assign p3_umul_1775_comb = umul32b_32b_x_5b(p2_x, 5'h16);
  assign p3_umul_1776_comb = umul32b_32b_x_5b(p2_x, 5'h17);
  assign p3_umul_1777_comb = umul32b_32b_x_5b(p2_x, 5'h18);
  assign p3_umul_1778_comb = umul32b_32b_x_5b(p2_x, 5'h19);
  assign p3_umul_1779_comb = umul32b_32b_x_5b(p2_x, 5'h1a);
  assign p3_umul_1780_comb = umul32b_32b_x_5b(p2_x, 5'h1b);
  assign p3_umul_1781_comb = umul32b_32b_x_5b(p2_x, 5'h1c);
  assign p3_umul_1782_comb = umul32b_32b_x_5b(p2_x, 5'h1d);
  assign p3_umul_1783_comb = umul32b_32b_x_5b(p2_x, 5'h1e);
  assign p3_add_1829_comb = {p3_add_1803_comb, p2_x[0]} + {p3_add_1805_comb, p2_bit_slice_1711};
  assign p3_add_1830_comb = {p3_add_1806_comb, p2_bit_slice_1712} + {p3_add_1807_comb, p2_bit_slice_1713};
  assign p3_add_1831_comb = {p3_add_1808_comb, p2_bit_slice_1714} + {p3_add_1809_comb, p2_bit_slice_1715};
  assign p3_add_1832_comb = {p3_add_1810_comb, p2_bit_slice_1716} + {p3_add_1811_comb, p2_bit_slice_1717};
  assign p3_bit_slice_1789_comb = p3_umul_1770_comb[31:1];
  assign p3_bit_slice_1790_comb = p3_umul_1771_comb[31:1];
  assign p3_bit_slice_1791_comb = p3_umul_1772_comb[31:2];
  assign p3_bit_slice_1792_comb = p3_umul_1773_comb[31:2];
  assign p3_bit_slice_1793_comb = p3_umul_1774_comb[31:1];
  assign p3_bit_slice_1794_comb = p3_umul_1775_comb[31:1];
  assign p3_bit_slice_1795_comb = p3_umul_1776_comb[31:3];
  assign p3_bit_slice_1796_comb = p3_umul_1777_comb[31:3];
  assign p3_bit_slice_1797_comb = p3_umul_1778_comb[31:1];
  assign p3_bit_slice_1798_comb = p3_umul_1779_comb[31:1];
  assign p3_bit_slice_1799_comb = p3_umul_1780_comb[31:2];
  assign p3_bit_slice_1800_comb = p3_umul_1781_comb[31:2];
  assign p3_bit_slice_1801_comb = p3_umul_1782_comb[31:1];
  assign p3_bit_slice_1802_comb = p3_umul_1783_comb[31:1];
  assign p3_bit_slice_1812_comb = p3_umul_1770_comb[0];
  assign p3_bit_slice_1813_comb = p3_umul_1772_comb[1:0];
  assign p3_bit_slice_1814_comb = p3_umul_1774_comb[0];
  assign p3_bit_slice_1815_comb = p3_umul_1776_comb[2:0];
  assign p3_bit_slice_1816_comb = p3_umul_1778_comb[0];
  assign p3_bit_slice_1817_comb = p3_umul_1780_comb[1:0];
  assign p3_bit_slice_1818_comb = p3_umul_1782_comb[0];
  assign p3_umul_1828_comb = umul32b_32b_x_5b(p2_x, 5'h1f);
  assign p3_add_1833_comb = p3_add_1829_comb + p3_add_1830_comb;
  assign p3_add_1834_comb = p3_add_1831_comb + p3_add_1832_comb;

  // Registers for pipe stage 3:
  reg [30:0] p3_bit_slice_1789;
  reg [30:0] p3_bit_slice_1790;
  reg [29:0] p3_bit_slice_1791;
  reg [29:0] p3_bit_slice_1792;
  reg [30:0] p3_bit_slice_1793;
  reg [30:0] p3_bit_slice_1794;
  reg [28:0] p3_bit_slice_1795;
  reg [28:0] p3_bit_slice_1796;
  reg [30:0] p3_bit_slice_1797;
  reg [30:0] p3_bit_slice_1798;
  reg [29:0] p3_bit_slice_1799;
  reg [29:0] p3_bit_slice_1800;
  reg [30:0] p3_bit_slice_1801;
  reg [30:0] p3_bit_slice_1802;
  reg p3_bit_slice_1812;
  reg [1:0] p3_bit_slice_1813;
  reg p3_bit_slice_1814;
  reg [2:0] p3_bit_slice_1815;
  reg p3_bit_slice_1816;
  reg [1:0] p3_bit_slice_1817;
  reg p3_bit_slice_1818;
  reg [31:0] p3_umul_1828;
  reg [31:0] p3_add_1833;
  reg [31:0] p3_add_1834;
  always_ff @ (posedge clk) begin
    p3_bit_slice_1789 <= p3_bit_slice_1789_comb;
    p3_bit_slice_1790 <= p3_bit_slice_1790_comb;
    p3_bit_slice_1791 <= p3_bit_slice_1791_comb;
    p3_bit_slice_1792 <= p3_bit_slice_1792_comb;
    p3_bit_slice_1793 <= p3_bit_slice_1793_comb;
    p3_bit_slice_1794 <= p3_bit_slice_1794_comb;
    p3_bit_slice_1795 <= p3_bit_slice_1795_comb;
    p3_bit_slice_1796 <= p3_bit_slice_1796_comb;
    p3_bit_slice_1797 <= p3_bit_slice_1797_comb;
    p3_bit_slice_1798 <= p3_bit_slice_1798_comb;
    p3_bit_slice_1799 <= p3_bit_slice_1799_comb;
    p3_bit_slice_1800 <= p3_bit_slice_1800_comb;
    p3_bit_slice_1801 <= p3_bit_slice_1801_comb;
    p3_bit_slice_1802 <= p3_bit_slice_1802_comb;
    p3_bit_slice_1812 <= p3_bit_slice_1812_comb;
    p3_bit_slice_1813 <= p3_bit_slice_1813_comb;
    p3_bit_slice_1814 <= p3_bit_slice_1814_comb;
    p3_bit_slice_1815 <= p3_bit_slice_1815_comb;
    p3_bit_slice_1816 <= p3_bit_slice_1816_comb;
    p3_bit_slice_1817 <= p3_bit_slice_1817_comb;
    p3_bit_slice_1818 <= p3_bit_slice_1818_comb;
    p3_umul_1828 <= p3_umul_1828_comb;
    p3_add_1833 <= p3_add_1833_comb;
    p3_add_1834 <= p3_add_1834_comb;
  end

  // ===== Pipe stage 4:
  wire [30:0] p4_add_1883_comb;
  wire [29:0] p4_add_1884_comb;
  wire [30:0] p4_add_1885_comb;
  wire [28:0] p4_add_1886_comb;
  wire [30:0] p4_add_1887_comb;
  wire [29:0] p4_add_1888_comb;
  wire [30:0] p4_add_1889_comb;
  wire [31:0] p4_add_1897_comb;
  wire [31:0] p4_add_1898_comb;
  wire [31:0] p4_add_1899_comb;
  wire [31:0] p4_add_1900_comb;
  wire [31:0] p4_add_1901_comb;
  wire [31:0] p4_add_1902_comb;
  wire [31:0] p4_add_1903_comb;
  assign p4_add_1883_comb = p3_bit_slice_1789 + p3_bit_slice_1790;
  assign p4_add_1884_comb = p3_bit_slice_1791 + p3_bit_slice_1792;
  assign p4_add_1885_comb = p3_bit_slice_1793 + p3_bit_slice_1794;
  assign p4_add_1886_comb = p3_bit_slice_1795 + p3_bit_slice_1796;
  assign p4_add_1887_comb = p3_bit_slice_1797 + p3_bit_slice_1798;
  assign p4_add_1888_comb = p3_bit_slice_1799 + p3_bit_slice_1800;
  assign p4_add_1889_comb = p3_bit_slice_1801 + p3_bit_slice_1802;
  assign p4_add_1897_comb = {p4_add_1883_comb, p3_bit_slice_1812} + {p4_add_1884_comb, p3_bit_slice_1813};
  assign p4_add_1898_comb = {p4_add_1885_comb, p3_bit_slice_1814} + {p4_add_1886_comb, p3_bit_slice_1815};
  assign p4_add_1899_comb = {p4_add_1887_comb, p3_bit_slice_1816} + {p4_add_1888_comb, p3_bit_slice_1817};
  assign p4_add_1900_comb = {p4_add_1889_comb, p3_bit_slice_1818} + p3_umul_1828;
  assign p4_add_1901_comb = p4_add_1897_comb + p4_add_1898_comb;
  assign p4_add_1902_comb = p4_add_1899_comb + p4_add_1900_comb;
  assign p4_add_1903_comb = p3_add_1833 + p3_add_1834;

  // Registers for pipe stage 4:
  reg [31:0] p4_add_1901;
  reg [31:0] p4_add_1902;
  reg [31:0] p4_add_1903;
  always_ff @ (posedge clk) begin
    p4_add_1901 <= p4_add_1901_comb;
    p4_add_1902 <= p4_add_1902_comb;
    p4_add_1903 <= p4_add_1903_comb;
  end

  // ===== Pipe stage 5:
  wire [31:0] p5_add_1910_comb;
  wire [31:0] p5_add_1911_comb;
  assign p5_add_1910_comb = p4_add_1901 + p4_add_1902;
  assign p5_add_1911_comb = p4_add_1903 + p5_add_1910_comb;

  // Registers for pipe stage 5:
  reg [31:0] p5_add_1911;
  always_ff @ (posedge clk) begin
    p5_add_1911 <= p5_add_1911_comb;
  end
  assign out = p5_add_1911;
endmodule
