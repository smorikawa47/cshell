# cshell

## Built-in commands
cshell comes with 4 built-in commands:
- **print**
  - The user can print either **words** typed in command line or **environment variable** (will be covered in other demo)
- **theme**
  - The user can **change the colour** of the text
  - cshell supports 3 colours: red, green, and blue
- **log**
  - The user can see the **commands history**
  - 0 indicates valid a command, and 1 indicates an invalid command
- **exit**

https://user-images.githubusercontent.com/80607636/176270969-baa490c4-f1b3-4ce3-a35c-46ab035bcff1.mov

## Non-built-in commands
cshell also supports non-built-in commands such as **ls**, **pwd**, **whoami**, and these commands are executed using **execvp**

https://user-images.githubusercontent.com/80607636/176270941-4e55bfc8-62e2-4fa6-b1cf-75e326ce3722.mov

## Environment variable
cshell allows the user to **define** and **update** an environment variable. Once an environment variable is defined, the user can print it using the print command followed by the environment variable name

https://user-images.githubusercontent.com/80607636/176270880-df58f44a-cf31-413e-87c1-80687febae0f.mov

## Script mode
cshell allows the user to provide commands using **txt file**

For example, myscript.txt contains following commands
```
$VAR=foo
print $VAR
pwd
whoami
theme green
log
```
And by typing `./"name of the executable" myscript.txt`, cshell executes the commands written on myscript.txt

https://user-images.githubusercontent.com/80607636/176270821-ff9f4d64-af0d-4626-9af2-f2ba49cfd85a.mov
