#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <iostream>

#define ERROR_EXIT_STATUS_CODE -1

using std::cout;
using std::endl;

bool enableSeDebugPrivilege() {
  HANDLE currentProcess = GetCurrentProcess();
  HANDLE tokenHandle;
  if (!OpenProcessToken(currentProcess, TOKEN_ALL_ACCESS, &tokenHandle)) {
    return false;
  }
  LUID luid;
  if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &luid)) {
    return false;
  }
  TOKEN_PRIVILEGES newState;
  newState.PrivilegeCount = 1;
  newState.Privileges[0].Attributes = true;
  newState.Privileges[0].Luid = luid;
  if (!AdjustTokenPrivileges(tokenHandle, false, &newState, sizeof(TOKEN_PRIVILEGES), nullptr,
                             0)) {
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    cout << "Usage: " << argv[0] << " PATH/TO/DLL PID" << endl;
    exit(ERROR_EXIT_STATUS_CODE);
  }
  int pid = atoi(argv[2]);
  char *dllPath = argv[1];
  if (pid <= 0) {
    cout << "Make sure PID is a valid positive integer" << endl;
    exit(ERROR_EXIT_STATUS_CODE);
  }
  cout << "ASDDASD" << endl;
  if (!enableSeDebugPrivilege()) {
    cout << "Changing privileges failed" << endl;
    exit(ERROR_EXIT_STATUS_CODE);
  }

  HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
  if (!processHandle) {
    cout << "Creating process handle failed: " << GetLastError() << endl;
    exit(ERROR_EXIT_STATUS_CODE);
  }

  LPVOID remoteProcessBuf =
      VirtualAllocEx(processHandle, nullptr, strlen(dllPath) + 1,
                     MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!remoteProcessBuf) {
    cout << "Remote memory allcation failed" << endl;
    exit(ERROR_EXIT_STATUS_CODE);
  }

  if (!WriteProcessMemory(processHandle, remoteProcessBuf, dllPath,
                          strlen(dllPath) + 1, nullptr)) {
    cout << "Copying library path to remote process failed" << endl;
    exit(ERROR_EXIT_STATUS_CODE);
  }

  CreateRemoteThread(processHandle, nullptr, 0,
                     reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA),
                     remoteProcessBuf, 0, nullptr);
}
