local project_name = "ygopen-srv"
local ygopen_dir   = "../ygopen"
local json_dir     = "../json-develop/include"

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

	include(ygopen_dir)

project(project_name)
	kind("ConsoleApp")
	flags("ExtraWarnings")
	defines("ASIO_STANDALONE")
	files({"../ocgcore-proto/generated/*.*", "../ocgcore-proto/*.hpp", "../ocgcore-proto/*.cpp", "*.hpp", "*.cpp"})
	includedirs (ygopen_dir)

	links("ygopen")

	configuration("windows or macosx")
		includedirs(json_dir)

	configuration("not windows")
		buildoptions({"-pedantic", "--std=c++17"})
		links({"dl", "pthread", "sqlite3", "protobuf"})
