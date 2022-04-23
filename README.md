# Information

This branch contains 5 files which I started to learn about File Management System Calls (`open`, `close`, `read`, `write`).

| File                    | Description                                                                                                                                                                                     |
|-------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `CheckFilePermission.c` | This checks the permissions a file have (existence, read access, write access). <br><br>Use: `./<exename> <filename>`                                                                           |
| `CreateAndOpen.c`       | This creates a file with the specified filename and opens it - this is to understand the `open` system call. <br><br>Use: `./<exename> <filename>`                                              |
| `DisplayContent.c`      | This takes a text file and displays its contents and returns appropriate messages if the file doesn't exist or can't be opened. <br><br>Use: `./<exename> <filename>`                           |
| `CopyFile.c`            | This mimics the `cp` command with implementation using `open()`, `read()`, and `write()`. <br><br>Use: `./<exename> <source filename> <destination filename>`                                   |
| `ReadAndReplace.c`      | Given some text file, we read 100 characters at a time and replace '1's with 'L's and write the updated text to a new file. <br><br>Use: `./<exename> <source filename> <destination filename>` |
