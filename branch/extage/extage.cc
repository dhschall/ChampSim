#include <map>
#include <unordered_set>

#include "msl/fwcounter.h"
#include "ooo_cpu.h"

namespace
{
constexpr std::size_t BIMODAL_TABLE_SIZE = 16384;
constexpr std::size_t BIMODAL_PRIME = 16381;
constexpr std::size_t COUNTER_BITS = 2;

std::map<O3_CPU*, std::array<champsim::msl::fwcounter<COUNTER_BITS>, BIMODAL_TABLE_SIZE>> bimodal_table;
} // namespace

// void CPU::initialize_branch_predictor() {}

uint8_t bim_predict_branch(uint64_t ip)
{
  auto hash = ip % ::BIMODAL_PRIME;
  auto value = ::bimodal_table[0][hash];

  return value.value() >= (value.maximum / 2);
}

void bim_last_branch_result(uint64_t ip, uint64_t branch_target, uint8_t taken, uint8_t branch_type)
{
  auto hash = ip % ::BIMODAL_PRIME;
  ::bimodal_table[0][hash] += taken ? 1 : -1;
}


#include "base_predictor_lib.h"


OpType convertBrType(uint8_t type) {
    switch (type) {
        case BRANCH_DIRECT_JUMP:
            return OPTYPE_JMP_DIRECT_UNCOND;
        case BRANCH_INDIRECT:
            return OPTYPE_JMP_INDIRECT_UNCOND;
        case BRANCH_CONDITIONAL:
            return OPTYPE_JMP_DIRECT_COND;
        case BRANCH_DIRECT_CALL:
            return OPTYPE_CALL_DIRECT_UNCOND;
        case BRANCH_INDIRECT_CALL:
            return OPTYPE_CALL_INDIRECT_UNCOND;
        case BRANCH_RETURN:
            return OPTYPE_RET_UNCOND;
        default:
            return OPTYPE_OP;
    }
}



uint8_t prediction;
uint8_t bim_prediction;
uint8_t tage_prediction;
uint8_t observedTaken;
const bool filterNeverTaken = true;
std::unordered_set<uint64_t> takenPCs;
std::array<std::unordered_set<uint64_t>, 8> uBranches;



BasePredictor* brpred;

int bim_mispredictions = 0;
int tage_mispredictions = 0;


void O3_CPU::initialize_branch_predictor(std::string model)
{
 brpred = CreateBP(model);
}


void O3_CPU::finish_branch_predictor(int inst)
{

constexpr std::array<std::pair<std::string_view, std::size_t>, 6> types{
      {std::pair{"BRANCH_DIRECT_JUMP", BRANCH_DIRECT_JUMP}, std::pair{"BRANCH_INDIRECT", BRANCH_INDIRECT}, std::pair{"BRANCH_CONDITIONAL", BRANCH_CONDITIONAL},
       std::pair{"BRANCH_DIRECT_CALL", BRANCH_DIRECT_CALL}, std::pair{"BRANCH_INDIRECT_CALL", BRANCH_INDIRECT_CALL},
       std::pair{"BRANCH_RETURN", BRANCH_RETURN}}};


    brpred->PrintStat(inst);
    delete brpred;
    printf("ZZZ Num_unique_taken_branches %lu\n", takenPCs.size());


    for (auto [str, idx] : types)
        printf("ZZZ %s_NUNIQUE %lu\n", str.data(), uBranches[idx].size());

    printf("ZZZ BIM_MISPREDICTIONS %d\n", bim_mispredictions);
    printf("ZZZ TAGE_MISPREDICTIONS %d\n", tage_mispredictions);
    printf("ZZZ BIM_MPKI %f\n", (double)bim_mispredictions / inst * 1000);
    printf("ZZZ TAGE_MPKI %f\n", (double)tage_mispredictions / inst * 1000);

}

uint8_t O3_CPU::predict_branch(uint64_t ip)
{
    // auto it = takenPCs.find(PC);
    auto taken = uBranches[BRANCH_CONDITIONAL].find(ip) != uBranches[BRANCH_CONDITIONAL].end();

    observedTaken = !filterNeverTaken || taken;
    prediction = 0;

    if (observedTaken) {


        bim_prediction = bim_predict_branch(ip);
        tage_prediction = brpred->GetPrediction(ip);

        prediction = bim_prediction | (tage_prediction << 1);
    }

    return prediction;
}

void O3_CPU::last_branch_result(uint64_t ip, uint64_t branch_target, uint8_t taken, uint8_t branch_type)
// void O3_CPU::last_branch_result(uint64_t PC, uint8_t taken, uint64_t branch_target, uint8_t branch_type)
{
    OpType opType = convertBrType(branch_type);
    if (branch_type == BRANCH_CONDITIONAL) {
        if (observedTaken) {
            brpred->UpdatePredictor(ip, opType, taken, tage_prediction,
                                    branch_target);
            // printf("PC: %llx type: %x outcome: %d pred: %d\n", ip, (UINT32)opType, taken, prediction);


        } else {
            brpred->FirstTimeUpdate(ip, taken, branch_target);
        }
    } else {
        brpred->TrackOtherInst(ip, opType, taken,
                                branch_target);
    }
    bim_last_branch_result(ip, branch_target, taken, branch_type);

    if (taken) {
        takenPCs.insert(ip);
        uBranches[branch_type].insert(ip);
    }

    if (branch_type == BRANCH_CONDITIONAL) {
        if (observedTaken) {
            if (tage_prediction != taken) {
                tage_mispredictions++;
            }
            if (bim_prediction != taken) {
                bim_mispredictions++;
            }
        }
    }

}


void O3_CPU::tick_branch_predictor(int inst) {
    brpred->tick();
}