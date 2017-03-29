
	b2.exe address-model=64 threading=multi toolset=msvc-14.0 link=static

project-config.jam:
	import option ; 
	 
	using msvc : 14.0 : "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.10.25017\bin\HostX64\x64\cl.exe"; 
		
	option.set keep-going : false ; 
		 
