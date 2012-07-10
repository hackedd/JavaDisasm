#include "util.h"

void print_string(FILE* fp, const char* string)
{
	const char* p;

	for (p = string; *p != '\0'; p++)
	{
		if (*p == '\\')
			fprintf(fp, "\\\\");
		else if (*p == '\t')
			fprintf(fp, "\\t");
		else if (*p == '\n')
			fprintf(fp, "\\n");
		else if (*p == '\r')
			fprintf(fp, "\\r");
		else if (*p == '"')
			fprintf(fp, "\\\"");
		else if (*p < 0x20 || *p >= 0x7f)
			fprintf(fp, "\\u%04x", *p & 0xff);
		else
			fprintf(fp, "%c", *p);
	}
}

void print_string_w(FILE* fp, const uint32_t* string)
{
	const uint32_t* p;

	for (p = string; *p != '\0'; p++)
	{
		if (*p == L'\\')
			fprintf(fp, "\\\\");
		else if (*p == L'\t')
			fprintf(fp, "\\t");
		else if (*p == L'\n')
			fprintf(fp, "\\n");
		else if (*p == L'\r')
			fprintf(fp, "\\r");
		else if (*p == L'"')
			fprintf(fp, "\\\"");
		else if (*p < 0x0020 || (*p >= 0x007f && *p <= 0xffff))
			fprintf(fp, "\\u%04x", *p);
		else
			fprintf(fp, "%c", *p);
	}
}


char* escape_string(char* dest, const char* string, size_t length)
{
	const char* p;
	char* q;

	if (length < 0)
		length = strlen(string);

	p = string;
	q = dest;
	while (length)
	{
		if (*p == '\0')
		{
			*q++ = '\\';
			*q++ = '0';
		}
		else if (*p == '\\')
		{
			*q++ = '\\';
			*q++ = '\\';
		}
		else if (*p == '\t')
		{
			*q++ = '\\';
			*q++ = 't';
		}
		else if (*p == '\n')
		{
			*q++ = '\\';
			*q++ = 'n';
		}
		else if (*p == '\r')
		{
			*q++ = '\\';
			*q++ = 'r';
		}
		else if (*p == '"')
		{
			*q++ = '\\';
			*q++ = '"';
		}
		else if (*p < 0x20 || *p >= 0x7f)
		{
			// TODO: use wchar_t
			sprintf(q, "\\u%04x", *p & 0xff);
			q += 6;
		}
		else
		{
			*q++ = *p;
		}

		p += 1;
		length -= 1;
	}

	*q++ = '\0';
	return dest;
}

char* unescape_string(char* dest, const char* string, size_t length)
{
	const char* p;
	char* q, c;

	if (length < 0)
		length = strlen(string);

	p = string;
	q = dest;
	while (length)
	{
		if (*p == '\\')
		{
			p += 1;
			if (*p == '0')
				*q++ = '\0';
			else if (*p == '\\')
				*q++ = '\\';
			else if (*p == 'n')
				*q++ = '\n';
			else if (*p == 'r')
				*q++ = '\r';
			else if (*p == '"')
				*q++ = '"';
			else if (*p == 'u')
			{
				p += 1;
				// TODO: use wchar_t
				p += 2;
				if (*p >= '0' && *p <= '9')
					c = (*p - '0') << 4;
				else
					c = (*p - 'A' + 10) << 4;
				p += 1;
				if (*p >= '0' && *p <= '9')
					c |= (*p - '0');
				else
					c |= (*p - 'A' + 10);
				*q++ = c;
			}
		}
		else
		{
			*q++ = *p;
		}

		p += 1;
		length -= 1;
	}

	*q++ = '\0';
	return dest;
}
