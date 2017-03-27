dir=$(dirname "$0")

echo "What is the url of your project?"
read url

pushd $dir

git remote set-url origin url

echo "Remotes set to:"
git remote -v

rm -- "$0"

popd
