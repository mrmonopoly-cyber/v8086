# Encoding

## Specials

PUSH(000xxx110): Segment Reg
POP (000xxx111): Segment Reg

## 4 Bits

- MOV (11011): Imm to Reg

## 5 Bits

- PUSH(01010): Reg
- POP (01011): Reg
- XCHG(10010): Reg with Acc

## 7 Bits

- MOV (1000010): Reg/Mem to/from Reg
- MOV (1100011): Imm to Reg/Mem
- MOV (1010000): Mem to Acc
- MOV (1010001): Acc to Mem
- XCHG(1000011): Reg/Mem with Reg
- IN  (1110010): Fixed Port
- IN  (1110110): Variable Port
- OUT (1110011): Fixed Port
- OUT (1110111): Variable Port

## 8 Bits

- MOV (10001100): Reg/Mem to segment register
- MOV (10001110): segment register to Reg/Mem
- PUSH(11111111): Reg/Mem
- POP (10001111): Reg/Mem
- XLAT(11010111): Translate Byte to AL

