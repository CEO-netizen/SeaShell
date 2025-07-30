# SeaShell - Beta Shell in C

This is a beta version of a simple shell coded in C.

## Features

- Displays a prompt `SeaShell>`
- Reads user input from the command line
- Parses commands and arguments
- Executes external commands using `fork` and `execvp`
- Supports the built-in `exit` command to quit the shell
- Handles EOF (Ctrl+D) gracefully
- Basic error handling for invalid commands

## Build Instructions

To build the shell executable, run the following command in the root directory:

```
make
```

This will compile the source code and create an executable named `seashell`.

## Usage

Run the shell by executing:

```
./seashell
```

You will see the prompt `SeaShell>`. Enter commands as you would in a typical shell.

To exit the shell, type:

```
exit
```

or press `Ctrl+D`.

## Notes

This is a minimal beta version intended for testing and further development.
