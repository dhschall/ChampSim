package helper

import (
	"fmt"
	"io/ioutil"
	// "encoding/json"
)

type Location struct {
	File string `json:"file"`
	Col  uint   `json:"col"`
	Line uint   `json:"line"`
}

type BrAnalysisResults struct {
	ID       uint     `json:"id"`
	Category string   `json:"category"`
	Tree     string   `json:"tree"`
	Loc      Location `json:"loc"`
	IP       []uint   `json:"ip"`
}

func (ba *BrAnalysisResults) AddIP(_IP uint) {
	ba.IP = append(ba.IP, _IP)
}

func (ba BrAnalysisResults) Print() {
	fmt.Printf("ID: %d\n", ba.ID)
	fmt.Printf("Addresses: %d\n", ba.IP)
	if (ba.Loc != Location{}) {
		fmt.Printf("Src location: %s:%d\n", ba.Loc.File, ba.Loc.Line)
	}
}

func (ba BrAnalysisResults) WriteTree() {
	message := []byte(ba.Tree)
	if err := ioutil.WriteFile("graph.dot", message, 0644); err != nil {
		fmt.Print(err)
	}
}
