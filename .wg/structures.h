typedef unsigned long DWORD;
typedef unsigned long long ULONG_PTR;
typedef unsigned short WORD;
typedef unsigned int UINT;

#define MOUSEEVENTF_MOVE	0x0001
#define MOUSEEVENTF_LEFTDOWN	0x0002
#define MOUSEEVENTF_LEFTUP	0x0004
#define MOUSEEVENTF_RIGHTDOWN	0x0008
#define MOUSEEVENTF_RIGHTUP	0x0010
#define MOUSEEVENTF_MIDDLEDOWN	0x0020
#define MOUSEEVENTF_MIDDLEUP	0x0040
#define MOUSEEVENTF_XDOWN	0x0080
#define MOUSEEVENTF_WHEEL	0x0800
#define MOUSEEVENTF_XUP	0x0100
#define MOUSEEVENTF_ABSOLUTE	0x8000
#define MOUSEEVENTF_HWHEEL	0x1000
#define INPUT_MOUSE 0
#define INPUT_KEYBOARD 1
#define INPUT_HARDWARE 2
#define KEYEVENTTF_KEYUP 0x0002


typedef struct
{
    int left;
    int top;
    int right;
    int bottom;

} RECT;

typedef struct
{
    int x;
    int y;

} POINT;

typedef struct {
    WORD wVk;
    WORD wScan;
    DWORD dwFlags;
    DWORD time;
    ULONG_PTR dwExtraInfo;
} KEYBDINPUT;

typedef struct {
  long dx;
  long dy;
  DWORD mouseData;
  DWORD dwFlags;
  DWORD time;
  ULONG_PTR dwExtraInfo;
} MOUSEINPUT;

typedef struct {
  DWORD uMsg;
  WORD  wParamL;
  WORD  wParamH;
} HARDWAREINPUT;

typedef struct {
    DWORD type;
    union {
        MOUSEINPUT mi;
        KEYBDINPUT ki;
        HARDWAREINPUT hi;
    };
    
} INPUT;