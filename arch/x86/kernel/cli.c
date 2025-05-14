#include "framebuffer.h"
#include "font.h"

int bufer_history = 0;
char key_history[128][128];
uint8_t key_history_it[128];

uint16_t t_colums = 80;
uint16_t t_rows = 25;
uint8_t t_row = 0;
uint8_t t_col = 0;
uint8_t t_color = 0x0f;
uint32_t t_color_char = 0xFFFFFF;

uint8_t t_pos_x = 1000;
uint8_t t_pos_y = 1000;

uint16_t cursor_x = 0;
uint16_t cursor_y = 0;
uint16_t cursor_visible = 1;
uint16_t t_width = 0;
uint16_t t_height = 0;
framebuffer_info_t *fbcli = 0;

enum Color
{
	COLOR_BLACK = 0x0,
	COLOR_RED = 0x4,
	COLOR_GREEN = 0x32A852,
	COLOR_YELLOW = 0x6,
	COLOR_BLUE = 0x1,
	COLOR_MAGENTA = 0x5,
	COLOR_CYAN = 0x3,
	COLOR_WHITE = 0xFFFFFF,
	COLOR_LIGHT_GREY = 0x8,
	COLOR_DEFAULT = 0x9
};

void neofetch(void);
void print_char(char c);
void print(const char *str);
int strcmp(const char *s1, const char *s2);
void print_hex(uint8_t value);
uint8_t strlen(const char *str);

void wait(volatile unsigned int ticks)
{
	for (volatile unsigned int i = 0; i < ticks; i++)
		;
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	__asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
	__asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

char scancode_to_ascii[128] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', // 0x00 - 0x09
	'9', '0', '-', '=', '\b', '\t',				   // ...
	'q', 'w', 'e', 'r',							   // ...
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // ...
	0,											   // Ctrl
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, // Left shift
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
	0,			 // Right shift
	'*', 0, ' ', // Spacebar
				 // ... (можна розширити за потребою)
};

#define KEY_UP 0x80
#define KEY_DOWN 0x81
#define KEY_LEFT 0x82
#define KEY_RIGHT 0x83

char read_ps2_key()
{
	uint8_t scancode = 0;
	bool is_extended = false;

	while (1)
	{
		// Чекаємо, поки з'являться дані
		while (!(inb(0x64) & 1))
			;

		scancode = inb(0x60);

		if (scancode == 0xE0)
		{
			is_extended = true;
			while (!(inb(0x64) & 1))
				;
			scancode = inb(0x60);
		}

		if (scancode & 0x80)
			continue;

		break;
	}

	if (is_extended)
	{
		switch (scancode)
		{
		case 0x48:
			return KEY_UP;
		case 0x50:
			return KEY_DOWN;
		case 0x4B:
			return KEY_LEFT;
		case 0x4D:
			return KEY_RIGHT;
		default:
			return 0;
		}
	}

	return scancode_to_ascii[scancode];
}

void terminal_putentryat(char c)
{
	draw_char(fbcli, c, t_pos_x + (t_col * 8), t_pos_y + (t_row * 8), t_color_char);
}
void move_cursor(uint8_t row, uint8_t col)
{
	return;
}
void read_line(char *buf, int max_len)
{
	int buf_it = bufer_history;
	int i = 0;
	while (i < max_len)
	{
		char c = read_ps2_key();

		if ((uint8_t)c == KEY_UP)
		{
			if (buf_it == 0)
				continue;
			t_col = 3;
			i = 0;
			for (int i = 0; i < strlen(buf); i++)
				print(" ");
			for (int j = 0; j < strlen(key_history[buf_it - 1]); j++)
			{
				buf[j] = key_history[buf_it - 1][j];
				i++;
			}
			t_col = 3;
			print(key_history[--buf_it]);
			continue;
		};
		if ((uint8_t)c == KEY_DOWN)
		{
			if (buf_it == bufer_history)
				continue;
			t_col = 3;
			i = 0;
			for (int i = 0; i < strlen(buf); i++)
				print(" ");
			for (int j = 0; j < strlen(key_history[buf_it + 1]); j++)
			{
				buf[j] = key_history[buf_it + 1][j];
				i++;
			}
			t_col = 3;
			print(key_history[++buf_it]);
			continue;
		}

		if (c == '\b')
		{
			if (i > 0 && t_col > 0)
			{
				i--;
				t_col--;
				terminal_putentryat(' ');
				move_cursor(t_row, t_col);
			}
			continue;
		}

		if (c == '\r' || c == '\n')
		{
			t_col = 0;
			t_row++;
			break;
		}
		buf[i++] = c;
		print_char(buf[i - 1]);
	}
	buf[i] = '\0';
}

