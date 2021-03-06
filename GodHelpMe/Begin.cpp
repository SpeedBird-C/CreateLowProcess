#include <stdio.h>
#include <windows.h>
#include <Sddl.h>

BOOL CreateLowProcess(void)
{
	BOOL fRet;
	HANDLE hToken = NULL;
	HANDLE hNewToken = NULL;
	PSID pIntegritySid = NULL;
	TOKEN_MANDATORY_LABEL TIL = { 0 };
	PROCESS_INFORMATION ProcInfo = { 0 };
	STARTUPINFO StartupInfo = { 0 };
	WCHAR wszProcessName[MAX_PATH] = L"C:\\Program Files\\Notepad++\\notepad++.exe kek";
	//WCHAR wszProcessName[MAX_PATH] = L"C:\\Windows\\MBKS\\vuln4.exe C:\\Windows\\MBKS\\config_4";
	//WCHAR wszProcessName[MAX_PATH] = L"C:\\Users\\Elvin\\Documents\\????2.1\\vuln4.exe C:\\Users\\Elvin\\Documents\\????2.1\\config_5";
	WCHAR wszIntegritySid[20] = L"S-1-16-4096";
	//WCHAR wszIntegritySid[20] = L"S-1-16-12288";
	//WCHAR wszIntegritySid[20] = L"S-1-16-16384";

	fRet = OpenProcessToken(GetCurrentProcess(),
		TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY,
		&hToken);
	if (!fRet)
	{
		printf("[-] OpenProcessToken - failed %u\n", GetLastError());
		goto CleanExit;
	}
	fRet = DuplicateTokenEx(hToken, 0, NULL, SecurityImpersonation, TokenPrimary, &hNewToken);
	if (!fRet)
	{
		printf("[-] DuplicateTokenEx - failed %u\n", GetLastError());
		goto CleanExit;
	}
	fRet = ConvertStringSidToSidW(wszIntegritySid, &pIntegritySid);
	if (!fRet)
	{
		printf("[-] ConvertStringSidToSid - failed %u\n", GetLastError());
		goto CleanExit;
	}
	TIL.Label.Attributes = SE_GROUP_INTEGRITY;
	TIL.Label.Sid = pIntegritySid;
	fRet = SetTokenInformation(hNewToken, TokenIntegrityLevel, &TIL,
		sizeof(TOKEN_MANDATORY_LABEL) + GetLengthSid(pIntegritySid));
	if (!fRet)
	{
		printf("[-] SetTokenInformation - failed %u\n", GetLastError());
		goto CleanExit;
	}
	fRet = CreateProcessAsUserW(hNewToken,NULL, wszProcessName,
		NULL, NULL, FALSE, 0,
		NULL, NULL, (LPSTARTUPINFOW)&StartupInfo, &ProcInfo);
CleanExit:
	if (ProcInfo.hProcess != NULL)
	{
		CloseHandle(ProcInfo.hProcess);
	}
	if (ProcInfo.hThread != NULL)
	{
		CloseHandle(ProcInfo.hThread);
	}
	LocalFree(pIntegritySid);
	if (hNewToken != NULL)
	{
		CloseHandle(hNewToken);
	}
	if (hToken != NULL)
	{
		CloseHandle(hToken);
	}
	return fRet;
}

int main(void)
{
	CreateLowProcess();
	return 0;
}