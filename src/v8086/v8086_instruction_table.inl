#ifndef ORG
#define ORG(Mnemonic, ...) {op_##Mnemonic, __VA_ARGS__},
#endif // !ORG

#ifndef VAR
#define VAR ORG
#endif // !VAR


#define Bits(bits) {BitSeq, sizeof(#bits)-1, 0b##bits}
#define S {Bit_S, 1}
#define W {Bit_W, 1}
#define D {Bit_D, 1}
#define V {Bit_V, 1}
#define Z {Bit_Z, 1}

#define MOD {Bits_MOD, 2}
#define SR  {Bits_SR, 2}
#define REG {Bits_REG, 3}
#define RM  {Bits_RM, 3}

#define DATA {Bits_DATA, 8}
#define DATA_IF_W {Bits_DATA_W, 8}
#define ADDR {Bits_DATA, 8} {Bits_DATA_ALWAYS_W, 8}


//mov
ORG(mov, {Bits(100010), D,W, MOD, REG, RM})                       /**reg/mem to/from reg*/
VAR(mov, {Bits(1100011), W, MOD, Bits(000), RM, DATA, DATA_IF_W}) /**imm to reg/mem*/
VAR(mov, {Bits(1011), W, REG, DATA, DATA_IF_W})                   /**mem to acc*/
VAR(mov, {Bits(1010000), W, REG, ADDR)                            /**acc to mem*/
VAR(mov, {Bits(1010001), W, REG, ADDR)                            /**reg/mem to seg mem*/
VAR(mov, {Bits(10001110), MOD, Bits(0), SR, RM})                  /**seg mem to reg/mem*/

//push
ORG(push, {Bits(11111111), MOD, Bits(110), RM})                   /**reg/mem*/
VAR(push, {Bits(01010), REG})                                     /**reg*/
VAR(push, {Bits(000), REG, Bits(110)})                            /**seg reg*/

//pop
ORG(pop, {Bits(10001111), MOD, Bits(000), RM})                    /**reg/mem*/
VAR(pop, {Bits(01011), REG})                                      /**reg*/
VAR(pop, {Bits(000), REG, Bits(111)})                             /**seg reg*/

//xchg
ORG(xchg, {Bits(1000011), W, MOD, REG, RN})                       /**reg/mem with reg*/
VAR(xchg, {Bits(10010), REG})                                     /**reg with acc*/

//in
ORG(in, {Bits(1110010), W, DATA})                                 /**fixed port*/
VAR(in, {Bits(1110110), W,})                                      /**variable port*/

//out
ORG(out, {Bits(1110011), W, DATA})                                /**fixed port*/
VAR(out, {Bits(1110111), W,})                                     /**variable port*/

//xlat
ORG(xlat, {Bits(11010111)})                                       /**translate byte to AL*/

//lead
ORG(lea, {Bits(10001101), MOD, REG, RM})                          /**load EA to register*/

//l[ds, es]
ORG(lds, {Bits(11000101), MOD, REG, RM})                          /**load pointer to DS*/
ORG(les, {Bits(11000100), MOD, REG, RM})                          /**load pointer to ES*/

//[l,s]ahf 
ORG(lahf, {Bits(10011111)})                                       /**load AH with flags*/
ORG(sahf, {Bits(10011110)})                                       /**store AH into flags*/

//[push,pop]f
ORG(pushf, {Bits(1001110)})                                       /**push flags*/
ORG(popf, {Bits(1001111)})                                        /**pop flags*/

//add
ORG(add, {Bits(000000), D, W, MOD, REG, RM})                      /**reg/mem with reg to either*/
ORG(add, {Bits(100000), S, W, MOD, Bits(000), RM})                /**imm to reg/mem*/
ORG(add, {Bits(0000010), W, DATA, DATA_IF_W})                     /**imm to acc*/

//adc
ORG(adc, {Bits(000100), D, W, MOD, REG, RM})                      /**reg/mem with reg to either*/
ORG(adc, {Bits(100000), S, W, MOD, Bits(010), RM})                /**imm to reg/mem*/
ORG(adc, {Bits(0001010), W, DATA, DATA_IF_W})                     /**imm to acc*/

//inc
ORG(inc, {Bits(1111111), W, MOD, Bits(000), RM})                  /**reg/mem*/
ORG(inc, {Bits(01000), REG})                                      /**reg*/

//aaa
ORG(aaa, {Bits(00110111)})                                        /**ASCII adjust for add*/

//daa
ORG(daa, {Bits(00100111)})                                        /**decimal adjust for add*/

//sub
ORG(sub, {Bits(001010), D, W, MOD, REG, RM})                      /**reg/mem with reg to either*/
ORG(sub, {Bits(100000), S, W, MOD, Bits(101), RM})                /**imm to reg/mem*/
ORG(sub, {Bits(0010110), W, DATA, DATA_IF_W})                     /**imm to acc*/

//sbb
ORG(sbb, {Bits(000110), D, W, MOD, REG, RM})                      /**reg/mem with reg to either*/
ORG(sbb, {Bits(100000), S, W, MOD, Bits(101), RM})                /**imm to reg/mem*/
ORG(sbb, {Bits(0001110), W, DATA, DATA_IF_W})                     /**imm to acc*/

//dec
ORG(dec, {Bits(1111111), W, MOD, Bits(001), RM})                  /**reg/mem*/
ORG(dec, {Bits(01001), REG})                                      /**reg*/

//neg
ORG(neg, {Bits(1111011), W, MOD, Bits(011), RM})                  /**change sign*/

//cmp
ORG(cmp, {Bits(001110), D, W, MOD, REG, RM})                      /**reg/mem and reg*/
ORG(cmp, {Bits(100000), S, W, MOD, Bits(111), RM})                /**imm with reg/mem*/
ORG(cmp, {Bits(0011110), DATA})                                   /**imm with acc*/

//aas
ORG(aas, {Bits(00111111)})                                        /**ASCII adjust for subtract*/

//das
ORG(das, {Bits(00100111)})                                        /**decimal adjust for subtract*/

//mul-imul
ORG(mul, {Bits(1111011), W, MOD, Bits(100), RM})                  /**multiply (unsigned)*/
ORG(imul, {Bits(1111011), W, MOD, Bits(101), RM})                 /**multiply (signed)*/

//aam
ORG(aam, {Bits(110101000), Bits(00001010)})                       /**ASCII adjust for multiply*/

//div-idiv
ORG(div, {Bits(1111011), W, MOD, Bits(110), RM})                  /**divide (unsigned)*/
ORG(idiv, {Bits(1111011), W, MOD, Bits(111), RM})                 /**divide (signed)*/

//aad
ORG(aad, {Bits(11010101), Bits(00001010)})                        /**ASCII adjust for divide*/

//cbw-cwd
ORG(cbw, {Bits(10011000)})                                        /**convert byte to word*/
ORG(cdw, {Bits(10011001)})                                        /**convert word to double word*/


#undef ORG
#undef VAR

#undef Bits
#undef S
#undef W
#undef D
#undef V
#undef Z

#undef MOD
#undef REG
