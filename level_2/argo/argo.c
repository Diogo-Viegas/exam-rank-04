#include "argo.h"

int	parse_value(json *dst, FILE *stream);
void	free_json(json j);
int	argo(json *dst, FILE *stream);

int	peek(FILE *stream)
{
	int	c = getc(stream);
	ungetc(c, stream);
	return c;
}

void	unexpected(FILE *stream)
{
	if (peek(stream) != EOF)
		printf("unexpected token '%c'\n", peek(stream));
	else
		printf("unexpected end of input\n");
}

int	accept(FILE *stream, char c)
{
	if (peek(stream) == c)
	{
		(void)getc(stream);
		return 1;
	}
	return 0;
}

int	expect(FILE *stream, char c)
{
	if (accept(stream, c))
		return 1;
	unexpected(stream);
	return 0;
}

void	free_json(json j)
{
	switch (j.type)
	{
		case MAP:
			for (size_t i = 0; i < j.map.size; i++)
			{
				free(j.map.data[i].key);
				free_json(j.map.data[i].value);
			}
			free(j.map.data);
			break ;
		case STRING:
			free(j.string);
			break ;
		default:
			break ;
	}
}

void	serialize(json j)
{
	switch (j.type)
	{
		case INTEGER:
			printf("%d", j.integer);
			break ;
		case STRING:
			putchar('"');
			for (int i = 0; j.string[i]; i++)
			{
				if (j.string[i] == '\\' || j.string[i] == '"')
					putchar('\\');
				putchar(j.string[i]);
			}
			putchar('"');
			break ;
		case MAP:
			putchar('{');
			for (size_t i = 0; i < j.map.size; i++)
			{
				if (i != 0)
					putchar(',');
				serialize((json){.type = STRING, .string = j.map.data[i].key});
				putchar(':');
				serialize(j.map.data[i].value);
			}
			putchar('}');
			break ;
	}
}

int	parse_integer(json *dst, FILE *stream)
{
	int	n;

	if (fscanf(stream, "%d", &n) != 1)
		return 0;
	dst->type = INTEGER;
	dst->integer = n;
	return 1;
}

int	parse_string(json *dst, FILE *stream)
{
	char	*str = NULL;
	int		size = 0;
	int		c;

	if (!expect(stream, '"'))
		return 0;
	while (1)
	{
		c = getc(stream);
		if (c == EOF)
		{
			unexpected(stream);
			free(str);
			return 0;
		}
		if (c == '"')
			break ;
		if (c == '\\')
		{
			c = getc(stream);
			if (c != '\\' && c != '"')
			{
				unexpected(stream);
				free(str);
				return 0;
			}
		}
		str = realloc(str, size + 2);
		str[size++] = c;
	}
	str[size] = 0;
	dst->type = STRING;
	dst->string = str;
	return 1;
}

int	parse_map(json *dst, FILE *stream)
{
	pair	p;

	dst->type = MAP;
	dst->map.data = NULL;
	dst->map.size = 0;

	if (!expect(stream, '{'))
		return 0;
	if (accept(stream, '}'))
		return 1;
	while (1)
	{
		json	key;

		if (!parse_string(&key, stream))
			return 0;
		p.key = key.string;
		if (!expect(stream, ':'))
			return 0;
		if (!parse_value(&p.value, stream))
			return 0;
		dst->map.data = realloc(dst->map.data, sizeof(pair) * (dst->map.size + 1));
		dst->map.data[dst->map.size++] = p;
		if (accept(stream, '}'))
			return 1;
		if (!expect(stream, ','))
			return 0;
	}
}

int	parse_value(json *dst, FILE *stream)
{
	int	c;

	c = peek(stream);
	if (c == '"')
		return parse_string(dst, stream);
	if (c == '{')
		return parse_map(dst, stream);
	if (isdigit(c) || c == '-')
		return parse_integer(dst, stream);
	unexpected(stream);
	return 0;
}

int	argo(json *dst, FILE *stream)
{
	if (!parse_value(dst, stream))
		return -1;
	return 1;
}

int	main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	char *filename = argv[1];
	FILE *stream = fopen(filename, "r");
	json	file;
	if (argo (&file, stream) != 1)
	{
		free_json(file);
		return 1;
	}
	serialize(file);
	printf("\n");
}
