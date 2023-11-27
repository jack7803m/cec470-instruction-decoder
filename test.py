# Simple program to test whether two files are identical
# Usage: python test.py <file1> <file2>

import sys

def main():
    if len(sys.argv) != 3:
        print("Usage: python test.py <file1> <file2>")
        sys.exit(1)

    file1 = sys.argv[1]
    file2 = sys.argv[2]

    with open(file1) as f1, open(file2) as f2:
        if f1.read() == f2.read():
            print("Test passed ✅")
        else:
            print("Test failed ❌")

if __name__ == "__main__":
    main()