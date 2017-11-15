package xatomic

import (
	"github.com/jurgen-kluft/xcode/denv"
	"github.com/jurgen-kluft/xbase/package"
	"github.com/jurgen-kluft/xentry/package"
	"github.com/jurgen-kluft/xunittest/package"
)

// GetPackage returns the package object of 'xatomic'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := xunittest.GetPackage()
	entrypkg := xentry.GetPackage()
	basepkg := xbase.GetPackage()

	// The main (xatomic) package
	mainpkg := denv.NewPackage("xatomic")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(entrypkg)
	mainpkg.AddPackage(basepkg)

	// 'xatomic' library
	mainlib := denv.SetupDefaultCppLibProject("xatomic", "github.com\\jurgen-kluft\\xatomic")
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())

	// 'xatomic' unittest project
	maintest := denv.SetupDefaultCppTestProject("xatomic_test", "github.com\\jurgen-kluft\\xatomic")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, entrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)

	return mainpkg
}
