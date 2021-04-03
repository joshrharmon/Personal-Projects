#!/usr/bin/env bash

fileDelta=()
fileRemoval=()
fileCopy=()
sourceDir=""
targetDir=""
FLAC_EXT_LEN=5
MP3_EXT_LEN=4

# First arg is source, second is target
CCOPY() {
	if [[ $(cp -r "$1" "$2") -ne 0 ]]; then
		echo ":: Copy of $1 -> $2 FAILED"
	fi
}

# First arg is file, second are args for soxi
FFILE() {
	if [[ -z $(soxi $2 "$1") ]]; then
		echo ":: Inspecting file type of $1 FAILED"
	else
		echo $(soxi $2 "$1")
	fi
}

#First arg is file, second are args
GGREP() {
	if [[ -z $(grep $2 "$1") ]]; then
		echo ":: Grep on $1 with "$2" FAILED"
	else
		echo $(grep $2 "$1")
	fi
}

validDirs() {
	if [[ -d "$sourceDir" && -d "$targetDir" && -r "$sourceDir" && -r "$targetDir" && -w "$sourceDir" && -w "$targetDir" && "$sourceDir" != "$targetDir" ]]; then
		echo ":: Directories valid. Scanning..."
		if [[ ${sourceDir:(-1)} == "/" ]]; then		# Remove trailing slash if present
			sourceDir="${sourceDir::-1}"
		fi
		if [[ ${targetDir:(-1)} == "/" ]]; then
			targetDir="${targetDir::-1}"
		fi
		return 0
	else
		echo ":: One or both directories invalid."
		echo ":: Both directories need to exist, be read/writable and not be the same."
		return 1
	fi
}

deltaStats() {
	if [[ ${#fileDelta[@]} -gt 0 ]]; then
		echo "File(s) to convert and transfer: "
		for ((i = 0; i < ${#fileDelta[@]}; i++)); do
			echo -e "-> ${fileDelta[$i]##*/}"
		done
		echo ":: ${#fileDelta[@]} file(s) will be converted and transferred."
	fi

	if [[ ${#fileCopy[@]} -gt 0 ]]; then
		echo "File(s) to copy over: "
		for ((i = 0; i < ${#fileCopy[@]}; i++)); do
			echo -e "-> ${fileCopy[$i]##*/}"
		done
		echo ":: ${#fileCopy[@]} file(s) will be transferred over."
	fi
	
	if [[ ${#fileRemoval[@]} -gt 0 ]]; then
		echo "Folder(s) to delete from target: "
		for ((i = 0; i < ${#fileRemoval[@]}; i++)); do
			echo -e "-X ${fileRemoval[$i]##*/}"
		done
		echo ":: ${#fileRemoval[@]} folder(s) will be deleted from target."
	fi
	
	echo -n "Are you sure you would like to continue? [Y/N]: "
	read choice
	if [[ $choice =~ [Yy] ]]; then
		return 0
	elif [[ $choice =~ [Nn] ]]; then
		return 1
	else 
		echo "Invalid choice, re-run program."
		return -1
	fi
}

syncCommit() {

	# Remove files present at target but not at source
	for ((i = 0; i < ${#fileRemoval[@]}; i++)); do
    	if [[ $(rm -rf "${fileRemoval[$i]}") -eq 0 ]]; then
			echo ":: Successfully deleted ${fileRemoval[$i]}."
		else
			echo ":: Deletion of ${fileRemoval[$i]} FAILED!"
		fi
    done
    
    # Convert FLAC files to MP3 V0 
    for ((j = 0; j < ${#fileDelta[@]}; j++)); do

    	# Check if output directory exists. If not, create it.
    	regexPath=$(echo "${fileDelta[$j]}" | grep -o '^\/\(.\+\/\)*')
    	existCheck="${targetDir}${regexPath:sourceLen}"
    	if [[ ! -e $existCheck ]]; then
    		if [[ $(mkdir -p "$existCheck") -ne 0 ]]; then
    			echo ":: Creation of missing directory FAILED."
    		fi
    	fi
    	
    	# Check if file is a FLAC or not. If not, simply copy it over.
    	if [[ $(echo "${fileDelta[$j]}" | grep -o -E 'flac') != "flac" ]]; then
    		echo ":: Will copy over ${fileDelta[$j]##*/}"
    		CCOPY "${fileDelta[$j]}" "$existCheck"
    		continue
    	fi
    	
		echo ":: Converting ${fileDelta[$j]##*/} to MP3 V0"
    	
    	if [[ $(pv "${fileDelta[$j]}" | ffmpeg -y -i pipe:0 -codec:a libmp3lame -q:a 0 -map_metadata 0 -id3v2_version 3 -write_id3v1 1 -loglevel fatal "${targetDir}${fileDelta[$j]:sourceLen:-FLAC_EXT_LEN}.mp3") -ne 0 ]]; then
			echo ":: Conversion of ${fileDelta[$j]} FAILED"
		fi
    done
}

dirClone() {
	sourceLen=${#sourceDir}
	targetLen=${#targetDir}
	for file in "$1"/*
	do
		case "$2" in
			"source")
			if [[ ! -d ${file} ]]; then

                : '
                If FLACs exist at source but the MP3 equivalent is not at target or the source is newer, add to queue
                Also, if the file exists on both ends, but the source is newer, the file will be re-converted to replace
                the older copy.
                '
				fileLook="${targetDir}${file:sourceLen:-FLAC_EXT_LEN}.mp3"
				if [[ $(echo "${file}" | grep -o -E 'flac') == "flac" ]]; then
					if [[ ! -e $fileLook || ${file} -nt $fileLook ]]; then
						fileDelta+=("${file}")
					fi
				else
					if [[ ! -e "${targetDir}${file:sourceLen}" ]]; then
						fileDelta+=("${file}")
					fi
				fi

			else
				dirClone "${file}" "source"
			fi
			;;

			"target")
			if [[ -d ${file} ]]; then

				# If MP3s exist at target, but the FLAC equivalent is not at source, add to delete queue
				folderLook="${sourceDir}${file:targetLen}"
				if [[ ! -e $folderLook ]]; then
					fileRemoval+=("${file}")
				fi
				
				dirClone "${file}" "target"	
			fi
			;;
		esac
	done
}

main() {
	if [[ ! -d "$1" ]] || [[ ! -d "$2" ]]; then
		echo "Usage: ./script [source path] [target path] (e.g. ./script /home/Music/ /home/MusicCopy/)"
		echo -n "Source directory: "
		read sourceDir
		echo -n "Target Directory: "
		read targetDir
	else
		sourceDir="$1"
		targetDir="$2"
	fi
	
	if validDirs; then
		 dirClone $sourceDir "source"
		 dirClone $targetDir "target"
		 if deltaStats; then
		 	syncCommit
		 	echo ":: Job complete."
		 else
		 	echo "Exiting..."
		 	exit
		 fi
	else
		exit	
	fi
}

main $1 $2
