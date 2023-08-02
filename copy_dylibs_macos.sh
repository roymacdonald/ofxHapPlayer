#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_PATH="$( pwd )"
PROJECT_NAME="$( basename "$PROJECT_PATH" )"


# echo "PROJECT_PATH $PROJECT_PATH"
echo "Project $PROJECT_NAME"
echo "ofxHapPlayer path: $SCRIPT_DIR"
copy_lib(){
	echo "Copying lib $( basename $1 ) "
	rsync -avz --exclude='.DS_Store' $1 $2
	# cp $1 $2
}
for app in $( find $PROJECT_PATH/bin -type d -name "$PROJECT_NAME*.app")
do
	
	# copy_lib $SCRIPT_DIR/libs/ffmpeg/lib/osx/  $app/Contents/MacOS/
	# copy_lib $SCRIPT_DIR/libs/ffmpeg/lib/osx/  $app/Contents/MacOS/
	# copy_lib $SCRIPT_DIR/libs/ffmpeg/lib/osx/  $app/Contents/MacOS/
	copy_lib $SCRIPT_DIR/libs/ffmpeg/lib/osx/ $app/Contents/MacOS/
	copy_lib $SCRIPT_DIR/libs/snappy/lib/osx/ $app/Contents/MacOS/

done

