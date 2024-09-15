#include <IDT.h>

struct IDT64 {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t  types_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero = 0;
};

#define IDT_TYPE_INTERRUPT 0xE
#define IDT_TYPE_TRAP      0xF

#define IDT_ATTR_PRESENT   0x80
#define IDT_ATTR_DPL0      0x00
#define IDT_ATTR_DPL1      0x20
#define IDT_ATTR_DPL2      0x40
#define IDT_ATTR_DPL3      0x60

extern IDT64 _idt[256];
extern uint64_t isr1;
extern "C" void LoadIDT();

void set_idt_entry(int index, uint64_t offset) {
	_idt[index].offset_low = (uint16_t)(((uint64_t)&isr1 & 0x000000000000ffff));
	_idt[index].offset_mid = (uint16_t)(((uint64_t)&isr1 & 0x00000000ffff0000) >> 16);
	_idt[index].offset_high = (uint32_t)(((uint64_t)&isr1 & 0xffffffff00000000) >> 32);
	_idt[index].ist = 0;
	_idt[index].selector = 0x08;
	_idt[index].types_attr = 0x8e;
}

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void RemapPic() {
    uint8_t a1, a2;

    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC1_DATA, 0x20);   // ICW2: PIC1 vector offset
    outb(PIC2_DATA, 0x28);   // ICW2: PIC2 vector offset
    outb(PIC1_DATA, 4);      // ICW3: Tell PIC1 about PIC2
    outb(PIC2_DATA, 2);      // ICW3: Tell PIC2 its cascade identity
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}

void InitializeIDT() {
    set_idt_entry(1, isr1);

    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
    LoadIDT();
}

extern "C" void isr1_handler() {
    uint8_t scanCode = inb(0x60);
    uint8_t chr = 0;

	outb(0xE9, 'w');

    // Process scan code here (e.g., lookup and convert to character)
    // if (scanCode < 0x3A) {
    //     chr = KBSet1::ScanCodeLookupTable[scanCode];
    // }

    // Call external handler if defined
    // if (MainKeyboardHandler != nullptr) {
    //     MainKeyboardHandler(scanCode, chr);
    // }

    outb(0x20, 0x20); // EOI for PIC1
    outb(0xA0, 0x20); // EOI for PIC2
}
