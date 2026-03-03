typedef struct {
    short winapi_code;
    short linux_code;
} KeyMapping;

const KeyMapping key_mappings[] = {
    // Буквенно-цифровые
    { 0x41, 0x1E },  // A
    { 0x42, 0x30 },  // B
    { 0x43, 0x2E },  // C
    { 0x44, 0x20 },  // D
    { 0x45, 0x12 },  // E
    { 0x46, 0x21 },  // F
    { 0x47, 0x22 },  // G
    { 0x48, 0x23 },  // H
    { 0x49, 0x17 },  // I
    { 0x4A, 0x24 },  // J
    { 0x4B, 0x25 },  // K
    { 0x4C, 0x26 },  // L
    { 0x4D, 0x32 },  // M
    { 0x4E, 0x31 },  // N
    { 0x4F, 0x18 },  // O
    { 0x50, 0x19 },  // P
    { 0x51, 0x10 },  // Q
    { 0x52, 0x13 },  // R
    { 0x53, 0x1F },  // S
    { 0x54, 0x14 },  // T
    { 0x55, 0x16 },  // U
    { 0x56, 0x2F },  // V
    { 0x57, 0x11 },  // W
    { 0x58, 0x2D },  // X
    { 0x59, 0x15 },  // Y
    { 0x5A, 0x2C },  // Z
    
    // Цифры
    { 0x30, 0x0B },  // 0
    { 0x31, 0x02 },  // 1
    { 0x32, 0x03 },  // 2
    { 0x33, 0x04 },  // 3
    { 0x34, 0x05 },  // 4
    { 0x35, 0x06 },  // 5
    { 0x36, 0x07 },  // 6
    { 0x37, 0x08 },  // 7
    { 0x38, 0x09 },  // 8
    { 0x39, 0x0A },  // 9
    
    // Функциональные клавиши
    { 0x70, 0x3B },  // F1
    { 0x71, 0x3C },  // F2
    { 0x72, 0x3D },  // F3
    { 0x73, 0x3E },  // F4
    { 0x74, 0x3F },  // F5
    { 0x75, 0x40 },  // F6
    { 0x76, 0x41 },  // F7
    { 0x77, 0x42 },  // F8
    { 0x78, 0x43 },  // F9
    { 0x79, 0x44 },  // F10
    { 0x7A, 0x57 },  // F11
    { 0x7B, 0x58 },  // F12
    
    // Управление
    { 0x08, 0x0E },  // Backspace
    { 0x09, 0x0F },  // Tab
    { 0x0D, 0x1C },  // Enter
    { 0xA0, 0x2A },  // Left Shift
    { 0xA1, 0x36 },  // Right Shift
    { 0xA2, 0x1D },  // Left Ctrl
    { 0xA3, 0x61 },  // Right Ctrl
    { 0xA4, 0x38 },  // Left Alt
    { 0xA5, 0x64 },  // Right Alt
    { 0x20, 0x39 },  // Space
    { 0x14, 0x3A },  // Caps Lock
    { 0x1B, 0x01 },  // Escape
    
    // Навигация
    { 0x2D, 0x6E },  // Insert
    { 0x2E, 0x6F },  // Delete
    { 0x24, 0x66 },  // Home
    { 0x23, 0x6B },  // End
    { 0x21, 0x68 },  // Page Up
    { 0x22, 0x6D },  // Page Down
    { 0x25, 0x69 },  // Left Arrow
    { 0x26, 0x67 },  // Up Arrow
    { 0x27, 0x6A },  // Right Arrow
    { 0x28, 0x6C },  // Down Arrow
    
    // Цифровой блок
    { 0x90, 0x45 },  // Num Lock
    { 0x60, 0x52 },  // Numpad 0
    { 0x61, 0x4F },  // Numpad 1
    { 0x62, 0x50 },  // Numpad 2
    { 0x63, 0x51 },  // Numpad 3
    { 0x64, 0x4B },  // Numpad 4
    { 0x65, 0x4C },  // Numpad 5
    { 0x66, 0x4D },  // Numpad 6
    { 0x67, 0x47 },  // Numpad 7
    { 0x68, 0x48 },  // Numpad 8
    { 0x69, 0x49 },  // Numpad 9
    { 0x6A, 0x37 },  // Numpad Multiply (*)
    { 0x6B, 0x4E },  // Numpad Add (+)
    { 0x6D, 0x4A },  // Numpad Subtract (-)
    { 0x6E, 0x53 },  // Numpad Decimal (.)
    { 0x6F, 0x62 },  // Numpad Divide (/)
    
    // Мультимедиа
    { 0xAF, 0x73 },  // Volume Up
    { 0xAE, 0x72 },  // Volume Down
    { 0xAD, 0x71 },  // Volume Mute
    { 0xB3, 0xA8 },  // Play/Pause
    { 0xB2, 0x80 },  // Stop
    { 0xB0, 0xA3 },  // Next Track
    { 0xB1, 0xA5 },  // Prev Track
};


