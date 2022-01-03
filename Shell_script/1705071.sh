#!/bin/bash

if (($# == 2)) ; then
	directory=$1
	if [[ ! -d "$1" ]]; then
		echo "No such directory"
		proceed=0
	else
		if [[ -f "$2" ]]; then
			echo "file found"
			filename=$2
			proceed=1
		else
			echo "no such file"
			proceed=0
		fi
	fi	
elif (($# == 1 )) ; then 
	if [[ -f "$1" ]]; then
		echo "file found"
		filename=$1
		directory="."
		proceed=1
	else
		echo "no such file"
		proceed=0
	fi
else
	echo "enter and file name  directory name(optional)"
	proceed=0
fi

if (( proceed == 1 )) ; then
	declare -a invalid_filetypes

	readarray -t invalid_filetypes <"$filename"

	for i in "${invalid_filetypes[@]}";
	do
		echo $i
	done

	touch allfiles.txt
	find $directory -type f >"allfiles.txt"

	readarray -t filedirectories <"allfiles.txt"

	mkdir out
	mkdir out/others
	touch out/others/desc_others.txt

	mkdir out/ignored
	touch out/ignored/desc_ignored.txt

	declare -a valid_extensions
	declare -a invalid
	declare -a copied
	
	for i in "${filedirectories[@]}";
	do
		filename_with_extension="${i##*/}"
		filename="${filename_with_extension%.*}"
		extension="${filename_with_extension##*.}"
		
		#extension=$(cut -d. -f3 <<< $i)
		#echo $filename
		#extension=$(awk -F"." <<< $i)
		#extension=$(echo $i | awk -F. '{print $NF}')
		#echo $extension
		
		if [[ ! "${invalid_filetypes[*]}" =~ "${extension}" ]]; then
			if [[ ! "${valid_extensions[*]}" =~ "${extension}" ]]; then
				if [[ "$extension" != "$filename" ]]; then
					valid_extensions+=($extension)
					mkdir out/"$extension"
					touch out/"$extension"/desc_"$extension".txt
					#echo $extension
				else
					invalid+=($extension)
				fi
				
			fi
			if [[ ! $i = *"allfiles.txt" ]]; then
				if [[ "$extension" != "$filename" ]]; then
					cp "$i" out/"$extension"
					echo $i >> out/"$extension"/desc_"$extension".txt
					copied+=($i)
				fi
			fi
		else
			if [[ ! $extension = "" ]]; then
				if [[ "$extension" != "$filename" ]]; then
					cp "$i" out/ignored
					copied+=($i)
					echo $i >> out/ignored/desc_ignored.txt
					#echo $i
				fi
			fi
		fi
	done
	
	for i in "${filedirectories[@]}";
	do
		if [[ ! "${copied[*]}" =~ "${i}" ]]; then
			if [[ ! $i = *"allfiles.txt" ]]; then
				cp "$i" out/others
				echo $i >> out/others/desc_others.txt
			fi
		fi
	done

	# csv file 

	echo "type of files,number of files" >> out.csv

	for i in "${valid_extensions[@]}";
	do
		lines=`wc --lines < out/"$i"/desc_"$i".txt`
		echo "$i,$lines" >> out.csv
	done

	lines=`wc --lines < out/others/desc_others.txt`
	echo "others,$lines" >> out.csv

	lines=`wc --lines < out/ignored/desc_ignored.txt`
	echo "ignored,$lines" >> out.csv

	rm allfiles.txt
	
	rm -r out/ignored

	#cat out.csv

fi
