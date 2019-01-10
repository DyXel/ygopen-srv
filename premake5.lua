local project_name = "ygopen-srv"
local ygopen_dir   = "../ygopen"
local json_dir     = "../json-develop/include"

workspace(project_name)
	location("build")
	language("C++")
	objdir("obj")

	configurations({"Debug", "Release"})
	startproject(project_name)

	filter("action:vs*")
		characterset("ASCII")

	filter("configurations:Debug")
		symbols("On")
		defines("_DEBUG")
		targetdir("bin/debug")

	filter("configurations:Release")
		optimize("Speed")
		defines("_RELEASE")
		targetdir("bin/release")

	filter("system:windows")
		defines { "WIN32", "_WIN32", "NOMINMAX" }

	include(ygopen_dir)

project(project_name)
	kind("ConsoleApp")
	warnings("Extra")
	defines("ASIO_STANDALONE")
	files({"*.hpp", "*.cpp"})
	includedirs(ygopen_dir)

	links("ygopen")
	
	filter("system:windows or system:macosx")
		includedirs(json_dir)

	filter("system:not windows")
		buildoptions({"-pedantic", "--std=c++11"})
		links({"dl", "pthread", "sqlite3"})
