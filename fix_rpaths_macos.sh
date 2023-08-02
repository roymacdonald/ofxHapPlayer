#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

change_id(){
	
	install_name_tool -id "@executable_path/$( basename $1)" $1
}

fix_rpath() {
	if [ "$#" -eq 2 ]; then
	OLD="$1"
	NEW="@executable_path/$( basename "$OLD")"
	echo "----------------------"
	echo "lib: $2" 
	echo "OLD: $OLD"
	echo "NEW: $NEW"
	echo "======================"
	install_name_tool -change $OLD $NEW $2

	fi
}

check(){
	echo " --- $1 ----"
	otool -l $1 | grep -Z -o -C4 --color "@executable_path.//.*\.dylib"
}


find_rpaths_and_fix(){
	
	change_id $1

	echo "Checking $1"
	 for rp in $(otool -l $1 | grep -Z -o "@rpath/.*\.dylib")
	 do 
	 	fix_rpath "$rp" "$1"

	 done
}


 find_rpaths_and_fix libs/ffmpeg/lib/osx/libswresample.3.9.100.dylib
 find_rpaths_and_fix libs/ffmpeg/lib/osx/libavcodec.58.134.100.dylib
 find_rpaths_and_fix libs/ffmpeg/lib/osx/libavutil.56.70.100.dylib
 find_rpaths_and_fix libs/ffmpeg/lib/osx/libavformat.58.76.100.dylib
 find_rpaths_and_fix libs/snappy/lib/osx/libsnappy.1.1.9.dylib


 # check libs/ffmpeg/lib/osx/libswresample.3.9.100.dylib
 # check libs/ffmpeg/lib/osx/libavcodec.58.134.100.dylib
 # check libs/ffmpeg/lib/osx/libavutil.56.70.100.dylib
 # check libs/ffmpeg/lib/osx/libavformat.58.76.100.dylib
 # check libs/snappy/lib/osx/libsnappy.1.1.9.dylib