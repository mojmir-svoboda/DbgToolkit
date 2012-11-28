void StartTraceServer()
{
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.dwFlags = STARTF_USESHOWWINDOW;
	startupInfo.wShowWindow = SW_SHOWMINIMIZED;

	PROCESS_INFORMATION processInformation;
	ZeroMemory(&processInformation, sizeof(processInformation));

	if(CreateProcess("..\\bin64\\trace_server_launcher.exe", "..\\bin64\\trace_server_launcher.exe ..\\bin64\\trace_server.exe ..\\bin64\\trace_server.run.exe -n -q", nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &processInformation))
	{
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
	}
	else
	{
		LPVOID lpMsgBuf;
		DWORD const dw = GetLastError(); 
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf, 0, NULL );
		LPVOID lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + 256) * sizeof(TCHAR)); 
		StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR),
			TEXT("Startup of trace_server.exe failed with error %08x: %s"), dw, lpMsgBuf);

		OutputDebugStringA((LPCTSTR)lpDisplayBuf);
		int size = 0;
		char * pwd = 0;
		OutputDebugStringA(_getcwd(pwd, size));

		LocalFree(lpMsgBuf);
		LocalFree(lpDisplayBuf);
	}
