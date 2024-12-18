# Your first via script
To write your first via script, make a file in a prefered directory ending with the `.via` extension.
Your directory should look something like this:
```
├── directory
└── file.via
```
Open the `.via` file you've just created and paste in this code:
```lua
println("Hello, world!")
```
Now close the file and open up your preferred terminal, and type in this command:
```bash
$ via file.via
```
If you did everything correctly, the output should be:
```bash
$ via file.via
Hello, world!

VM exiting with exit_code=0 exit_message='' 
```
And congurats, you've wrote your first program in via!

Next page: [Basic syntax](basic_syntax.md)