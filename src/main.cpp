#pragma comment(lib, "OneCore.lib")

#define UNICODE

#include <cstdio>
#include <string>
#include <Windows.h>
#include <vector>
#include <sstream>
#include <intrin.h>
// #include <base64.h>

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#define WC_COMBOBOX (TEXT("COMBOBOX"))
#define WC_BUTTON (TEXT("BUTTON"))

TCHAR com_strings[256][7];
HWND dropDownList;
HWND button;
unsigned char comSelected;
short portNumber = -1;

union UInteger32 {
    unsigned int integer;
    unsigned char bytes[4];
};

void ExecuteBytecode(const char *byteCode, unsigned long length) {
    MessageBox(GetActiveWindow(), TEXT("Not implemented"), TEXT("Remote Access Control"), MB_ICONERROR);
}

#define __LCONCAT(x, y) x##y
#define __LCONCAT2(x, y) __LCONCAT(x, y)

#define READ_RELIABLE(com, bytes, len) DWORD __LCONCAT2(__read_bytes_, __LINE__) = 0;     \
                                                                                          \
while (__LCONCAT2(__read_bytes_, __LINE__) < (len)) {                                     \
    DWORD __LCONCAT2(__prev_read_bytes_, __LINE__) = __LCONCAT2(__read_bytes_, __LINE__); \
    ReadFile(com, bytes, len, &__LCONCAT2(__read_bytes_, __LINE__), NULL);                \
    __LCONCAT2(__read_bytes_, __LINE__) += __LCONCAT2(__prev_read_bytes_, __LINE__);      \
}                                                                                         \

#define WRITE_RELIABLE(com, bytes, len) DWORD __LCONCAT2(__write_bytes_, __LINE__) = 0;     \
                                                                                            \
while (__LCONCAT2(__write_bytes_, __LINE__) < (len)) {                                      \
    DWORD __LCONCAT2(__prev_write_bytes_, __LINE__) = __LCONCAT2(__write_bytes_, __LINE__); \
    WriteFile(com, bytes, len, &__LCONCAT2(__write_bytes_, __LINE__), NULL);                \
    __LCONCAT2(__write_bytes_, __LINE__) += __LCONCAT2(__prev_write_bytes_, __LINE__);      \
}                                                                                           \

