#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "cho2txt.h"

void printHelp()
{
	static const char help[] = "Usage: cho2txt [-t] [-d] [FILE]...\n"
		"Extract lyrics from chordpro files.\n\n"
		"  -t, --title\t\tPrint title\n"
		"  -d, --directive\tPrint title as chordpro directive\n"
		"  -h, --help\t\tPrints help information\n"
		"  -v, --version\t\tPrint program version to stdout\n";
	printf("%s", help);
}

bool isDirective(enum direc d, const char *str)
{
	bool isDirective = false;
	size_t len;
	int i = 0;
	int k = 1;
	char **names = (char **)dirs[d].names;
	while (names[i] != NULL)
	{
		isDirective = true;
		len = strlen(names[i]);
		while (k <= len)
		{
			if (names[i][k-1] != str[k])
			{
				isDirective = false;
				break;
			}
			k++;
		}
		if (isDirective)
			return true;
		k = 1;
		i++;
	}
	return false;
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

char *extractLyrics(int fd, enum print printTitle)
{
	char *text = malloc(sizeof(char));
	int i = 0;
	int d = 0;
	char buf;
	bool isLyric = true;
	bool isLyricInLine = false;
	bool isDirectiveInLine = false;
	bool isCurlyBrace = false;
	char *directive = NULL;
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
				text[i] = buf;
				i++;
				text = realloc(text, (i+1) * sizeof(char));
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
				if (
					isDirective(DIREC_GRID_START, directive) ||
					isDirective(DIREC_TAB_START, directive)
				)
					isLyric = false;
				else if (
					isDirective(DIREC_GRID_END, directive) ||
					isDirective(DIREC_TAB_END, directive)
				)
					isLyric = true;
				if (printTitle > 0 && isDirective(DIREC_TITLE, directive))
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
						text[i] = title[k];
						i++;
						text = realloc(text, (i+1) * sizeof(char));
					}
					free(title);
					text[i] = '\n';
					i++;
					text = realloc(text, (i+1) * sizeof(char));
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
	text[i] = '\0';
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
		{ "version", no_argument, 0, 'v' },
		{ 0, 0, 0, 0 }
	};
	while ((o = getopt_long(argc, argv, "tdhv", long_options, &optionIndex)) != -1) {
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
			case 'v':
				printf("%.1f\n", VERSION);
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
