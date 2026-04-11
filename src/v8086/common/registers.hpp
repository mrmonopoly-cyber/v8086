#pragma once

#define REG_HL(Name) Name##x, Name##h, Name##l
#define REG_X(Name) Name
enum Reg{
  REG_HL(a),
  REG_HL(b),
  REG_HL(c),
  REG_HL(d),
  REG_X(sp),
  REG_X(bp),
  REG_X(si),
  REG_X(di),
};
#undef REG_HL
#undef REG_X
