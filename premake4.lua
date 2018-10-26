local project_name = "ygopen-srv"
local ygopen_dir   = "../ygopen"

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

	include(ygopen_dir)

	project(project_name)
		kind("ConsoleApp")
		flags("ExtraWarnings")
		defines("ASIO_STANDALONE")
		files({"*.hpp", "*.cpp"})
		includedirs(ygopen_dir)
		links({"ygopen", "sqlite3"})

		configuration("not windows")
			buildoptions("-pedantic")
			links({"dl", "pthread"})
