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
	mainpkg := denv.NewPackage("catomic")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(basepkg)

	// 'catomic' library
	mainlib := denv.SetupDefaultCppLibProject("catomic", "github.com\\jurgen-kluft\\catomic")
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())

	// 'catomic' unittest project
	maintest := denv.SetupDefaultCppTestProject("catomic_test", "github.com\\jurgen-kluft\\catomic")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, basepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)

	return mainpkg
}
