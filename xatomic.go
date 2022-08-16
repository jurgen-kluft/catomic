package main

import (
	xatomic "github.com/jurgen-kluft/xatomic/package"
)

func main() {
	xcode.Init()
	xcode.Generate(xatomic.GetPackage())
}
