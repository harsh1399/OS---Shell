# OS---Shell

Simple shell terminal, which can handle following commands - 
1. Commands like ls, echo etc. Ex - 
```
ls /home
```
2. I/O redirection commands - used for changing standard input/output of the command. Ex - 
 ```
 echo "utsh is cool" > x.txt
 ```

3. Pipe - Allows inter-process communication. Ex -
```
ls | sort | uniq | wc
```

4. Change directory - ex -
```
cd ../hw1
```

5. History -  displays a history of the commands entered in the current session of shell invocation.

6. List of commands separated by ; - ex - 
```
ls ; echo "hi" ; cat < x.txt
```

7. Run command in background using & - run a command in the "background" by appending a & at the end. The command is then run as a job, asynchronously. ex - 
```
cat < input.txt &
```
