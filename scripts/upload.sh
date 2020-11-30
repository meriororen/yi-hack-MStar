#/bin/bash

get_script_dir()
{
    echo "$(cd `dirname $0` && pwd)"
}

SRC_DIR=$(get_script_dir)/../src
BUILD_DIR=$(get_script_dir)/../build

echo "upload to bashupload"
cp $BUILD_DIR/home/yi-hack/bin/$1 $1
curl https://bashupload.com/$1.elf --data-binary "@"$1
