use std::
{
	env::args,
	path::PathBuf,
	process::exit
};
use super::pretty_printing::print_error;

pub struct ArgumentsParser
{ arguments: Vec<String> }

impl ArgumentsParser
{
	pub fn from_environment() -> ArgumentsParser
	{ ArgumentsParser { arguments: args().collect() } }

	fn has_enough_arguments(&self) -> bool
	{
		const DEFAULT_ARGUMENTS_LENGTH: usize = 1;
		self.arguments.len() > DEFAULT_ARGUMENTS_LENGTH
	}

	pub fn is_to_show_help(&self) -> bool
	{
		self.arguments.contains(&String::from("-h")) ||
		self.arguments.contains(&String::from("--help"))
	}

	pub fn get_path(&self) -> PathBuf
	{
		let last_argument_index: usize = self.arguments.len() - 1;
		let path: PathBuf =
		if self.has_enough_arguments()
		{
			PathBuf::from(self.arguments[last_argument_index].clone())
		}
		else
		{
			PathBuf::from(".")
		};
		path
			.canonicalize()
			.unwrap_or_else(
				|_error|
				{
					print_error(String::from("The path does not exists."));
					exit(1);
				}
			)
	}
}

