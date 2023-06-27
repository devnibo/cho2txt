#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

char *extractLyrics(int fd)
{
	char *text = malloc(sizeof(char));
	int i = 0;
	char buf;
	bool isLyrics = true;
	bool isBracket = false;
	while (1)
	{
		if (read(fd, &buf, 1) == 1)
    {
			if (buf == '{')
				isLyrics = false;
			if (buf == '[')
				isLyrics = false;
			if (isLyrics)
			{
				if (buf == '\n' && isBracket)
				{
					isBracket = false;
					continue;
				}
				text[i] = buf;
				i++;
				text = realloc(text, (i+1) * sizeof(char));
			}
			if (buf == '}' || buf == ']')
			{
				isLyrics = true;
				isBracket = true;
			}
			else
				isBracket = false;
		}
		else
			break;
	}
	text[i] = '\0';
	return text;
}

char *trimText(char *text)
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

int main(int argc, char *argv[])
{
	char *lyrics, *trimmedLyrics;
	switch (argc)
	{
		case 1:
			lyrics = extractLyrics(0);
			trimmedLyrics = trimText(lyrics);
			printf("%s\n", trimmedLyrics);
			free(trimmedLyrics);
			break;
		case 2:
			int fd = open(argv[1], O_RDONLY);
			if (fd == -1)
			{
				fprintf(stderr, "open failed.\n");
				return -1;
			}
			lyrics = extractLyrics(fd);
			trimmedLyrics = trimText(lyrics);
			printf("%s\n", trimmedLyrics);
			free(trimmedLyrics);
			break;
		default:
			fprintf(stderr, "Either provide exactly one file or no file for reading from stdin.\n");
			return -1;
	}
	return 0;
}
