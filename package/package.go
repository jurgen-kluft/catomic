package catomic

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'catomic'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := cunittest.GetPackage()
	basepkg := cbase.GetPackage()

	// The main (catomic) package
	mainpkg := denv.NewPackage("github.com\\jurgen-kluft", "catomic")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(basepkg)

	// 'catomic' library
	mainlib := denv.SetupCppLibProject(mainpkg, "catomic")
	mainlib.AddDependencies(basepkg.GetMainLib()...)

	// 'catomic' unittest project
	maintest := denv.SetupCppTestProject(mainpkg, "catomic_test")
	maintest.AddDependencies(unittestpkg.GetMainLib()...)
	maintest.AddDependencies(basepkg.GetMainLib()...)
	maintest.AddDependency(mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)

	return mainpkg
}
