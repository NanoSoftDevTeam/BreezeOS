#include <cstdint>
#include <cstddef>
#include <limine.h>
#include <VortexGLIB>
//#include <VortexUILib> // UI Library
#include <io.h>
#include <cstdarg>
#include <ATA.h>
#include <FAT32.h>

using namespace std;

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

namespace {

__attribute__((used, section(".requests")))
volatile LIMINE_BASE_REVISION(2);

}

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

namespace {

__attribute__((used, section(".requests")))
volatile limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
    .response = nullptr
};

}

// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .cpp file, as seen fit.

namespace {

__attribute__((used, section(".requests_start_marker")))
volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end_marker")))
volatile LIMINE_REQUESTS_END_MARKER;

}

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .cpp file.

extern "C" {

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = static_cast<uint8_t *>(dest);
    const uint8_t *psrc = static_cast<const uint8_t *>(src);

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = static_cast<uint8_t *>(s);

    for (size_t i = 0; i < n; i++) {
        p[i] = static_cast<uint8_t>(c);
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = static_cast<uint8_t *>(dest);
    const uint8_t *psrc = static_cast<const uint8_t *>(src);

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = static_cast<const uint8_t *>(s1);
    const uint8_t *p2 = static_cast<const uint8_t *>(s2);

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

}

// Halt and catch fire function.
namespace {

void hcf() {
    for (;;) {
#if defined (__x86_64__)
        asm ("hlt");
#elif defined (__aarch64__) || defined (__riscv)
        asm ("wfi");
#elif defined (__loongarch64)
        asm ("idle 0");
#endif
    }
}

}

// The following stubs are required by the Itanium C++ ABI (the one we use,
// regardless of the "Itanium" nomenclature).
// Like the memory functions above, these stubs can be moved to a different .cpp file,
// but should not be removed, unless you know what you are doing.
extern "C" {
    int __cxa_atexit(void (*)(void *), void *, void *) { return 0; }
    void __cxa_pure_virtual() { hcf(); }
    void *__dso_handle;
}

// Extern declarations for global constructors array.
extern void (*__init_array[])();
extern void (*__init_array_end[])();

void nop() {
    asm volatile ("nop");
}

uint32_t backColor    = 0x212121;
uint32_t dockColor    = 0x0F0F0F;
uint32_t toolBarColor = 0x0B0B0B;

struct version {
    uint64_t major;     // Major   #.-.-
    uint64_t version;   // Version -.#.-
    uint64_t minor;     // Minor   -.-.#
    bool isHalfYear;    // 1 (true) -- 2 (false)
    char* version_user; // What is the thing that version_t used?
    char* other_info;   // More info about the version of ---
};

typedef struct version version_t;

struct video {
    uint64_t x;
    uint64_t y;
};

typedef struct video video_t;

struct BreezeOS_TransferData_List_MetaData_Structure {
    char* os_name;                              // Probably will be "BreezeOS"
    version_t version;                          // Version of BreezeOS
    struct limine_framebuffer *framebuffer;     // Will be the VortexGLIB Framebuffer address
    video_t res;                                // Resolution of the framebuffer
    uint64_t resx;                              // Resolution X
    uint64_t resy;                              // Resolution Y
};

typedef struct BreezeOS_TransferData_List_MetaData_Structure BreezeOSTransferListMetaDataStruct_t;

const char* to_string(uint64_t value) {
    static char buffer[21]; // 20 digits + null terminator for uint64_t
    char* p = buffer + sizeof(buffer) - 1;
    *p = '\0';

    if (value == 0) {
        *--p = '0';
    } else {
        char* start = p;
        do {
            *--p = '0' + (value % 10);
            value /= 10;
        } while (value > 0);

        // Reverse the string
        char* end = start - 1;
        while (p < end) {
            char temp = *p;
            *p++ = *end;
            *end-- = temp;
        }
    }

    return p;
}

char* to_string_BOOL(bool boolean) {
    if (boolean) {
        return "true";
    } else {
        return "false";
    }
}

void writeln(const char *str) {
    do {
        outb(0xE9, *str);
    } while (*str++);

    outb(0xE9, '\n');
}

void write(const char *str) {
    do {
        outb(0xE9, *str);
    } while (*str++);
}

void print_TDLMS_t_table(BreezeOSTransferListMetaDataStruct_t TDLMS_TABLE) {
    writeln("---------------------------------------------");
    writeln("BREEZEOS TDLMS TABLE - NANOSOFT");
    writeln("---------------------------------------------");
    write("OS Name: ");
    writeln(TDLMS_TABLE.os_name);
    write("Version: ");
    write(to_string(TDLMS_TABLE.version.major));
    write(".");
    write(to_string(TDLMS_TABLE.version.version));
    write(".");
    writeln(to_string(TDLMS_TABLE.version.minor));
    write("Is Half Year Version: ");
    writeln(to_string_BOOL(TDLMS_TABLE.version.isHalfYear));
    write("What is using the version list: ");
    writeln(TDLMS_TABLE.version.version_user);
    write("Other Info About Version: ");
    writeln(TDLMS_TABLE.version.other_info);
    write("Resolution WIDTH(X): ");
    writeln(to_string(TDLMS_TABLE.resx));
    write("Resolution HEIGHT(Y): ");
    writeln(to_string(TDLMS_TABLE.resy));
    write("Framebuffer Address: ");
    writeln(to_string((size_t)&TDLMS_TABLE.framebuffer));
    writeln("---------------------------------------------");
    writeln("MISC INFO:");
    writeln("---------------------------------------------");
    writeln("Access TDLMS Menu Using LSHIFT+LCTRL");
    writeln(to_string((size_t)&TDLMS_TABLE.framebuffer->address));
}

int indexTLDMS = 10;

void println(const char* str) {
    font_str(str, 15, indexTLDMS, 0xEEEEEE);
    indexTLDMS += 15;
}

void println_offset(const char* str, int offset) {
    font_str(str, 15+offset, indexTLDMS, 0xEEEEEE);
    indexTLDMS += 15;
}

void print(const char* str) {
    font_str(str, 15, indexTLDMS, 0xEEEEEE);
}

void print_offset(const char* str, int offset) {
    font_str(str, 15+offset, indexTLDMS, 0xEEEEEE);
}

char* copystr(char* dest, const char* src) {
    char* dest_start = dest;
    
    while ((*dest++ = *src++) != '\0') {
    }
    
    return dest_start;
}

size_t getstrlength(const char *str) {
    size_t i = 0;
    while (*str++) {
        i++;
    }
    return i;
}

char* quoteString(const char* input) {
    static char buffer[256];
    
    size_t inputLength = getstrlength(input);
    size_t totalLength = inputLength + 2;
    
    if (totalLength >= sizeof(buffer)) {
        writeln("DebugERROR: 0x00B001");
        return NULL;
    }
    
    buffer[0] = '\"';
    copystr(buffer + 1, input); 
    buffer[totalLength - 1] = '\"';
    buffer[totalLength] = '\0';
    
    return buffer;
}

void TDLMS_TABLE_APP(BreezeOSTransferListMetaDataStruct_t TDLMS_TABLE) {
    DrawSquare(0, 0, gFramebuffer->width, gFramebuffer->height, 0x112211);

    println("-----------------------------------------------------------");
    println("   BREEZEOS - TDLMS TABLE - NanoSoft");
    println("-----------------------------------------------------------");
    println("ADVANCED");
    println("OS Name:");
    println(TDLMS_TABLE.os_name);
    println("V Major:");
    println(to_string(TDLMS_TABLE.version.major));
    println("V Build");
    println(to_string(TDLMS_TABLE.version.version));
    println("V Minor");
    println(to_string(TDLMS_TABLE.version.minor));
    println("Is Half Year Version?:");
    println(to_string_BOOL(TDLMS_TABLE.version.isHalfYear));
    println("What is using the version list:");
    println(quoteString(TDLMS_TABLE.version.version_user));
    println("Other Info About Version:");
    println(TDLMS_TABLE.version.other_info);
    println("Resolution WIDTH(X):");
    println(to_string(TDLMS_TABLE.resx));
    println("Resolution HEIGHT(Y):");
    println(to_string(TDLMS_TABLE.resy));
    println("Framebuffer Address:");
    println(to_string((size_t)&TDLMS_TABLE.framebuffer));
    println("-----------------------------------------------------------");
    println("MISC INFO:");
    println(to_string((size_t)&TDLMS_TABLE.framebuffer->address));
    println("-----------------------------------------------------------");

    while (1) {
        if (inb(0x60) == 0x01) {
            break;
        }
    }
}

void Draw() {
    DrawSquare(0, 0, gFramebuffer->width, gFramebuffer->height, backColor);
    DrawSquare(0, 0, 128, gFramebuffer->height, dockColor);
    DrawSquare(0, 0, gFramebuffer->width, 32, toolBarColor);

    font_str("BreezeOS Desktop - Alpha", gFramebuffer->width-200, gFramebuffer->height-16, backColor*2);
}

void putchar(char ch) {
    outb(0xE9, ch);
}

void puts(char *ch) {
    while (*ch++) {
        outb(0xE9, *ch);
    }
}

// Helper function to convert an integer to a hexadecimal string
void int_to_hex(uint32_t value, char* buffer, int width, int precision) {
    // We need a buffer to store the hexadecimal digits
    char temp[16];
    int i = 0;

    // Convert integer to hexadecimal string
    do {
        int digit = value % 16;
        temp[i++] = (digit < 10) ? (digit + '0') : (digit - 10 + 'A');
        value /= 16;
    } while (value > 0);

    // Add leading zeros for precision
    while (i < precision) {
        temp[i++] = '0';
    }

    // Reverse the string
    int j;
    for (j = 0; j < i / 2; j++) {
        char c = temp[j];
        temp[j] = temp[i - j - 1];
        temp[i - j - 1] = c;
    }

    // Add padding for width
    int padding = width - i;
    if (padding > 0) {
        for (j = 0; j < padding; j++) {
            *buffer++ = ' ';
        }
    }

    // Copy the hexadecimal string to the buffer
    for (j = 0; j < i; j++) {
        *buffer++ = temp[j];
    }

    *buffer = '\0';  // Null-terminate the string
}

// Custom printf function
void printf_serial(const char* format, ...) {
    const char* p;
    va_list args;
    va_start(args, format);

    for (p = format; *p; p++) {
        if (*p != '%') {
            putchar(*p);
            continue;
        }

        // Handle '%'
        p++;

        // Default width and precision
        int width = 0;
        int precision = 0;
        int is_precision_set = 0;

        // Parse width
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }

        // Parse precision
        if (*p == '.') {
            p++;
            is_precision_set = 1;
            precision = 0;
            while (*p >= '0' && *p <= '9') {
                precision = precision * 10 + (*p - '0');
                p++;
            }
        }

        // Handle format specifiers
        if (*p == 'x') {
            uint32_t value = va_arg(args, uint32_t);
            char buffer[64];
            int_to_hex(value, buffer, width, is_precision_set ? precision : 1);
            // Output the buffer
            for (char* b = buffer; *b; b++) {
                putchar(*b);
            }
        } else {
            putchar('%');
            putchar(*p);
        }
    }

    va_end(args);
}