// Кнопки мыши (0x01-0x06)
#define VK_LBUTTON        0x01  // Левая кнопка мыши
#define VK_RBUTTON        0x02  // Правая кнопка мыши
#define VK_CANCEL         0x03  // Control-break обработка
#define VK_MBUTTON        0x04  // Средняя кнопка мыши
#define VK_XBUTTON1       0x05  // X1 кнопка мыши
#define VK_XBUTTON2       0x06  // X2 кнопка мыши

// Управляющие клавиши (0x08-0x0F)
#define VK_BACK           0x08  // Backspace
#define VK_TAB            0x09  // Tab
#define VK_CLEAR          0x0C  // Clear
#define VK_RETURN         0x0D  // Enter

// Модификаторы (0x10-0x12)
#define VK_SHIFT          0x10  // Shift (любой)
#define VK_CONTROL        0x11  // Ctrl (любой)
#define VK_MENU           0x12  // Alt (любой)

// Специальные клавиши (0x13-0x1F)
#define VK_PAUSE          0x13  // Pause
#define VK_CAPITAL        0x14  // Caps Lock
#define VK_KANA           0x15  // IME Kana mode
#define VK_HANGUEL        0x15  // IME Hanguel mode (для совместимости)
#define VK_HANGUL         0x15  // IME Hangul mode
#define VK_IME_ON         0x16  // IME On
#define VK_JUNJA          0x17  // IME Junja mode
#define VK_FINAL          0x18  // IME Final mode
#define VK_HANJA          0x19  // IME Hanja mode
#define VK_KANJI          0x19  // IME Kanji mode
#define VK_IME_OFF        0x1A  // IME Off
#define VK_ESCAPE         0x1B  // Esc
#define VK_CONVERT        0x1C  // IME Convert
#define VK_NONCONVERT     0x1D  // IME Nonconvert
#define VK_ACCEPT         0x1E  // IME Accept
#define VK_MODECHANGE     0x1F  // IME Mode change

// Пробел (0x20)
#define VK_SPACE          0x20  // Пробел

// Навигация (0x21-0x2E)
#define VK_PRIOR          0x21  // Page Up
#define VK_NEXT           0x22  // Page Down
#define VK_END            0x23  // End
#define VK_HOME           0x24  // Home
#define VK_LEFT           0x25  // Стрелка влево
#define VK_UP             0x26  // Стрелка вверх
#define VK_RIGHT          0x27  // Стрелка вправо
#define VK_DOWN           0x28  // Стрелка вниз
#define VK_SELECT         0x29  // Select
#define VK_PRINT          0x2A  // Print
#define VK_EXECUTE        0x2B  // Execute
#define VK_SNAPSHOT       0x2C  // Print Screen
#define VK_INSERT         0x2D  // Insert
#define VK_DELETE         0x2E  // Delete
#define VK_HELP           0x2F  // Help


// Левая клавиша Windows (0x5B-0x5F)
#define VK_LWIN           0x5B  // Левая клавиша Windows
#define VK_RWIN           0x5C  // Правая клавиша Windows
#define VK_APPS           0x5D  // Клавиша приложений (контекстное меню)
#define VK_SLEEP          0x5F  // Sleep

// Цифровой блок (0x60-0x6F)
#define VK_NUMPAD0        0x60  // Numpad 0
#define VK_NUMPAD1        0x61  // Numpad 1
#define VK_NUMPAD2        0x62  // Numpad 2
#define VK_NUMPAD3        0x63  // Numpad 3
#define VK_NUMPAD4        0x64  // Numpad 4
#define VK_NUMPAD5        0x65  // Numpad 5
#define VK_NUMPAD6        0x66  // Numpad 6
#define VK_NUMPAD7        0x67  // Numpad 7
#define VK_NUMPAD8        0x68  // Numpad 8
#define VK_NUMPAD9        0x69  // Numpad 9
#define VK_MULTIPLY       0x6A  // Numpad *
#define VK_ADD            0x6B  // Numpad +
#define VK_SEPARATOR      0x6C  // Numpad Separator
#define VK_SUBTRACT       0x6D  // Numpad -
#define VK_DECIMAL        0x6E  // Numpad .
#define VK_DIVIDE         0x6F  // Numpad /

