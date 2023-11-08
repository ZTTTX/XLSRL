# proto-file: xls/contrib/xlscc/hls_block.proto
# proto-message: HLSBlock

channels {
  name: "in"
  is_input: true
  type: FIFO
  width_in_bits: 32
}
channels {
  name: "out"
  is_input: false
  type: FIFO
  width_in_bits: 32
}
name: "TestBlock"