void printSectorData(uint8_t* buffer, size_t data_size) {
    for (int i = 0; i < (int)data_size*FS_SECT_SIZE; i++) {
        if (i%25==0) puts("\n");
	printf_serial("%02x ", buffer[i]);
    }
}

__attribute__((noreturn)) extern "C" void kmain() {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    for (size_t i = 0; &__init_array[i] != __init_array_end; i++) {
        __init_array[i]();
    }

    if (framebuffer_request.response == nullptr
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    InitializeFramebuffer(framebuffer_request.response->framebuffers[0]);

    Draw();

    BreezeOSTransferListMetaDataStruct_t BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER;

    version_t VERSION_INITIALIZER;
    video_t   VIDEO_INITIALIZER;

    VERSION_INITIALIZER.major = 1;
    VERSION_INITIALIZER.version = 0;
    VERSION_INITIALIZER.minor = 0;
    VERSION_INITIALIZER.isHalfYear = false;
    VERSION_INITIALIZER.version_user = "BreezeOS";
    VERSION_INITIALIZER.other_info = "ALPHA";

    VIDEO_INITIALIZER.x = gFramebuffer->width;
    VIDEO_INITIALIZER.y = gFramebuffer->height;

    uint64_t x__ = 0;
    uint64_t y__ = 0;

    for (int i = 0; i <= VIDEO_INITIALIZER.x; i++) {
        x__++;
    }

    for (int i = 0; i <= VIDEO_INITIALIZER.y; i++) {
        y__++;
    }

    BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER.os_name = "BreezeOS";
    BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER.version = VERSION_INITIALIZER;
    BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER.framebuffer = gFramebuffer;
    BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER.res = VIDEO_INITIALIZER;
    BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER.resx = x__;
    BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER.resy = y__;

    print_TDLMS_t_table(BreezeOS_TRANSFER_DATA_LIST_METADATA_STRUCT_T_INITIALIZER);
    
    bool shift = false;
    
    initialize_fat32();

    uint8_t data[] = "Data Test";
    create_file("FileName.txt", data, sizeof(data));

    hcf();
}