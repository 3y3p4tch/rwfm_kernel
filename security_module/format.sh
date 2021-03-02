#!/bin/sh

# Format the new changes as per clang-format of the project
#
CLANG_FORMAT="clang-format-diff -p1 -sort-includes -style=file -iregex '.*\.(c|cxx|cpp|hpp|h)$' "
GIT_DIFF="git diff -U0 --no-color "
GIT_APPLY="git apply -v -p0 - "
FORMATTER_DIFF=$(eval ${GIT_DIFF} --cached | eval ${CLANG_FORMAT})
echo  "\n------Format code hook is called-------"
if [ -z "${FORMATTER_DIFF}" ]; then
	echo "Nothing to be formatted"
else
	echo "${FORMATTER_DIFF}"
	echo "${FORMATTER_DIFF}" | eval ${GIT_APPLY} --cached
	echo "      ---Format of staged area completed. Begin format unstaged files---"
	eval ${GIT_DIFF} | eval ${CLANG_FORMAT} | eval ${GIT_APPLY}
fi
echo "------Format code hook is completed----\n"
exit