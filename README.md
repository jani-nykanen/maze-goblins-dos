![](https://media.githubusercontent.com/media/jani-nykanen/maze-goblins-dos/main/misc/logo_git_out.png)

## The Maze of DOS Goblins

This was supposed to be a DOS port of my js13k 2021 entry. And in a way, it is. I mean, yes, it has all the features it was supposed to have, you can play through it, and after some violent bug fixing it even has a decent performance and the bugs are mostly visual. But... eh, well, I forgot my point. 

-------

### Downloading and playing

Go here https://github.com/jani-nykanen/maze-goblins-dos/releases/tag/v1.0.0 and download. You need [DOSBox](https://www.dosbox.com/) to play this. **Enable aspect ratio correction on DOSBox config file before playing**, otherwise things might look a bit... uh, flat?

-------

### About the code

The code is bad, but in a similar way all the code is bad: you have something broken, you fix it. Then later your fix breaks apart, so you made another fix. And you keep fixing your fixes until you have a tasty meal of spaghetti, which, for some weird reason, actually works. At least most of the time.

I guess one can use my code to learn how to make DOS games since finding good information on DOS programming in C is very, *very* hard. But... I think there are much better ways to learn to write a DOS game in C. However, feel free to use "my" keyboard routine. It's based on a piece of code I found on Stack Overflow after hours of googling, and I never found it again. I have reused the same routines in most of my DOS projects.

The code has some weird parts, like a component called "Window". I was originally supposed to port the game to modern devices using SDL2, but that never happened.

-------

### Building

Now this is the tricky part. See the `makefile`? It does not really help, you can use it only to build the assets (run `make tools assets`). To build the game (on Linux), follow the following steps:

0) (Optional) Run `make tools assets` to build the tools required to convert the assets to the correct format, and then convert the assets afterwards. Only works on Linux for now, might work on Windows with MSYS and MinGW or something like that.
1) Download the newest version of OpenWatcom (take the Windows version to get the IDE, use Wine on Linux). Make sure to have 32-bit DOS C toolchain.
2) Create a `bin` folder and create a new 32-bit DOS32A LE project there (make sure to choose LE, not sure if LX works)
3) Add all the files from the `src` folder to the project.
4) Compile.

It should compile without errors (only some warnings). You can enable some optimization flags if you desire, I did not use them to build the "official" binary, though. Run `make dist` to make a distributable zip package (or just find the essential files, your `bin` folder most likely has all those project files)

--------

### License

Assets: Attribution CC BY
Code: MIT (not that I really care what you do with my code since if you use it in your own code, you do more harm to yourself than to me...)

--------


(c) 2021-2022 Jani Nykänen