int StartPort(unsigned char portNumber) {
    std::string str = std::to_string(portNumber);
    char portName[7] = { 0 };
    portName[0] = 'C';
    portName[1] = 'O';
    portName[2] = 'M';
    portName[3] = str[0];
    portName[4] = str[1];

    if (str.size() >= 2) {
        portName[5] = str[2];
    }

    unsigned int attempts = 1024;
    HANDLE com = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    while (com == INVALID_HANDLE_VALUE) {
        if (--attempts == 0) {
            MessageBox(GetActiveWindow(), TEXT("Failed to open port"), TEXT("Remote Access Control"), MB_ICONERROR);
            return 1;
        }

        com = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    BOOL status = GetCommState(com, &dcbSerialParams);

    if (status == FALSE) {
        MessageBox(GetActiveWindow(), TEXT("Failed to get port state"), TEXT("Remote Access Control"), MB_ICONERROR);
        CloseHandle(com);
        return 1;
    }

    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    status = SetCommState(com, &dcbSerialParams);

    if (status == FALSE) {
        MessageBox(GetActiveWindow(), TEXT("Failed to set port state"), TEXT("Remote Access Control"), MB_ICONERROR);
        CloseHandle(com);
        return 1;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (SetCommTimeouts(com, &timeouts) == FALSE) {
        MessageBox(GetActiveWindow(), TEXT("Failed to set timeouts"), TEXT("Remote Access Control"), MB_ICONERROR);
        CloseHandle(com);
        return 1;
    }

    DWORD eventMask;
    UInteger32 length;

    READ_RELIABLE(com, &length.bytes, sizeof(length.integer))
    // _base64_init();

    // unsigned char *bytes = base64_decode(lengthBuffer, 4, nullptr);
    length.integer = ntohl(length.integer);

    if (length.integer < 6) {
        std::stringstream ss;
        ss << length.integer;
        MessageBoxA(GetActiveWindow(), ss.str().c_str(), "Remote Access Control", MB_ICONINFORMATION);
        MessageBox(GetActiveWindow(), TEXT("Invalid data from port"), TEXT("Remote Access Control"), MB_ICONERROR);
        CloseHandle(com);
        return 1;
    }

    char signature[7] = { 0 };

    READ_RELIABLE(com, &signature, sizeof(char) * 6)

    if (memcmp(signature, "RACv1w", 6) != 0) {
        MessageBox(GetActiveWindow(), TEXT("Invalid signature from port"), TEXT("Remote Access Control"), MB_ICONERROR);
        CloseHandle(com);
        return 1;
    }

    if (length.integer > 6) {
        MessageBox(GetActiveWindow(), TEXT("Invalid port broadcast message. May be it is already connected?"), TEXT("Remote Access Control"), MB_ICONERROR);
        return 1;
    }

    unsigned char connectedMessage[10];
    *((unsigned int *)connectedMessage) = htonl(sizeof(connectedMessage));
    std::memcpy(connectedMessage + 4, "RACv1x", sizeof(char) * 6);
    WRITE_RELIABLE(com, connectedMessage, sizeof(connectedMessage));
    MessageBox(GetActiveWindow(), TEXT("Connected to port"), TEXT("Remote Access Control"), MB_ICONINFORMATION);

    CloseHandle(com);
    // _base64_destroy();
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            dropDownList = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 100, 10, 200, 190, hwnd, NULL, HINST_THISCOMPONENT, NULL);
            SendMessage(dropDownList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)TEXT("(none)"));

            for (unsigned char i = 0; i < 255; i++) {
                if (com_strings[i][0] == 0) {
                    break;
                }

                SendMessage(dropDownList, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)com_strings[i]);
            }

            SendMessage(dropDownList, (UINT)CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
            button = CreateWindow(WC_BUTTON, TEXT("Continue"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_OVERLAPPED | BS_DEFPUSHBUTTON, 150, 110, 100, 40, hwnd, NULL, HINST_THISCOMPONENT, NULL);
            EnableWindow(button, false);
            return 0;
        }
        case WM_CLOSE: {
            DestroyWindow(hwnd);
            return 0;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_COMMAND: {
            switch (HIWORD(wParam)) {
                case CBN_SELCHANGE: {
                    int index = SendMessage(dropDownList, CB_GETCURSEL, NULL, NULL);

                    if (index == CB_ERR) {
                        return 0;
                    }

                    comSelected = index;
                    int len = SendMessage(dropDownList, CB_GETLBTEXTLEN, (WPARAM)index, NULL);

                    if (len == CB_ERR) {
                        return 0;
                    }

                    std::vector<wchar_t> buffer(len + 1);
                    SendMessageW(dropDownList, CB_GETLBTEXT, (WPARAM)index, (LPARAM)buffer.data());

                    if (buffer[0] == '(') { // (none)
                        EnableWindow(button, false);
                    } else {
                        EnableWindow(button, true);
                    }
                    return 0;
                }

                case BN_CLICKED: {
                    if (comSelected == 0) { // (none)
                        return 0;
                    }

                    std::string str;

                    for (unsigned char i = 3; com_strings[comSelected - 1][i] != 0; i++) {
                        str.push_back(com_strings[comSelected - 1][i]);
                    }

                    str.push_back(0);
                    portNumber = std::strtoull(str.c_str(), nullptr, 10);
                    DestroyWindow(hwnd);
                    return 0;
                }

                default: {
                    break;
                }
            }

            break;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    unsigned long *ports = new unsigned long[1];
    unsigned long foundPorts;

    switch (GetCommPorts(ports, 1 * sizeof(unsigned char), &foundPorts)) {
        case ERROR_SUCCESS: {
            break;
        };
        case ERROR_MORE_DATA: {
            ports = new unsigned long[foundPorts];

            switch (GetCommPorts(ports, foundPorts * sizeof(unsigned long), &foundPorts)) {
                case ERROR_SUCCESS: {
                    break;
                };
                case ERROR_MORE_DATA:
                case ERROR_FILE_NOT_FOUND: {
                    MessageBox(GetActiveWindow(), TEXT("Error while scanning for COM ports"), TEXT("Remote Access Software"), MB_ICONERROR);
                    return -1;
                }
            }

            break;
        };
        case ERROR_FILE_NOT_FOUND: {
            MessageBox(GetActiveWindow(), TEXT("No available COM ports"), TEXT("Remote Access Software"), MB_ICONERROR);
            return 1;
        }
    }

    memset(&com_strings, 0, sizeof(com_strings));

    for (unsigned long i = 0; i < foundPorts; i++) {
        com_strings[i][0] = TEXT('C');
        com_strings[i][1] = TEXT('O');
        com_strings[i][2] = TEXT('M');
        std::string strpp = std::to_string(ports[i]);
        com_strings[i][3] = strpp[0];
        com_strings[i][4] = strpp[1];

        if (strpp[2] != 0) {
            com_strings[i][5] = strpp[2];
        }
    }

    const wchar_t CLASS_NAME[] = L"by.ndtp.remote_access_control.software";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    ATOM value = RegisterClass(&wc);

    if (value == 0) {
        MessageBox(GetActiveWindow(), TEXT("Register class failed"), TEXT("Remote Access Software"), MB_ICONERROR);
    }

    HWND window = CreateWindowEx(0, (LPCWSTR)value, L"Remote Access Software", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 200, NULL, NULL, hInstance, NULL);

    if (window == nullptr) {
        DWORD err = GetLastError();
        wchar_t msg[1024] = { 0 };
        swprintf_s(msg, 1024, L"Failed to create prompt window: %d", err);
        MessageBox(GetActiveWindow(), msg, TEXT("Remote Access Software"), MB_ICONERROR);
        return -1;
    }

    ShowWindow(window, nCmdShow);
    BOOL end = false;
    MSG msg = {};

    while (end = GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (portNumber == -1) {
        return 0;
    }

    return StartPort(portNumber);
}
