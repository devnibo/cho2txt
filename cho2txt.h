enum print
{
	PRINT_NO,
	PRINT_TITLE,
	PRINT_TITLE_DIRECTIVE
};

enum direc
{
	DIREC_TITLE,
	DIREC_GRID_START,
	DIREC_GRID_END,
	DIREC_TAB_START,
	DIREC_TAB_END
};

struct directive
{
	enum direc type;
	const void *names;
};

static const char *title[] = { "title:", NULL };
static const char *gridStarts[] = { "start_of_grid", NULL };
static const char *gridEnds[] = { "end_of_grid", NULL };
static const char *tabStarts[] = { "start_of_tab", "sot", NULL };
static const char *tabEnds[] = { "end_of_tab", "eot", NULL };

static const struct directive dirs[] = {
	{ DIREC_TITLE, title },
	{ DIREC_GRID_START, gridStarts },
	{ DIREC_GRID_END, gridEnds },
	{ DIREC_TAB_START, tabStarts },
	{ DIREC_TAB_END, tabEnds },
};
