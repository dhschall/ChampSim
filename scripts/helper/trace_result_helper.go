package helper

import (
	"fmt"
	"math"
	"sort"
)

type TAGEInfo struct {
	Tage_miss       uint    `json:"tage_miss"`
	Sc_miss         uint    `json:"sc_miss"`
	Loop_miss       uint    `json:"loop_miss"`
	Tage_pred       uint    `json:"tage_pred"`
	Sc_pred         uint    `json:"sc_pred"`
	Loop_pred       uint    `json:"loop_pred"`
	N_alloc         uint    `json:"n_alloc"`
	N_entries_alloc uint    `json:"n_entries_alloc"`
	Hit_bank        uint    `json:"hit_bank"`
	Avg_hit_bank    float64 `json:"avg_hit_bank"`
	Utilization     uint    `json:"utilization"`
	MaxUtil         uint    `json:"max_util"`
	Avg_utilization float64 `json:"avg_utilization"`
}

func (r *TAGEInfo) Merge(other TAGEInfo) {
	r.Tage_miss += other.Tage_miss
	r.Sc_miss += other.Sc_miss
	r.Loop_miss += other.Loop_miss
	r.Tage_pred += other.Tage_pred
	r.Sc_pred += other.Sc_pred
	r.Loop_pred += other.Loop_pred
	r.Hit_bank += other.Hit_bank
	r.N_alloc += other.N_alloc
	r.N_entries_alloc += other.N_entries_alloc
	r.Utilization += other.Utilization
	if other.MaxUtil > r.MaxUtil {
		r.MaxUtil = other.MaxUtil
	}
}

// func (r *TAGEInfo) Process() {

// 	r.Avg_hit_bank = uint(float64(r.Hit_bank) / float64(r.N_exe))
// 	r.Avg_utilization = uint(float64(r.Utilization) / float64(r.N_exe))
// }

type BranchResult struct {
	// BrIP     uint `json:"BR_IP"`
	// BrType   uint `json:"BR_TYPE"`
	// BrTarget uint `json:"BR_TARGET"`
	// N_exe    uint `json:"N_EXE"`
	// N_miss   uint `json:"N_MISS"`
	// N_taken  uint `json:"N_TAKEN"`

	BrIP      uint     `json:"ip"`
	BrType    uint     `json:"type"`
	BrTarget  uint     `json:"target"`
	N_exe     uint     `json:"N_EXE"`
	N_miss    uint     `json:"N_MISS"`
	N_taken   uint     `json:"N_TAKEN"`
	Tage_info TAGEInfo `json:"tage_info"`

	BrIP_H     string  `json:"BR_IP_H"`
	BrTarget_H string  `json:"BR_TARGET_H"`
	BrType_T   string  `json:"BR_TYPE_T"`
	Accuracy   float32 `json:"BR_ACCURACY"`
	Dynamism   float32 `json:"BR_DYNAMISM"`
	Static     bool    `json:"BR_Static"`
	H2P        bool    `json:"BR_H2P"`
	MissRatio  float32 `json:"MISS_RATIO"`
	MissOrder  uint    `json:"MISS_ORDER"`

	BAResult *BrAnalysisResults `json:"ba_result"`
}

func (r *BranchResult) Merge(other BranchResult) {
	r.N_exe += other.N_exe
	r.N_miss += other.N_miss
	r.N_taken += other.N_taken
	if r.BrTarget == 0 {
		r.BrTarget = other.BrTarget
	}
	r.Tage_info.Merge(other.Tage_info)
}

