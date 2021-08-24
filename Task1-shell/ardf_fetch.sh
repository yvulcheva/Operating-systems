#!/bin/bash

if [[ $# -ne 1 ]]; then 
	echo "Needs one file to read from"
	exit 1
fi

if [[ ! -f "$1" ]];then
	echo "The file must exist"
	exit 1
fi

grep -v '^#' $1 | while read line; do
	#file with the current html code
	URL_OUTPUT=$(curl -s $line)
	FILE_HTML_CODE=$(mktemp HTML_CODEXXX)
	echo "$URL_OUTPUT" > $FILE_HTML_CODE

	#DATE
	DATE=$(cat $FILE_HTML_CODE | awk '/[0-9][0-9]?\.[0-9][0-9]\.[0-9][0-9][0-9][0-9]/ {print $6}')

	#if the second symbol is '.' add zero
	#format DD.MM.YYYY	
	if [[ $(echo ${DATE:1:1}) == . ]]; then
	       DATE="0$DATE"	
	fi

	#CATEGORIES
	FILE_CATEGORIES=$(mktemp CATEGORIESXXX) 
	CATEGORIES=$(cat $FILE_HTML_CODE | awk -F '<td>' '/<td>[A-Z][0-9][0-9]/ {for(i=3;i<=13;i++) printf $i"\n"}')
	echo "$CATEGORIES" > $FILE_CATEGORIES
	
	#getting only the info between >< symbols where the actual text is presented in html code
	#then getting lines which have 'ResLine' class where the information we need is written
	#then doing some manipulation on the text
	RAW_TEXT=$(mktemp TEMP_TEXTXXX)

	RAW_OUTPUT=$(cat $FILE_HTML_CODE | grep 'ResLine' | tail +2 |
	       	sed 's/<[^>]*//g'| sed 's/>>/>/g' | sed 's/\.//g'|
		sed 's/^.//g' |sed 's/$..//g' |  sed 's/>/:/g' | sed 's/&nbsp;//g')

	echo "$RAW_OUTPUT" > $RAW_TEXT
	
	#bool value to check whether the line with first place is read 
	BOOL=true
	while read CATEGORY; do	
		while read -r LINE; do 
			
			if [[ "$LINE" == 1:* ]] && [[ $BOOL == false ]]; then
			    	break
			fi
			#PLACE
			PL=$(echo $LINE | awk -F ':' '{print $1}' | sed 's/ //g')
			#NAME - checks first if the format is Family, Name or Name Family
			if [[ $(echo $LINE | grep -o ',') == ',' ]];then
				NAME=$(echo $LINE | awk -F ':' '{print $2}' | awk -F', ' '{print $2,$1}')
			else
				NAME=$(echo $LINE | awk -F':' '{print $2}')
			fi
			#NATIONALITY
			DOK=$(echo $LINE | awk -F ':' '{print $3}')
			#CALL SIGN
                        CALL=$(echo $LINE | awk -F ':' '{print $4}')
			#TIME
		       	TIME=$(echo $LINE | awk -F ':' '{print $5}')
			#COUNT 
                        FOX=$(echo $LINE | awk -F ':' '{print $6}')
			#START NUMBER
		       	STNR=$(echo $LINE | awk -F ':' '{print $7}')
                        
			
			#print all the information we need
                   	echo "$DATE:$CATEGORY:$PL:$NAME:$DOK:$CALL:$TIME:$FOX:$STNR"

			BOOL=false
			#deletes the last read line
			sed -i "1 d" "$RAW_TEXT"
	
		done < $RAW_TEXT 
		BOOL=true
	
	done < $FILE_CATEGORIES	

	#clearing the temp files
	rm $FILE_HTML_CODE
	rm $FILE_CATEGORIES
	rm $RAW_TEXT
done


