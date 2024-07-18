
#pragma once
// #ifndef __BASE_PREDICTOR__
// #define __BASE_PREDICTOR__


#define UINT32      unsigned int
#define INT32       int
#define UINT64      unsigned long long
// #define COUNTER     unsigned long long

//JD2_17_2016 break down types into COND/UNCOND
typedef enum {
  OPTYPE_OP               =2,

  OPTYPE_RET_UNCOND,
  OPTYPE_JMP_DIRECT_UNCOND,
  OPTYPE_JMP_INDIRECT_UNCOND,
  OPTYPE_CALL_DIRECT_UNCOND,
  OPTYPE_CALL_INDIRECT_UNCOND,

  OPTYPE_RET_COND,
  OPTYPE_JMP_DIRECT_COND,
  OPTYPE_JMP_INDIRECT_COND,
  OPTYPE_CALL_DIRECT_COND,
  OPTYPE_CALL_INDIRECT_COND,

  OPTYPE_ERROR,

  OPTYPE_MAX
}OpType;



typedef enum {
    NoBranch,
    Return,
    CallDirect,
    CallIndirect,
    DirectCond,
    DirectUncond,
    IndirectCond,
    IndirectUncond,
    MAX
} BrType;


// enum branch_type {
//   NOT_BRANCH = 0,
//   BRANCH_DIRECT_JUMP = 1,
//   BRANCH_INDIRECT = 2,
//   BRANCH_CONDITIONAL = 3,
//   BRANCH_DIRECT_CALL = 4,
//   BRANCH_INDIRECT_CALL = 5,
//   BRANCH_RETURN = 6,
//   BRANCH_OTHER = 7
// };








class BasePredictor {
    static inline UINT32 SatIncrement(UINT32 x, UINT32 max) {
        if (x < max) return x + 1;
        return x;
    }

    static inline UINT32 SatDecrement(UINT32 x) {
        if (x > 0) return x - 1;
        return x;
    }

   public:
    BasePredictor() {};
    virtual ~BasePredictor() = default;

    virtual bool GetPrediction(uint64_t PC) = 0;
    virtual void FirstTimeUpdate(uint64_t PC, bool taken,
                                uint64_t branchTarget) {};
    virtual void UpdatePredictor(uint64_t PC, bool resolveDir,
                                 bool predDir, uint64_t branchTarget) = 0;

    virtual void TrackOtherInst(uint64_t PC, OpType opType, bool taken,
                                uint64_t branchTarget) = 0;

    virtual void PrintStat(double NUMINST) {};
    virtual void DumpTables(std::string filename) {};
    virtual void LoadTables(std::string filename) {};
    virtual void StartTracer(std::string filename) {};
    virtual void tick() {};
    virtual void resetStats() {};
    virtual void btbMiss() {};
    virtual void setState(bool warmup=false) {};
    virtual void commit() {};
};



BasePredictor* CreateBP(std::string bp_name);

// #endif  //__BASE_PREDICTOR__
