package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"strings"
	"os/exec"
	"io/ioutil"
	"sync"
)

// mkdir -p results_${N_SIM}M
// (./bin/${BINARY} -warmup_instructions ${N_WARM}000000 -simulation_instructions ${N_SIM}000000 ${OPTION} -traces ${TRACE_DIR}/${TRACE} ) &> results_${N_SIM}M/${TRACE}-${BINARY}${OPTION}.txt

// type Job struct {
// 	BP string
// 	N_WARM int
// 	N_SIM int

// }
type job struct {
	cmd []string
	res string
}
func runCmd(j job, wg *sync.WaitGroup) {
	defer wg.Done() 
	fmt.Println(j.cmd)
	// construct command
	// cmd := &exec.Cmd {
	// 	Path: "./../bin/tage_sc_l_64k-no-no-no-no-lru-1core-dev-filter2",
	// 	Args: []string{ "./../bin/tage_sc_l_64k-no-no-no-no-lru-1core-dev-filter2", 
	// 									"-warmup_instructions=1000000", 
	// 									"-simulation_instructions=1000000", 
	// 									"-traces=../spec2017_traces/1B/605.mcf_s-9-llvm11.champsim.gz" },
	// }
	cmd := exec.Command(j.cmd[0], j.cmd...)
	// run command
	fmt.Println(cmd.String())
	out, err := cmd.Output()
	if err != nil {
		log.Fatal(err)
	}
	if err = ioutil.WriteFile(j.res, out, 0644); err != nil {
		log.Fatal(err)
	}
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
func findFilesInDir(dir string, values []string) (ret []string) {
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
	var nWarm uint
	flag.UintVar(&nWarm, "warm", 1, "Number of warmup instructions in million")
	var nSim uint
	flag.UintVar(&nSim, "sim", 1, "Number of simulated instructions in million")

	var traceDir string = "../spec2017_traces/1B/"
	// flag.StringVar(&trace_dir, "d", "../spec2017_traces/1B/", "Specify the trace directory.")
	var traceSFlags searchFlags
	flag.Var(&traceSFlags, "t", "Specify strings that must match in the trace file.")

	var progDir string = "../bin/"
	var progSFlags searchFlags
	flag.Var(&progSFlags, "p", "Specify strings that must match in the binary")

	flag.Parse()

	// Get the correct trace files --------------------------------
	var traceFiles []string
	if len(traceSFlags) > 0 {
		// We search for files in a directory. Traces must be in compressed form
		traceFiles = append(traceFiles, ".xz", ".gz")
		traceFiles = findFilesInDir(traceDir, traceSFlags)
		fmt.Printf("Found %d traces matching the given constraints:\n", len(traceFiles))
		if len(traceFiles) == 0 {
			return
		}
		fmt.Println(traceFiles)
		fmt.Println("Do you want to run them? [y,yes]")
		var ans string
		fmt.Scanf("%s", &ans)
		if strings.Compare(ans, "y") != 0 && strings.Compare(ans, "yes") != 0 {
			return
		}
	} else {
		log.Fatal("No traces specified")
	}

	// Get the correct binaries --------------------------------
	var programms []string
	if len(progSFlags) > 0 {
		// Find mode:
		// In this mode we search for files in a directory
		programms = findFilesInDir(progDir, progSFlags)
		fmt.Printf("Found %d binaries matching the given constraints:\n", len(programms))
		if len(programms) == 0 {
			return
		}
		fmt.Println(programms)
		fmt.Println("Do you want to run them? [y,yes]")
		var ans string
		fmt.Scanf("%s", &ans)
		if strings.Compare(ans, "y") != 0 && strings.Compare(ans, "yes") != 0 {
			return
		}
	} else {
		log.Fatal("No binaries specified")
	}

	
	result_dir := fmt.Sprintf("../results_%dM", nSim)

	var jobs []job
	for _, trace := range traceFiles {
		for _, prog := range programms {

			var _cmd []string
			_cmd = append(_cmd, 
						fmt.Sprintf("./%s%s",progDir, prog),
						"-warmup_instructions",  fmt.Sprintf("%d000000",nWarm),
						"-simulation_instructions",  fmt.Sprintf("%d000000",nSim),
						"-traces",  fmt.Sprintf("%s%s",traceDir,trace) )
			
			_res := fmt.Sprintf("%s/%s-%s.txt",result_dir, trace, prog)
			jobs = append(jobs, job{cmd: _cmd, res: _res})
		}
	}
	// fmt.Println(jobs)

	fmt.Printf("Create %d jobs to run with %d warmup and %d sim inst. Do you want to start them? [y,yes]", len(jobs), nWarm, nSim)
	var ans string
	fmt.Scanf("%s", &ans)
	if strings.Compare(ans, "y") != 0 && strings.Compare(ans, "yes") != 0 {
		return
	}

	// Create a wait group
	wg := new(sync.WaitGroup) 
  wg.Add(len(jobs)) 

	for _,j := range jobs {
		go runCmd(j, wg)
	}
	// wait for all to be done
	wg.Wait()
}