// Функциональные клавиши (0x70-0x7F)
#define VK_F1             0x70  // F1
#define VK_F2             0x71  // F2
#define VK_F3             0x72  // F3
#define VK_F4             0x73  // F4
#define VK_F5             0x74  // F5
#define VK_F6             0x75  // F6
#define VK_F7             0x76  // F7
#define VK_F8             0x77  // F8
#define VK_F9             0x78  // F9
#define VK_F10            0x79  // F10
#define VK_F11            0x7A  // F11
#define VK_F12            0x7B  // F12
#define VK_F13            0x7C  // F13
#define VK_F14            0x7D  // F14
#define VK_F15            0x7E  // F15
#define VK_F16            0x7F  // F16
#define VK_F17            0x80  // F17
#define VK_F18            0x81  // F18
#define VK_F19            0x82  // F19
#define VK_F20            0x83  // F20
#define VK_F21            0x84  // F21
#define VK_F22            0x85  // F22
#define VK_F23            0x86  // F23
#define VK_F24            0x87  // F24

// Клавиши состояния (0x90-0x92)
#define VK_NUMLOCK        0x90  // Num Lock
#define VK_SCROLL         0x91  // Scroll Lock

// OEM специальные клавиши (0x92-0xBF)
#define VK_OEM_FJ_JISHO   0x92  // Fujitsu/OASYS
#define VK_OEM_FJ_MASSHOU 0x93  // Fujitsu/OASYS
#define VK_OEM_FJ_TOUROKU 0x94  // Fujitsu/OASYS
#define VK_OEM_FJ_LOYA    0x95  // Fujitsu/OASYS
#define VK_OEM_FJ_ROYA    0x96  // Fujitsu/OASYS

// Левая/правая версии модификаторов (0xA0-0xA5)
#define VK_LSHIFT         0xA0  // Левый Shift
#define VK_RSHIFT         0xA1  // Правый Shift
#define VK_LCONTROL       0xA2  // Левый Ctrl
#define VK_RCONTROL       0xA3  // Правый Ctrl
#define VK_LMENU          0xA4  // Левый Alt
#define VK_RMENU          0xA5  // Правый Alt

// Мультимедиа клавиши (0xAD-0xB9)
#define VK_BROWSER_BACK        0xA6  // Browser Back
#define VK_BROWSER_FORWARD     0xA7  // Browser Forward
#define VK_BROWSER_REFRESH     0xA8  // Browser Refresh
#define VK_BROWSER_STOP        0xA9  // Browser Stop
#define VK_BROWSER_SEARCH      0xAA  // Browser Search
#define VK_BROWSER_FAVORITES   0xAB  // Browser Favorites
#define VK_BROWSER_HOME        0xAC  // Browser Home
#define VK_VOLUME_MUTE         0xAD  // Volume Mute
#define VK_VOLUME_DOWN         0xAE  // Volume Down
#define VK_VOLUME_UP           0xAF  // Volume Up
#define VK_MEDIA_NEXT_TRACK    0xB0  // Next Track
#define VK_MEDIA_PREV_TRACK    0xB1  // Previous Track
#define VK_MEDIA_STOP          0xB2  // Stop
#define VK_MEDIA_PLAY_PAUSE    0xB3  // Play/Pause
#define VK_LAUNCH_MAIL         0xB4  // Launch Mail
#define VK_LAUNCH_MEDIA_SELECT 0xB5  // Select Media
#define VK_LAUNCH_APP1         0xB6  // Launch App 1
#define VK_LAUNCH_APP2         0xB7  // Launch App 2

// OEM дополнительные клавиши (0xBA-0xC0)
#define VK_OEM_1          0xBA  // ;: для US клавиатуры
#define VK_OEM_PLUS       0xBB  // =+ для любой клавиатуры
#define VK_OEM_COMMA      0xBC  // ,< для любой клавиатуры
#define VK_OEM_MINUS      0xBD  // -_ для любой клавиатуры
#define VK_OEM_PERIOD     0xBE  // .> для любой клавиатуры
#define VK_OEM_2          0xBF  // /? для US клавиатуры
#define VK_OEM_3          0xC0  // `~ для US клавиатуры

