pub fn print_help_instructions()
{
	eprintln!("Help Instructions");
	eprintln!("\tStarting Point");
	eprintln!("\t\tThis is a program to reveal directory entries and file contents.");
	eprintln!("\tSyntax");
	eprintln!("\t\tUse this program with following syntax:");
	eprintln!("\t\t\treveal [flags] <path>");
	eprintln!("\t\tThe flags it can accept are:");
	eprintln!("\t\t\t--help: print these help instructions.");
	eprintln!("\t\tIf no path is provided, it will consider your current directory.");
	eprintln!("\t\tIf multiple paths are provided, only the last one will be considered.");
}

