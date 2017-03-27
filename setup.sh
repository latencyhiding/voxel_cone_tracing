dir=$(dirname "$0")

echo "What is the url of your project?"
read url

git -C $dir remote set-url origin $url

echo "Remotes set to:"
git -C $dir remote -v

rm -- "$0"