func (r *BranchResult) Process() {
	// Create hexadecimal values
	r.BrIP_H = fmt.Sprintf("%#x", r.BrIP)
	r.BrTarget_H = fmt.Sprintf("%#x", r.BrTarget)

	// Convert branch type
	switch r.BrType {
	case 0:
		r.BrType_T = "NOT_BRANCH"
	case 1:
		r.BrType_T = "DIRECT_JUMP"
	case 2:
		r.BrType_T = "INDIRECT"
	case 3:
		r.BrType_T = "CONDITIONAL"
	case 4:
		r.BrType_T = "DIRECT_CALL"
	case 5:
		r.BrType_T = "INDIRECT_CALL"
	case 6:
		r.BrType_T = "RETURN"
	case 7:
		r.BrType_T = "OTHER"
	default:
		fmt.Println("Don't know the type")
	}

	// Calculate some statistical properties
	r.Accuracy = 1.0 - (float32(r.N_miss) / float32(r.N_exe))
	//// Dynamism
	// How much dynamism is within this branch..
	// A static branch has 0%. If the branch is taken as much as it is not taken, then the dynamism is 100%
	r.Dynamism = 1.0 - float32(math.Abs(float64(r.N_taken)/float64(r.N_exe)-0.5))*2.0

	if r.N_exe == r.N_taken || r.N_taken == 0 {
		r.Static = true
	} else {
		r.Static = false
	}

	if r.N_exe > 15000 && r.N_miss > 1000 && r.Accuracy < 0.99 {
		r.H2P = true
	} else {
		r.H2P = false
	}

	r.Tage_info.Avg_hit_bank = float64(r.Tage_info.Hit_bank) / float64(r.N_exe)
	if r.Tage_info.N_alloc > 0 {
		r.Tage_info.Avg_utilization = float64(r.Tage_info.Utilization) / float64(r.Tage_info.N_alloc) / 2.0
	}
}

type TraceResult struct {
	Sim_inst     uint                   `json:"sim_inst"`
	Sim_cycle    uint                   `json:"sim_cycle"`
	IPC          float32                `json:"IPC"`
	MPKI         float32                `json:"MPKI"`
	Sum_retired  uint                   `json:"sum_retired"`
	Warm_inst    uint                   `json:"warm_inst"`
	Sum_branch   uint                   `json:"sum_branch"`
	Sum_miss     uint                   `json:"sum_miss"`
	Rob_occu     uint                   `json:"rob_occu"`
	Accuracy     float32                `json:"Accuracy"`
	Avg_rob_occu float32                `json:"avg_rob_occu"`
	Branches     []BranchResult         `json:"branches"`
	Branch_map   map[uint]*BranchResult `json:"branch_map"`
}

func (r *TraceResult) Merge(other TraceResult) {
	r.Sim_inst += other.Sim_inst
	r.Sim_cycle += other.Sim_cycle
	r.Sum_retired += other.Sum_retired
	r.Warm_inst += other.Warm_inst
	r.Sum_branch += other.Sum_branch
	r.Sum_miss += other.Sum_miss
	r.Rob_occu += other.Rob_occu
}

func (r *TraceResult) Process() {
	r.IPC = float32(r.Sim_inst) / float32(r.Sim_cycle)
	r.Accuracy = (float32(r.Sum_branch) - float32(r.Sum_miss)) / float32(r.Sum_branch)
	r.MPKI = 1000 * float32(r.Sum_miss) / (float32(r.Sum_retired) - float32(r.Warm_inst))
	r.Avg_rob_occu = float32(r.Rob_occu) / float32(r.Sum_miss)
}

// Sort functions
type SortPair struct {
	Key   uint
	Value float32
}

type SortList []SortPair

func (a SortList) Len() int           { return len(a) }
func (a SortList) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }
func (a SortList) Less(i, j int) bool { return a[i].Value > a[j].Value }

func sortMissRatio(m *map[uint]*BranchResult) []SortPair {
	// Prepare a pair list with key and miss ratio
	pairList := make(SortList, len(*m))
	i := 0
	for k, v := range *m {
		pairList[i] = SortPair{k, v.MissRatio}
		i++
	}

	// Do the sorting
	sort.Sort(pairList)

	// Write the order in the struct
	for i, p := range pairList {
		(*m)[p.Key].MissOrder = uint(i)
	}

	return pairList
}

func PrintMostSignificant(m *map[uint]*BranchResult, n int) {
	miss_o := sortMissRatio(m)

	fmt.Printf("%d Most signficat branch instructions:\n", n)
	fmt.Printf("  | Br. IP   | Miss R   | Accuracy | Dynamism | Avg Util |   ID   |\n")
	for i := 0; i < n; i++ {
		e := (*m)[miss_o[i].Key]
		fmt.Printf("%d | %#x | %f | %f | %f | %8f |",
			i, e.BrIP, e.MissRatio, e.Accuracy, e.Dynamism, e.Tage_info.Avg_utilization)

		if e.BAResult != nil {
			fmt.Printf(" %d", e.BAResult.ID)
		} else {
			fmt.Printf("\n")
		}
	}
}
