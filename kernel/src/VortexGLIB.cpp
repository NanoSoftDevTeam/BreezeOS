#include <VortexGLIB>

struct limine_framebuffer *gFramebuffer;
volatile uint32_t *fbPtr;

void InitializeFramebuffer(struct limine_framebuffer *framebuffer) {
    gFramebuffer = framebuffer;
    fbPtr = static_cast<volatile uint32_t *>(framebuffer->address);
}

void putPx(uint64_t x, uint64_t y, uint32_t color) {
    fbPtr[y * (gFramebuffer->pitch / 4) + x] = color;
}

void DrawSquare(uint64_t startX, uint64_t startY, uint64_t endX, uint64_t endY, uint32_t color) {
    uint64_t x1 = min(startX, endX);
    uint64_t x2 = max(startX, endX);
    uint64_t y1 = min(startY, endY);
    uint64_t y2 = max(startY, endY);

    for (uint64_t x = x1; x <= x2; ++x) {
        for (uint64_t y = y1; y <= y2; ++y) {
            putPx(x, y, color);
        }
    }
}

void font_char(char c, size_t x, size_t y, uint32_t color) {
    const uint8_t *glyph = FONT[(size_t) c];

    for (size_t yy = 0; yy < 8; yy++) {
        for (size_t xx = 0; xx < 8; xx++) {
            if (glyph[yy] & (1 << xx)) {
                putPx(x + xx, y + yy, color);
            }
        }
    }
}

void font_str(const char *s, size_t x, size_t y, uint32_t color) {
    char c;

    while ((c = *s++) != 0) {
        font_char(c, x, y, color);
        x += 8;
    }
}