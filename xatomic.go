package main

import (
	"github.com/jurgen-kluft/xatomic/package"
	"github.com/jurgen-kluft/xcode"
)

func main() {
	xcode.Generate(xatomic.GetPackage())
}
