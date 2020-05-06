#!/usr/bin/env bash

bold=$(tput bold)
normal=$(tput sgr0)

keeppath=0				# Determines if user wants to keep path across deletions
validsearch=0			# Determines if query exists in filesystem
validpath=0				# Determmines if path exists in filesystem
numfilesfound=0			# Keeps track of number of files found

: '
Catch-all function that handles all user-input
scenarios
'
userInputHandler() 
{
	# Handle empty input
	if [[ -z "$1" ]]; then
		printf ": No input entered. Try again.\n\n"

	# Handle quit
	elif [[ "$1" == "quit" ]]; then
		printf ": Exiting...\n"
		exit 0

	# Handle search query validation
	elif [[ $2 == "search" ]]; then
		if  [[ $1 == "back" ]]; then
			keeppath=0					# Reset choice
			pathprompt
		elif [[ -n $(find "$initpath" -type f -iname "*$1*") ]]; then
			validsearch=1															# Set search as valid
			numfilesfound="$(find "$initpath" -type f -iname "*$1*" | wc -l)"		# Count number of files found
			deletepath="$(find "$initpath" -type f -iname "*$1*" -printf "%h\n")"	# Save valid path
			filename="$(find "$initpath" -type f -iname "*$1*" -printf "%f\n")"		# Save full filename
		else
			printf ": File not found when searching. Try again.\n\n"
		fi

	# Handle path validation
	elif [[ $2 == "path" ]]; then					
		if [[  -d "$1" ]]; then
			validpath=1
		else	
			printf ": Path not found. Try again.\n\n"
		fi

	# Handle deletion choice
	elif [[ $2 == "delete" ]]; then	
		case $1 in
			Y | y)
				$(rm "$deletepath"/"$filename")
				case $? in
					0)
						printf ": %s successfully deleted.\n\n" "$filename"
					;;
					*)
						printf ": %s failed to delete. Error code %d.\n\n" "$filename" "$?"
					;;
				esac
				keeppath
			;;
			N | n)
				printf ": Not deleting ${bold}%s ${normal}\n\n" "$filename"
				keeppath
			;;
			"back")
				queryprompt
			;;
			*)
				printf ": Invalid choice. Try again.\n\n"
				deletefile
			;;
		esac
	
	# Handle keep path choice
	elif [[ $2 == "keep" ]]; then
		case $1 in
			Y | y)
				echo ": Continuing to search path $initpath"
				keepchoice=1
				keeppath
			;;
			N | n)
				printf "::: Please specify a new path ('quit' to exit, 'back' to go back): "
				read initpath
				userInputHandler $initpath "path"
			;;
			"back")
				echo ": Can't undo deletion of file if deleted."
				deletefile
			;;
			*)
				printf ": Invalid choice. Try again.\n\n"
				keeppath
			;;
		esac
	fi
}

deletefile() 
{
	# Only allow 1 file deletetion at a time
	if [[ $numfilesfound != 1 ]]; then
		echo ": Search not specific enough, more than one file found. Try again."
		validsearch=0
		searchvalidate
	fi

	# Show path to user to confirm/deny deletion for each file found
	printf ": Delete: %s in %s? ('quit' to exit, 'back' to go back) [Y/N]: " "$filename" "$deletepath"
	read deletechoice
	userInputHandler "$deletechoice" "delete"
}

keeppath() 
{
	validsearch=0
	case $keepchoice in
		1)
			queryprompt
			;;
		0 | *)
			printf "Keep searching specified path? ('quit' to exit, 'back' to go back) [Y/N]: "
			read keepchoice
			userInputHandler $keepchoice "keep"
			;;
	esac
}

pathprompt() 
{
	printf "::: Path to search (E.g. /, /home/, etc.) ('quit' to exit)?: "
	read initpath
	userInputHandler "$initpath" "path"	
}

queryprompt() 
{
	printf "::: Enter the name of the file you would like to delete ('quit' to exit, 'back' to go back): "
	read query
	userInputHandler "$query" "search"
}

# While path is blank or is not valid, keep looping unless user wants to quit
pathvalidate() {
	while [[ $validpath -eq 0 ]]; do
		pathprompt
	done
}

# While query is blank or DNE, keep looping unless user wants to quit
searchvalidate() {
	while [[ $validsearch -eq 0 ]]; do
		queryprompt
	done
	
	# Show amount of files found
	echo ": Found $numfilesfound file(s) matching your search."
	
	# Start delete file routine
	deletefile
}

# Unless user hasn't typed 'quit', continue to prompt user to delete files
while true; do	
	pathvalidate
	searchvalidate
done
exit 0