// Игровые клавиши (0xC1-0xDA)
#define VK_GAMEPAD_A          0xC3  // Gamepad A
#define VK_GAMEPAD_B          0xC4  // Gamepad B
#define VK_GAMEPAD_X          0xC5  // Gamepad X
#define VK_GAMEPAD_Y          0xC6  // Gamepad Y
#define VK_GAMEPAD_RIGHT_SHOULDER 0xC7  // Gamepad Right Shoulder
#define VK_GAMEPAD_LEFT_SHOULDER  0xC8  // Gamepad Left Shoulder
#define VK_GAMEPAD_LEFT_TRIGGER    0xC9  // Gamepad Left Trigger
#define VK_GAMEPAD_RIGHT_TRIGGER   0xCA  // Gamepad Right Trigger
#define VK_GAMEPAD_DPAD_UP         0xCB  // Gamepad D-pad Up
#define VK_GAMEPAD_DPAD_DOWN       0xCC  // Gamepad D-pad Down
#define VK_GAMEPAD_DPAD_LEFT       0xCD  // Gamepad D-pad Left
#define VK_GAMEPAD_DPAD_RIGHT      0xCE  // Gamepad D-pad Right
#define VK_GAMEPAD_MENU            0xCF  // Gamepad Menu
#define VK_GAMEPAD_VIEW            0xD0  // Gamepad View
#define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON 0xD1  // Gamepad Left Thumbstick Button
#define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON 0xD2  // Gamepad Right Thumbstick Button
#define VK_GAMEPAD_LEFT_THUMBSTICK_UP     0xD3  // Gamepad Left Thumbstick Up
#define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN   0xD4  // Gamepad Left Thumbstick Down
#define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT  0xD5  // Gamepad Left Thumbstick Right
#define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT   0xD6  // Gamepad Left Thumbstick Left
#define VK_GAMEPAD_RIGHT_THUMBSTICK_UP    0xD7  // Gamepad Right Thumbstick Up
#define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN  0xD8  // Gamepad Right Thumbstick Down
#define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT 0xD9  // Gamepad Right Thumbstick Right
#define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT  0xDA  // Gamepad Right Thumbstick Left

// OEM дополнительные клавиши (продолжение)
#define VK_OEM_4          0xDB  // [{ для US клавиатуры
#define VK_OEM_5          0xDC  // \| для US клавиатуры
#define VK_OEM_6          0xDD  // ]} для US клавиатуры
#define VK_OEM_7          0xDE  // '" для US клавиатуры
#define VK_OEM_8          0xDF  // OEM специфичные
#define VK_OEM_AX         0xE1  // AX ключ на японской клавиатуре
#define VK_OEM_102        0xE2  // <> или \| на 102-клавишной клавиатуре
#define VK_ICO_HELP       0xE3  // Help на ICO клавиатуре
#define VK_ICO_00         0xE4  // 00 на ICO клавиатуре
#define VK_PROCESSKEY     0xE5  // IME Process
#define VK_ICO_CLEAR      0xE6  // Clear на ICO клавиатуре
#define VK_PACKET         0xE7  // Используется для передачи Unicode символов

// Специальные клавиши (0xF6-0xFF)
#define VK_ATTN           0xF6  // Attn
#define VK_CRSEL          0xF7  // CrSel
#define VK_EXSEL          0xF8  // ExSel
#define VK_EREOF          0xF9  // Erase EOF
#define VK_PLAY           0xFA  // Play
#define VK_ZOOM           0xFB  // Zoom
#define VK_NONAME         0xFC  // Noname
#define VK_PA1            0xFD  // PA1
#define VK_OEM_CLEAR      0xFE  // Clear

short winapi_to_linux_key(short winapi_code) {
    for (int i = 0; i < sizeof(key_mappings); i++) {
        if (key_mappings[i].winapi_code == winapi_code) {
            return key_mappings[i].linux_code;
        }
    }
    return -1;
}

short linux_to_winapi_key(short linux_code) {
    for (int i = 0; i < sizeof(key_mappings); i++) {
        if (key_mappings[i].linux_code == linux_code) {
            return key_mappings[i].winapi_code;
        }
    }
    return -1;
}

#define MAPVK_VK_TO_VSC 0
#define MAPVK_VSC_TO_VK 1
#define MAPVK_VK_TO_CHAR 2
#define MAPVK_VSC_TO_VK_EX 3
#define MAPVK_VK_TO_VSC_EX 4
