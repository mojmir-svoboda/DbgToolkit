-- This is script for PreMake. It generates one project per platform and Visual studio version
-- The script needs variable "projectIncludeFunction" - here is loaded script "project_include.lua"
-- The script needs all variables needed by "project_include.lua"

project "flogging"

	is3rdProject = true; -- must be before projectIncludeFunction()

	projectIncludeFunction() -- some common settings (we can't use "dofile" here - it is inefficient and it runs in different folder)
	
	uuid("ADE7CE18-844A-11D0-8D11-00A0C91BC942") -- this is GUID in project file. It must be generated manually for each new project

	flags("NoPCH")
	
	buildoptions("/EHsc")

	configuration { "*DLL" }
		defines {"TRACE_DLL"}
		defines {"PROFILE_DLL"}

	configuration { "not *DLL" }
		defines {"TRACE_STATIC"}
		defines {"PROFILE_STATIC"}
	
	configuration {}

	defines {"TRACE_ENABLED"}
	defines {"PROFILE_ENABLED"}
	defines {"TRACE_CONTEXTS_INCLUDE=<Shared/Include/TraceContexts.h>"}

	includedirs {
		os.getenv("WH_CODE_ROOT").."/externals_code/flogging/src/"
	}

	if isXbox == true then
		includedirs {
			sdkRootPath.."/STLPORT/stlport",
			"$(XDKInstallDir)/include/xbox",
			sdkRootPath.."/boost",
			sdkRootPath.."",
			externalsCodePath,
			externalsCodePath.."/PhysX2.8.4/Include"
		}

		libdirs {"$(XDKInstallDir)/lib/xbox"}
	else
		includedirs {
			sdkRootPath.."/STLPORT/stlport",
			sdkRootPath.."/DXSDK/Include",
			"$(VCInstallDir)include",
			"$(VCInstallDir)atlmfc/include",
			"$(WindowsSdkDir)/include",
			"$(FrameworkSDKDir)/include",
			enginePath.."/CryEngine/CryCommon", -- add engine for all projects (because nested includes are without path)
			enginePath.."/CryEngine/CryAction", -- add engine for all projects (because nested includes are without path)
			enginePath.."/CryEngine",
			sdkRootPath.."/boost",
			sdkRootPath,
			externalsCodePath,
			externalsCodePath.."/PhysX2.8.4/Include"
		}
	end

    files { "**.cpp" }
    files { "**.h" }

	if isXbox == true then
		linkoptions {"/ignore:4089 /dll /entry:_DllMainCRTStartup /include:XboxKrnlBuildNumber "}
		links {"xnet" }
	else
	end

