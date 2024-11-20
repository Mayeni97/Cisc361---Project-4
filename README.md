# README for DESH Shell Extension

## Project: Programming in the Shell

### Overview
This project enhances the **DESH shell** to support features that turn it into a limited programming language. It includes executing scripts, environment variable handling, conditional execution, and an accumulator functionality.

---

### Objectives
1. **Extend DESH shell to support script files**.
2. **Enable environment variable substitution**.
3. **Introduce conditional execution** using the `$?` variable.
4. **Implement an accumulator variable (`$ACC`)** for arithmetic operations.
5. Support **silent execution** with the `NOECHO` environment variable.

---

### Key Features

#### 1. **Required Existing Features**  
Ensure the following work as per the original shell project:
- Command parsing and execution via absolute paths.
- Built-in commands: `setenv`, `printenv`, `exit`.
- Handling of `exit` with exit codes.

#### 2. **Script Execution**  
- Accept a file as a command-line argument and execute its commands line by line.
- No prompts should appear during script execution.
- Stop on `exit` or EOF. The shell exit code will match the last executed command's exit code.

#### 3. **Environment Variable Substitution**  
- Replace `$VAR` with its value. If undefined, substitute with an empty string.
- `$0`: Substitutes the name of the shell executable.
- `$?`: Substitutes the exit code of the previous command.
- Works within built-in commands like `setenv`.

#### 4. **Accumulator Variable (`$ACC`)**  
- `$ACC` starts at `0` and is exported by default.
- `addacc` command:
  - Adds the given integer to `$ACC`.
  - Defaults to adding `1` if no argument is provided.
  - Treats non-integer values as `0`.

#### 5. **Conditional Execution**  
- Lines starting with `?` execute only if `$?` equals `0`.

#### 6. **Silent Execution**  
- If `NOECHO` is set and non-empty, suppress the "executing command" output.

---

### Usage Examples

#### **Script Execution**  
```bash
./desh script.desh
```

#### **Environment Variable Substitution**  
```bash
$ VAR=hello
$ echo $VAR       # Output: hello
$ unset VAR
$ echo $VAR       # Output: 
```

#### **Accumulator (`$ACC`)**  
```bash
$ echo $ACC       # Output: 0
$ addacc 5
$ echo $ACC       # Output: 5
$ addacc
$ echo $ACC       # Output: 6
```

#### **Conditional Execution**  
```bash
$ echo $?         # Output: 0
$ ? echo "This runs"  # Runs if $? is 0
$ false
$ ? echo "This won't run"
```

#### **Silent Mode**  
```bash
$ export NOECHO=1
$ echo hello      # Executes silently
```

---

### Notes
- Recursive Fibonacci computation examples are provided in `fibstart.sh` and `fib.sh`.
- **Rubric Total**: 100 points  
  - Exit code handling: 10 points  
  - `NOECHO` variable: 10 points  
  - Variable substitution: 20 points  
  - Accumulator `$ACC`: 20 points  
  - Conditional execution: 20 points  
  - Script execution: 20 points  

---

### Development Tips
- **Keep changes modular** to minimize errors.
- **Test frequently** to ensure correctness after implementing new features.
- Use `git` commands (`add`, `commit`, `push`) to track and back up your progress.

---

Good luck! 🎉