void clear(void)
{
	uint32_t *pixels = (uint32_t *)fbcli->base;
	for (int i = 0; i < (fbcli->pitch / 4) * fbcli->height; ++i)
	{
		pixels[i] = 0x000000;
	}
	t_col = t_row = 0;
}
int atoi(const char *str)
{
	int result = 0;
	while (*str >= '0' && *str <= '9')
	{
		result = result * 10 + (*str - '0');
		str++;
	}
	return result;
}

int cli(framebuffer_info_t *fb)
{
	fbcli = fb;

	clear();

	neofetch();

	while (1)
	{
		print("> ");
		char buffer[100];
		read_line(key_history[bufer_history++], 100);

		if (key_history[bufer_history - 1][0] == '\0')
		{
			--bufer_history;
			print("\n");
			continue;
		}

		if (strcmp(key_history[bufer_history - 1], "exit") == 0)
		{
			print("\nGoodbye!\n");
			break;
		}
		else if (strcmp(key_history[bufer_history - 1], "hello") == 0)
		{
			print("\nHello!\n");
		}
		else if (strcmp(key_history[bufer_history - 1], "neofetch") == 0)
		{
			neofetch();
		}
		else if (strcmp(key_history[bufer_history - 1], "clear") == 0)
		{
			clear();
		}
		else if (strcmp(key_history[bufer_history - 1], "change-y") == 0)
		{
			read_line(key_history[bufer_history - 1], 100);
			t_pos_y = atoi(buffer);
		}
		else if (strcmp(key_history[bufer_history - 1], "change-x") == 0)
		{
			read_line(key_history[bufer_history - 1], 100);
			t_pos_x = atoi(buffer);
		}
		else if (strcmp(key_history[bufer_history - 1], "help") == 0)
		{
			print("\n");
			print("exit\n");
			print("hello\n");
			print("neofetch\n");
			print("clear\n");
			print("help\n");
			print("change-x\n");
			print("change-y\n");
		}
		else
		{
			print("\nUnknown command!\n");
		}
	}

	return 0;
}
void print_hex(uint8_t value)
{
	char hex_digits[] = "0123456789ABCDEF";
	char buf[5];

	buf[0] = '0';
	buf[1] = 'x';
	buf[2] = hex_digits[(value >> 4) & 0x0F];
	buf[3] = hex_digits[value & 0x0F];
	buf[4] = '\0';

	print(buf); // твоя функція print(const char*)
}
void print_char(char c)
{
	if (c == '\n' || t_col >= t_colums)
	{
		t_row++;
		t_col = 0;
	}
	terminal_putentryat(c);
	t_col++;
}

void print(const char *str)
{
	while (*str)
	{
		print_char(*str);
		++str;
	}
}

void neofetch(void)
{
	t_color_char = COLOR_GREEN;
	print(
		"                \n"
		"  _____                _              \n"
		" / ____|              (_)             \n"
		"| |       ___   _ __   _  _   _  _ __ ___\n"
		"| |      / _ \\ | '_ \\ | || | | || '_ ` _ \\\n"
		"| |____ | (_) || |_) || || |_| || | | | | |\n"
		" \\_____| \\___/ | .__/ |_| \\__,_||_| |_| |_|\n"
		"               | |                         \n"
		"               |_|                         \n"
		"\n");

	t_color_char = COLOR_WHITE;
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
uint8_t strlen(const char *str)
{
	uint8_t len = 0;
	while (str[len])
		len++;
	return len;
}