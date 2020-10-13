package main

import (
	xatomic "github.com/jurgen-kluft/xatomic/package"
	"github.com/jurgen-kluft/xcode"
)

func main() {
	xcode.Init()
	xcode.Generate(xatomic.GetPackage())
}
