struct output_format {
	char *name;
	format_writer writer;
};

void parse_flag(const char *flag);

