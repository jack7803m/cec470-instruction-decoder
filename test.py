# Simple program to test whether two files are identical and print the location of the differences
# Usage: python test.py <file1> <file2>

import sys

def main():
    if len(sys.argv) != 3:
        print("Usage: python test.py <file1> <file2>")
        sys.exit(1)

    file1 = sys.argv[1]
    file2 = sys.argv[2]

    passed = True

    with open(file1) as f1, open(file2) as f2:
        for i, (line1, line2) in enumerate(zip(f1, f2)):
            if line1 != line2:
                passed = False
                print("----------------------------------------")
                print("Test failed ❌")
                print(f"Difference found on line {i + 1}\n")
                print(f"File 1: {line1.rstrip()}")
                print(f"File 2: {line2.rstrip()}")

    if passed:
        print("Test passed ✅")


if __name__ == "__main__":
    main()