find . -regex ".*\.\(ino\|cpp\|hpp\|cu\|c\|h\)" -exec clang-format -style=file -i {} \;
