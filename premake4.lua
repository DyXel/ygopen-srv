local project_name = "ygopen-srv"
local ygopen_dir   = "../ygopen"
local sqlite_dir = "../sqlite3"
local json_dir = "../json-develop/include"

solution(project_name)
	location(".")
	language("C++")
	objdir("obj")
	
	configurations({"Debug", "Release"})
	
	configuration("windows")
        defines { "WIN32", "_WIN32", "NOMINMAX" }

	configuration("Debug")
        symbols("On")
		defines("_DEBUG")
		targetdir("bin/debug")

	configuration("Release")
        optimize("Speed")
		defines("_RELEASE")
		targetdir("bin/release")

	include(ygopen_dir)
	configuration "windows"
		include(sqlite_dir)
		
	configuration "vs*"
        characterset("MBCS")

	project(project_name)
		kind("ConsoleApp")
		flags("ExtraWarnings")
		defines("ASIO_STANDALONE")
		files({"*.hpp", "*.cpp"})
		includedirs(ygopen_dir)
		configuration "windows"
			includedirs (sqlite_dir)
			includedirs (json_dir)
		links({"ygopen", "sqlite3"})

		configuration("not windows")
			buildoptions("-pedantic")
			links({"dl", "pthread"})
			
		configuration "vs*"
			characterset("MBCS")
