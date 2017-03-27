dir=$(dirname "$0")

echo "What is the name of your project?"
read name

sed -i -e "s/set(APP_NAME .*)/set(APP_NAME $name)/g" "$dir/CMakeLists.txt"

echo "What is the url of your project?"
read url

git -C $dir remote set-url origin $url

echo "Remotes set to:"
git -C $dir remote -v

rm -- "$0"

echo "# $name" >> $dir/README.md

git -C $dir add -A
git -C $dir commit -m "First commit"
git -C $dir push -f -u origin master
