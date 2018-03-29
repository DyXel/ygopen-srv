solution("ygopen-srv")
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

	project("ygopen-srv")
		kind("ConsoleApp")
		flags("ExtraWarnings")
		defines("ASIO_STANDALONE")
		files({"*.hpp", "*.cpp"})
		--links({"sqlite3"})

		configuration("not windows")
			buildoptions("-pedantic")
			links("pthread")
			includedirs("/usr/include/SDL2")
