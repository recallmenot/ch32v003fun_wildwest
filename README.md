# purpose
This repository is supposed to be an easy point of entry to contributing to `ch32v003fun`.  
Here we can experiment and publish, even if the code isn't perfect.  
Some of us lack knowledge and / or experience, others have it and can help us improve.  
This is the right place for you if you:
 * are happy that you just got something to work
 * want to learn
 * want to contribute
 * don't have two degrees and a doctorate in embedded engineering
 * want to experiment with something outside of core functionality

"wildwest" because commit access will be handed out to anyone who asks, so all of us have to make it work by acting in good faith.

Nothing on here is set in stone, your code may be improved / rewritten at any time.

# install
this repository is supposed to be located side-by side with ch32v003fun and we will access it through a symbolic link.
You'll want to work from the ch32v003fun dir write all paths from there.

## clone
`git clone https://github.com/recallmenot/ch32v003fun.git`
`git clone https://github.com/recallmenot/ch32v003fun_wildwest.git`

## symlink
To compile and upload, we need to be able to work from within the ch32v003fun directory.
linux / macos:
`ln -s ch32v003fun_wildwest/proj ch32v003fun/wildwest_proj`  
`ln -s ch32v003fun_wildwest/lib ch32v003fun/wildwest_lib`

Windows:
`mklink /D ch32v003fun\wildwest_proj ch32v003fun_wildwest\proj`  
`mklink /D ch32v003fun\wildwest_lib ch32v003fun_wildwest\lib`


# contribute
Before you write anything, have a look at the guildelines, instructions and templates in the [contribute dir](https://github.com/recallmenot/ch32v003fun_wildwest/tree/main/contribute).

## own code
Any code you provide is welcome if it works; be it a project, library or instructions.

## improvements
Any improvements to other peoples code need to be explained in an easy-to-understand manner.

Large modifications to other peoples code should also state the principles by which the code was improved, which you may also add to the instructions in the contribute dir if helpful to others.
