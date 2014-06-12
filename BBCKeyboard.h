#define MAXCOL 10
#define MAXROW 8

struct KeyDet {
	const char *Desc;
	unsigned int Report;
};

//     0    1    2    3    4    5    6    7    8    9
//   /--------------------------------------------------
// 7 | ESC  F1   F2   F3   F5   F6   F8   F9   |\   RGHT
// 1 | Q    3    4    5    F4   8    F7   =-   ~^   LEFT
// 2 | F0   W    E    T    7    I    9    0    _£   DOWN
// 3 | 1    2    D    R    6    U    O    P    {[   UP
// 4 | CPLK A    X    F    Y    J    K    @    *:   RET
// 5 | SHLK S    C    G    H    N    L    +;   }]   DEL
// 6 | TAB  Z    SPC  V    B    M    <,   >.   ?/   COPY
// 0 | SHFT CTRL

#define MODKEY 0x100

#define MOD_LEFT_CTRL (1<<0)
#define MOD_LEFT_SHIFT (1<<1)
#define MOD_LEFT_ALT (1<<2)
#define MOD_LEFT_GUI (1<<3)
#define MOD_RIGHT_CTRL (1<<4)
#define MOD_RIGHT_SHIFT (1<<5)
#define MOD_RIGHT_ALT (1<<6)
#define MOD_RIGHT_GUI (1<<7)

KeyDet KeyDetails[MAXROW][MAXCOL] = {
/* Row 0 */{ { "SHFT", MODKEY | MOD_LEFT_SHIFT }, { "CTRL", MODKEY | MOD_LEFT_CTRL }, { "LNK0", 0x00 }, { "LNK1", 0x00 }, { "LNK2", 0x00 }, { "LNK3", 0x00 }, { "LNK4", 0x00 }, { "LNK5", 0x00 }, { "LNK6", 0x00 }, { "LNK7", 0x00 } },
/* Row 1 */{ { "Q", 0x14 }, { "3", 0x20 }, { "4", 0x21 }, { "5", 0x22 }, { "f4", 0x3d }, { "8", 0x25 }, { "f7", 0x40 }, { "-=", 0x2d }, { "^~", 0x35 }, { "LEFT", 0x50 } },
/* Row 2 */{ { "F0", 0x43 }, { "W", 0x1a }, { "E", 0x08 }, { "T", 0x17 }, { "7", 0x24 }, { "I", 0x0c }, { "9", 0x26 }, { "0", 0x27 }, { "_£", 0x2e }, { "DOWN", 0x51 } },
/* Row 3 */{ { "1", 0x1e }, { "2", 0x1f }, { "D", 0x07 }, { "R", 0x15 }, { "6", 0x23 }, { "U", 0x18 }, { "O", 0x12 }, { "P", 0x13 }, { "[{", 0x2f }, { "UP", 0x52 } },
/* Row 4 */{ { "CPLK", MODKEY | MOD_LEFT_ALT }, { "A", 0x04 }, { "X", 0x1b }, { "F", 0x09 }, { "Y", 0x1c }, { "J", 0x0d }, { "K", 0x0e }, { "@", 0x32 }, { ":*", 0x34 }, { "RET", 0x28 } },
/* Row 5 */{ { "SHLK", MODKEY | MOD_LEFT_GUI }, { "S", 0x16 }, { "C", 0x06 }, { "G", 0x0a }, { "H", 0x0b }, { "N", 0x11 }, { "L", 0x0f }, { ";+", 0x33 }, { "]}", 0x30 }, { "DEL", 0x2a } },
/* Row 6 */{ { "TAB", 0x2b }, { "Z", 0x1d }, { "SPC", 0x2c }, { "V", 0x19 }, { "B", 0x05 }, { "M", 0x10 }, { ",<", 0x36 }, { ".>", 0x37 }, { "/?", 0x38 }, { "COPY", 0x49 } },
/* Row 7 */{ { "Esc", 0x29 }, { "f1", 0x3a }, { "f2", 0x3b }, { "f3", 0x3c }, { "f5", 0x3e }, { "f6", 0x3f }, { "f8", 0x41 }, { "f9", 0x42 },{ "\\|", 0x31 },{ "RGHT", 0x4f } }
};

KeyDet BreakDetails = {"BRK", 0x48};


