#!/usr/bin/env python3
import argparse
import platform
import subprocess
import sys
import os


def run(cmd):
    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        print("Build failed")
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("version", nargs="?", default="USA")
    parser.add_argument("-skipfs", "-skip-fs", action="store_true")
    parser.add_argument("-onlyfs", "-fs", action="store_true")
    parser.add_argument("-clean", "-c", action="store_true")
    parser.add_argument("-h", "--help", action="help")

    args = parser.parse_args()

    py_cmd = sys.executable
    make_cmd = "make clean" if args.clean else "make"

    if args.clean == False and (args.skipfs == False or args.onlyfs):
        run(
            f'\"{py_cmd}\" tool/hConv.py filesystem src/filesystem '
            '--priority error_screens/Error_IPL.yaz0 '
            '--extensions zmap,zscene,bin,yaz0,tbl'
        )

        if args.onlyfs:
            return

    print("Compiling...")

    old_dir = os.getcwd()
    os.chdir("src")
    try:
        run(f'{make_cmd} "{args.version}"')
    finally:
        os.chdir(old_dir)

    print("OK.\n")


if __name__ == "__main__":
    main()
