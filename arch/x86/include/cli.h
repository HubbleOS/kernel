#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>




enum Color {
	COLOR_BLACK,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_WHITE,
	COLOR_LIGHT_GREY,
	COLOR_DEFAULT
};
void set_text_color(enum Color color);


enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};


uint8_t color_to_vga(enum Color color) {
	switch (color) {
		case COLOR_BLACK: return VGA_COLOR_BLACK;
		case COLOR_RED: return VGA_COLOR_RED;
		case COLOR_GREEN: return VGA_COLOR_GREEN;
		case COLOR_YELLOW: return VGA_COLOR_BROWN;
		case COLOR_BLUE: return VGA_COLOR_BLUE;
		case COLOR_MAGENTA: return VGA_COLOR_MAGENTA;
		case COLOR_CYAN: return VGA_COLOR_CYAN;
		case COLOR_WHITE: return VGA_COLOR_WHITE;
		case COLOR_LIGHT_GREY: return VGA_COLOR_LIGHT_GREY;
		default: return VGA_COLOR_LIGHT_GREY;
	}
}



#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

size_t strlen(const char* str) ;
static inline void outb(uint16_t port, uint8_t val) {
	__asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

void move_cursor(size_t row, size_t column);



static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}


void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void putchar(char c) ;
void terminal_write(const char* data, size_t size) 
{
	for (size_t i = 0; i < size; i++)
		putchar(data[i]);
}

void print(const char* data) 
{
	terminal_write(data, strlen(data));
}

static inline uint8_t inb(uint16_t port);

char scancode_to_ascii(uint8_t scancode);

char getchar(void);

void read_line(char *buf, int max_len);

int strcmp(const char *s1, const char *s2);

void terminal_initialize(void) ;

void terminal_setcolor(uint8_t color);

void set_text_color(enum Color color);

void scroll_terminal_up();







void print_dec(unsigned int num)
{
	char buf[10];
	int i = 0;

	if (num == 0)
	{
		putchar('0');
		return;
	}

	while (num > 0)
	{
		buf[i++] = '0' + (num % 10);
		num /= 10;
	}

	while (i--)
		putchar(buf[i]);
}



void neofetch(void)
{
	set_text_color(COLOR_GREEN);
	print(
		"  _____                _              \n"
		" / ____|              (_)             \n"
		"| |       ___   _ __   _  _   _  _ __ ___\n"
		"| |      / _ \\ | '_ \\ | || | | || '_ ` _ \\\n"
		"| |____ | (_) || |_) || || |_| || | | | | |\n"
		" \\_____| \\___/ | .__/ |_| \\__,_||_| |_| |_|\n"
		"               | |                         \n"
		"               |_|                         \n"
		"\n");

	set_text_color(COLOR_DEFAULT);
}

void clearConsole(void)
{
	print("\x1b[2J");
	print("\x1b[H");
}



void cli(void)
{
	neofetch();
	

	while (1)
	{
		set_text_color(COLOR_GREEN);
		print("> ");
		set_text_color(COLOR_DEFAULT);

		char buffer[128];
		read_line(buffer, sizeof(buffer));
	
		if (buffer[0] == ' ')
		{
			print("\n");
			continue;
		}

		if (strcmp(buffer, "exit") == 0)
		{
			print("\nGoodbye!\n");

		}
		else if (strcmp(buffer, "hello") == 0)
		{
			print("\nHello!\n");
		}
		else if (strcmp(buffer, "neofetch") == 0)
		{
			neofetch();
		}
		else if (strcmp(buffer, "clear") == 0)
		{
			terminal_initialize();
		}
		else
		{
			print("\nUnknown command!\n");
		}

		
	}
}



size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
void move_cursor(size_t row, size_t column) {
	uint16_t position = row * VGA_WIDTH + column;

	// Записуємо старший байт
	outb(0x3D4, 14);
	outb(0x3D5, (position >> 8) & 0xFF);

	// Записуємо молодший байт
	outb(0x3D4, 15);
	outb(0x3D5, position & 0xFF);
}
void putchar(char c) 
{
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT){
			scroll_terminal_up();
			terminal_row = VGA_HEIGHT - 1;
		}
	} else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
		}
	}
	
	move_cursor(terminal_row, terminal_column);
	
}
char scancode_to_ascii(uint8_t scancode) {
	static char kbd_us[128] = {
		0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b', // backspace
		'\t', // tab
		'q','w','e','r','t','y','u','i','o','p','[',']','\n', // enter
		0, // control
		'a','s','d','f','g','h','j','k','l',';','\'','`',
		0, // left shift
		'\\','z','x','c','v','b','n','m',',','.','/',
		0, // right shift
		'*',
		0, // alt
		' ', // space
		// ...
	};

	if (scancode > 127)
		return 0;
	return kbd_us[scancode];
}
char getchar(void) {
	uint8_t scancode;

	// Чекаємо на натиск
	while (1) {
		scancode = inb(0x60);

		// Чекаємо на натиск клавіші (не відпускання)
		if (!(scancode & 0x80)) {
			// Тепер чекаємо, поки клавішу відпустять
			while ((inb(0x60) & 0x80) == 0);
			break;
		}
	}

	return scancode_to_ascii(scancode);
}
void read_line(char *buf, int max_len)
{
	int i = 0;
	while (i < max_len - 1)
	{
		char c = getchar();

		if (c == '0')
		{
			if (getchar() == 'd' || c == 'c'){
				print("^C");
				buf[0] = '\0';
				return;
			}
		}


		if (c == '\b')
		{
			if (i > 0 && terminal_column > 0) {
				i--;
				terminal_column--;
				terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
				move_cursor(terminal_row, terminal_column);
			}
			continue;
		}

		putchar(c);

		if (c == '\r' || c == '\n')
			break;

		buf[i++] = c;
	}
	buf[i] = '\0';
}

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}

void set_text_color(enum Color color) {
	terminal_setcolor(vga_entry_color(color_to_vga(color), VGA_COLOR_BLACK));
}

void scroll_terminal_up() {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(y - 1) * VGA_WIDTH + x] =
                terminal_buffer[y * VGA_WIDTH + x];
        }
    }

    // Очищуємо останній рядок
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] =
            vga_entry(' ', terminal_color);
    }

    // Переміщуємо курсор
    if (terminal_row > 0)
        terminal_row--;
    terminal_column = 0;
    move_cursor(terminal_row, terminal_column);
}
int strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}
	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}
static inline uint8_t inb(uint16_t port) {
	uint8_t result;
	__asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}