local project_name = "ygopen-srv"

solution(project_name)
	location(".")
	language("C++")
	objdir("obj")

	configurations({"Debug", "Release"})

	configuration("Debug")
		flags("Symbols")
		defines("_DEBUG")
		targetdir("bin/debug")

	configuration("Release")
		flags("OptimizeSpeed")
		defines("_RELEASE")
		targetdir("bin/release")

	configuration("windows")
		defines({ "WIN32", "_WIN32", "NOMINMAX" })

project(project_name)
	kind("ConsoleApp")
	flags("ExtraWarnings")
	defines("ASIO_STANDALONE")
	files({"*.hpp", "*.cpp"})
	includedirs(".")

	configuration("not windows")
		buildoptions({"-pedantic", "--std=c++11"})
		links({"dl", "pthread", "sqlite3"})
