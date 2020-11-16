package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"strconv"
	"strings"

	. "./helper"
)

// "BR_IP" : 4218014, "BR_TYPE" : 3, "BR_TARGET" : 0, "N_EXE" : 2840, "N_TAKEN" : 0, "N_MISS" : 0

func readResult(filename string) (ret TraceResult) {
	file, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	reader := bufio.NewReader(file)
	var line string
	for {
		line, err = reader.ReadString('\n')
		if err != nil && err != io.EOF {
			log.Fatal(err)
		}

		if len(line) > 9 {
			if ret := strings.Compare(line[:9], "BT JSON: "); ret == 0 {
				break
			}
		}
	}
	var t []byte
	if line[len(line)-4] == ',' {
		fmt.Println("Found wrong comma. Remove it.")
		t = []byte(line[9:len(line)-4] + "]")
	} else {
		t = []byte(line[9:])
	}

	err = json.Unmarshal(t, &ret)
	if err != nil {
		log.Fatal("There was an error decoding the json. err = %s", err)
	}
	return ret
}

// New argument type that specify strings that need to be in the result file.
type searchFlags []string

func (v *searchFlags) String() string {
	var s string
	for _, v := range *v {
		s += v + " "
	}
	return s
}
func (v *searchFlags) Set(value string) error {
	ss := strings.Split(value, ",")
	for _, s := range ss {
		*v = append(*v, strings.TrimSpace(s))
	}
	return nil
}

func findResultFiles(dir string, values []string) (ret []string) {
	file, err := os.Open(dir + ".")
	if err != nil {
		log.Fatalf("failed opening directory: %s", err)
	}
	defer file.Close()

	fmt.Println(values)

	list, _ := file.Readdirnames(0) // 0 to read all files and folders
	for _, filename := range list {
		// fmt.Println(filename)
		// check that all search strings are contained in the file.
		match := true
		for _, s := range values {
			if !strings.Contains(filename, s) {
				match = false
			}
		}
		if match {
			ret = append(ret, filename)
		}
	}
	return
}

func main() {
	// Get arguments ---------------------------------------
	var sFlags searchFlags
	flag.Var(&sFlags, "f", "Specify strings that must match in the result file.")
	var dir string
	flag.StringVar(&dir, "d", "../results_1000M/", "Specify the results directory.")
	// var ba_file string
	// flag.StringVar(&ba_file, "a", "-", "If sepecified, the we will try to merge the results from the static branch-analysis.")
	flag.Parse()

	// Get the results files
	var files []string
	if len(sFlags) > 0 {
		// Find mode:
		// In this mode we search for files in a directory
		files = findResultFiles(dir, sFlags)
		fmt.Printf("Found %d result files matching the constraints:\n", len(files))
		if len(files) == 0 {
			return
		}
		fmt.Println(files)
		fmt.Println("Do you want to merge them? [y,yes]")
		var ans string
		fmt.Scanf("%s", &ans)
		if strings.Compare(ans, "y") != 0 && strings.Compare(ans, "yes") != 0 {
			return
		}
	} else {
		// Predefined mode:
		benchmark := "605.mcf_s"
		postfix := "-llvm11"
		binary := "tage_sc_l_64k-no-no-no-no-lru-1core-dev"

		n_ := 10
		for i := 0; i < n_; i++ {
			file := benchmark + "-" + strconv.Itoa(i) + postfix + ".champsim.gz-" + binary + ".txt"
			files = append(files, file)
		}
	}

	// Read the information from the trace result file -----
	var results []BranchResult
	var trace_result TraceResult
	for _, file := range files {
		ret := readResult(dir + file)
		trace_result.Merge(ret)
		results = append(results, ret.Branches...)
	}

	// Merge the branch structs -----------------------------------
	var results_merged = make(map[uint]*BranchResult)

	for i, res := range results {
		if _, ok := results_merged[res.BrIP]; !ok {
			// This address wasn't found in the map
			results_merged[res.BrIP] = &results[i]
		} else {
			// Add the information to the exitsting once.
			// fmt.Printf("%#x\n", vp)
			results_merged[res.BrIP].Merge(res)
		}
	}

	// Process results --------------------
	trace_result.Process()
	var n_miss_all uint
	for k := range results_merged {
		// fmt.Printf("%#x", v)
		// fmt.Printf("%#x\n", results_merged[k])
		results_merged[k].Process()
		n_miss_all += results_merged[k].N_miss
	}

	// Update miss ratio
	for k, v := range results_merged {
		results_merged[k].MissRatio = float32(v.N_miss) / float32(n_miss_all)
	}

	fmt.Printf(">> Overal IPC: %f, MPKI: %f, Accuracy: %f\n",
		trace_result.IPC, trace_result.MPKI, trace_result.Accuracy)

	PrintMostSignificant(&results_merged, 12)

	// Write the merged results ---------------
	trace_result.Branch_map = results_merged
	data, err := json.MarshalIndent(trace_result, "", " ")
	if err != nil {
		log.Fatal("Unable to create the JSON. err = %s", err)
	}

	if err := ioutil.WriteFile("test.json", data, 0644); err != nil {
		log.Fatal(err)
	}
}
