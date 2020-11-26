struct output_format {
	char *name;
	format_writer writer;
};

bool parse_flag(const char *flag);

