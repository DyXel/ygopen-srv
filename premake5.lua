local project_name = "ygopen-srv"

workspace(project_name)
	location("build")
	language("C++")
	objdir("obj")

	configurations({"Debug", "Release"})
	startproject(project_name)

	filter("action:vs*")
		characterset("ASCII")
		platforms { "x86", "x86_64" }
		filter "platforms:x86"
			architecture "x32"
		filter "platforms:x64"
			architecture "x64"

	filter("configurations:Debug")
		symbols("On")
		defines("_DEBUG")
		targetdir("bin/debug")
		runtime "Debug"

	filter("configurations:Release")
		optimize("Speed")
		defines("_RELEASE")
		targetdir("bin/release")

	filter("system:windows")
		defines { "WIN32", "_WIN32", "NOMINMAX" }

project(project_name)
	kind("ConsoleApp")
	warnings("Extra")
	defines("ASIO_STANDALONE")
	files({"**.hpp", "**.cpp"})
	staticruntime "on"

	filter("system:not windows")
		buildoptions({"-pedantic", "--std=c++17"})
		links({"dl", "pthread", "sqlite3"})

local function vcpkgStaticTriplet(prj)
	premake.w('<VcpkgTriplet Condition="\'$(Platform)\'==\'Win32\'">x86-windows-static</VcpkgTriplet>')
	premake.w('<VcpkgTriplet Condition="\'$(Platform)\'==\'x64\'">x64-windows-static</VcpkgTriplet>')
end
		
require('vstudio')
		
premake.override(premake.vstudio.vc2010.elements, "globals", function(base, prj)
	local calls = base(prj)
	table.insertafter(calls, premake.vstudio.vc2010.targetPlatformVersionGlobal, vcpkgStaticTriplet)
	return calls
end) 
