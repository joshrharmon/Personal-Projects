# Batch Delete Bash Script
####
A simple script to continuously delete specific files in a Linux filesystem.

## How it works
A flow chart of the process:
<img src="https://github.com/joshrharmon/Personal-Projects/blob/master/Bash/BatchDelete/flowchart.png" width="1000">
## Features
* Numerous safeguards to prevent accidental deletion
* Validates path and file existence before deletion
* Ability to save search path to avoid re-entering it upon every deletion
* Ability to go back during all steps in the deletion process
* Informative output that shows every step of the process
* Utilizes fast find command to search for files
  * Ignores case
  * Will match files that contain any part of the query
    * e.g. FooBar.txt will be found if "oob" is searched

## Why would I use this?
I designed this script for my personal use when I wanted to delete a file but did not know its exact location. I ended up  using the find command along with the delete switch to delete specific files but found it to be both tedious and not informative as to what was happening during the process. The find command does not output whether file deletion was successful or not on the standard output as well as which file was deleted. It also does not provide a prompt for deletion nor does it show which file it deleted (if it did delete it). I wanted to implement all of these features in an easy and straightforward script to make the entire process easier as well as have the ability to continuously delete files. 

## TODO
- [ ] Add ability to delete multiple files at once if query returns more than one result
- [ ] Add colors to make output more readable
- [ ] Show output of all files deleted in session on exit
