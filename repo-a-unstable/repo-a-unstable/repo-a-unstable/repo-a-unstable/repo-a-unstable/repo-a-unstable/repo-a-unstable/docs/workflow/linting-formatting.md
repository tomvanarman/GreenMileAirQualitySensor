# Linting & Formatting

The repository contains linting and formatting tools so all code and documentation follows the same
guidelines.

It is **required** that all linters pass wihtout warnings / errors before code is merged.

However, if you ever do need to disable the linters and formatters you can prepend `SKIP=flake8` to
your git command

```
SKIP=flake8 git commit -m "My message"
```

## How to setup?

**Reccomended:** Install the
[Prettier](https://marketplace.visualstudio.com/items?itemName=esbenp.prettier-vscode) vscode
extension and enable auto format.

Install[`pre-commit`](https://pre-commit.com/). This is a tool that can execute code _before_ a
commit is done (`git commit` command). We use this to run formatting and linting automatically.

`pre-commit` is included so it should be automatically installed if you installed the requirements
from `requirements.txt`.

You can run the command below to install pre-commit locally.

```
pre-commit install
```

If `pre-commit` is installed yet you can install it with pip.

```
pip install pre-commit
```

> **Note:** it could take some time on your first git commit to install all the dependencies

## What to do if a check fails during your git commit

If a check shows that if fails when you do a `git commit`, fix the errors that are shown. (Note that
a check will fail if a fail was changed)

Then add all changes again `git add .` and execute your `git commit` command again.

## What tools are used?

In order the following tools are used:

1. check yaml syntax, make all files have the same line endings and remove trailing whitespace
2. Format and lint python code
3. Format backend code and markdown using Prettier
4. Use `eslint` to check typescript code
5. Use `clang-format` to format c++ code according to google standards
6. Use `cpp-lint` to check code according to google standards.

Each entry in the list is called a "stage"

## Configuration

Prettier is configured in the `.prettierrc` dotfile.

cpplint rules are configured in `CPPLINT.cfg`

eslint rules are configured in `web/eslint.config.mts`

## Run linters locally

You can run any stage of `pre-commit` individually and locally.

Here are all the commands:

**Check yaml:**

```
pre-commit run check-yaml --all-files
```

**End of file fixer:**

```
pre-commit run end-of-file-fixer --all-files
```

**Fix trailing white space:**

```
pre-commit trailing-whitespace --all-files
```

**Python formatter**

```
pre-commit run black --all-files
```

**Format using prettier:**

```
pre-commit run prettier-format --all-files
```

**Clang format:**

```
pre-commit run clang-format --all-files
```

**C++ linter**

```
pre-commit run cpplint --all-files
```
