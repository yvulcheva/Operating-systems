#!/bin/bash

#function getting the names of the first N players who have been placed on 1 to M places
function top_places(){

	 cat "$1" | grep "$2" | sort -n -t ':' -k3 | awk -F ':' '{print $3,$4}' | awk "/^[1-"$3"] /" |
		 awk -F" " '{print $2,$3}' | sort -d | uniq -c | sort -k1,1nr -k2,2 | head -"$4" 

}
#function getting the category and the dates for particular player
function parts(){

	#list with all categories all dates sorted by dates
	ALL_CAT_DATES=$(cat "$1" | grep "$2" | awk -F':' '{print $2, $1}' | sort -t '.' -k1,1n -k2,2n -k3,3n)
	echo "$ALL_CAT_DATES" | awk '{a=$1; sub(a,","); S[a]=S[a] $0} END { for(i in S) print i S[i]}' | sed 's/,//' 
	
}

#if the parameters are 5
if [[ $# -eq 5 ]]; then
	if [[ -f "$1" ]] && [[ "$2" == "top_places" ]] && [[ "$4" =~ ^[1-9][0-9]* ]] && [[ "$5" =~ [1-9][0-9]* ]];then

		#list with all categories collected from the file - parameter
		CATEGORIES=$(cat $1 | awk -F':' '{printf $2"\n"}' | sort | uniq )

		if [[ $(echo "$CATEGORIES" | grep "$3") == "$3" ]];then

			top_places "$1" "$3" "$4" "$5" 
			exit 0	
		else
			echo "There is no $3 category"
			exit 1
		fi
	else 
		echo "Needs: file, top_places, category, number, number  OR  file, parts, name"
		exit 1
	fi
#else if the parameters are 3
elif  [[ $# -eq 3 ]]; then
	
	if [[ -f "$1" ]] && [[ "$2" == "parts" ]]; then

		#list with names collected from the file - parameter 
		NAMES=$(cat "$1" | awk -F':' '{print $4}' | sort -d -u)

		if [[ $(echo "$NAMES" | grep "$3") == "$3" ]]; then
			
			parts "$1" "$3"
			exit 0

		else
			echo "No such name in the list of names"
			exit 1
		fi
	
	else
		echo "Needs: file, top_places, category, number, number  OR  file, parts, name"
		exit 1
	fi
#else print message
else
	echo "Needs: file, top_places, category, number, number  OR  file, parts, name"
	exit 1
fi

