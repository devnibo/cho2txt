#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

enum print
{
	PRINT_NO,
	PRINT_TITLE,
	PRINT_TITLE_DIRECTIVE
};

enum grid
{
	GRID_NO,
	GRID_START,
	GRID_END
};

void printHelp()
{
	static const char help[] = "Usage: cho2txt [-t] [-d] [FILE]...\n"
		"Extract lyrics from chordpro files.\n\n"
		"  -t, --title\t\tPrint title\n"
		"  -d, --directive\tPrint title as chordpro directive\n";
	printf("%s", help);
}

char *trim(char *text)
{
	char *trimmedText = NULL;
	int begin = 0;
	int end = 0;
	for (int i=0; i<strlen(text); i++)
	{
		if
		(
				text[i] == ' ' ||
				text[i] == '\n' ||
				text[i] == '\t' ||
				text[i] == '\r'
		)
			begin++;
		else
			break;
	}
	for (int i=strlen(text)-1; i>=0; i--)
	{
		if
		(
			text[i] == ' '||
			text[i] == '\n' ||
			text[i] == '\t' ||
			text[i] == '\r'
		)
			end++;
		else
			break;
	}
	int k = 0;
	for (int i=0; i<strlen(text); i++)
	{
		if (i >= begin && i < strlen(text) - end)
		{
			trimmedText = realloc(trimmedText, (k+1) * sizeof(char));
			trimmedText[k] = text[i];
			k++;
		}
	}
	trimmedText = realloc(trimmedText, (k+1) * sizeof(char));
	trimmedText[k] = 0;
	free(text);
	return trimmedText;
}

char *parseTitle(const char *directive)
{
	char *title = NULL;
	bool doParse = false;
	int t = 0;
	for (int i=0; i<strlen(directive); i++)
	{
		if (directive[i] == '}')
			doParse = false;
		if (doParse)
		{
			title = realloc(title, (t+1) * sizeof(char));
			title[t] = directive[i];
			t++;
		}
		if (directive[i] == ':')
			doParse = true;
	}
	title = realloc(title, (t+1) * sizeof(char));
	title[t] = 0;
	return trim(title);
}

bool isTitle(const char *directive)
{
	static const char title[] = "title:";
	int t = 0;
	for (int i=0; i<strlen(directive); i++)
	{
		if (i > 0 && t < 7)
		{
			if (directive[i] != title[t])
				return false;
			t++;
			if (t == 6)
				return true;
		}
	}
}

enum grid isGrid(const char *directive)
{
	enum grid g = GRID_NO;
	static const char start[] = "start_of_grid";
	static const char end[] = "end_of_grid";
	size_t directiveLen = strlen(directive);
	int i = 1;
	while (i<directiveLen)
	{
		if (directive[i] != start[i-1])
		{
			g = GRID_NO;
			break;
		}
		i++;
		if (i == 14)
		{
			g = GRID_START;
			break;
		}
	}
	if (g != GRID_NO)
		return g;
	i = 1;
	while (i<directiveLen)
	{
		if (directive[i] != end[i-1])
		{
			g = GRID_NO;
			break;
		}
		i++;
		if (i == 12)
		{
			g = GRID_END;
			break;
		}
	}
	return g;
}

char *extractLyrics(int fd, enum print printTitle)
{
	char *text = malloc(sizeof(char));
	int *i = malloc(sizeof(int));
	*i = 0;
	int d = 0;
	char buf;
	bool isLyric = true;
	bool isLyricInLine = false;
	bool isDirectiveInLine = false;
	bool isCurlyBrace = false;
	char *directive = NULL;
	enum grid g = GRID_NO;
	while (1)
	{
		if (read(fd, &buf, 1) == 1)
    {
			if (buf == '[')
			{
				isLyric = false;
				isDirectiveInLine = true;
			}
			if (buf == '{')
			{
				isLyric = false;
				isDirectiveInLine = true;
				isCurlyBrace = true;
			}
			if (isLyric)
			{
				if (buf == '\n' && !isLyricInLine && isDirectiveInLine)
					goto IGNORE;
				text[*i] = buf;
				(*i)++;
				text = realloc(text, ((*i)+1) * sizeof(char));
				isLyricInLine = true;
				IGNORE:
			}
			else
			{
				if (isCurlyBrace)
				{
					directive = realloc(directive, (d+1) * sizeof(char));
					directive[d] = buf;
					d++;
				}
			}
			if (buf == '}')
			{
				isLyric = true;
				directive = realloc(directive, (d+1) * sizeof(char));
				directive[d] = 0;
				g = isGrid(directive);
				if (g == GRID_START)
					isLyric = false;
				else if (g == GRID_END)
					isLyric = true;
				if (printTitle > 0 && isTitle(directive))
				{
					char *title;
					if (printTitle == PRINT_TITLE)
						title = parseTitle(directive);
					else
					{
						title = malloc((strlen(directive)+1) * sizeof(char));
						strcpy(title, directive);
					}
					for (int k=0; k<strlen(title); k++)
					{
						text[*i] = title[k];
						(*i)++;
						text = realloc(text, ((*i)+1) * sizeof(char));
					}
					free(title);
					text[*i] = '\n';
					(*i)++;
					text = realloc(text, ((*i)+1) * sizeof(char));
				}
				d = 0;
				free(directive);
				directive = NULL;
				isCurlyBrace = false;
			}
			if (buf == ']')
				isLyric = true;
			if (buf == '\n')
			{
				isDirectiveInLine = false;
				isLyricInLine = false;
			}
		}
		else
			break;
	}
	text[*i] = '\0';
	free(i);
	return text;
}

int main(int argc, char *argv[])
{
	int o = 0;
	int optionIndex = 0;
	enum print printTitle = PRINT_NO;
	char *lyrics, *trimmedLyrics;
	static struct option long_options[] = {
		{ "title", no_argument, 0, 't' },
		{ "directive", no_argument, 0, 'd' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};
	while ((o = getopt_long(argc, argv, "td", long_options, &optionIndex)) != -1) {
		switch(o) {
			case 't':
				printTitle = PRINT_TITLE;
				break;
			case 'd':
				printTitle = PRINT_TITLE_DIRECTIVE;
				break;
			case 'h':
				printHelp();
				return 0;
		}
	}
	if (argc == optind)
	{
		lyrics = extractLyrics(0, printTitle);
		trimmedLyrics = trim(lyrics);
		printf("%s\n", trimmedLyrics);
		free(trimmedLyrics);
	}
	else
	{
		int fd;
		for (int i=optind; i<argc; i++)
		{
			fd = open(argv[i], O_RDONLY);
			if (fd == -1)
			{
				fprintf(stderr, "opening '%s' failed.\n", argv[i]);
				continue;
			}
			lyrics = extractLyrics(fd, printTitle);
			trimmedLyrics = trim(lyrics);
			printf("%s\n", trimmedLyrics);
			free(trimmedLyrics);
		}
	}
	return 0;
}
