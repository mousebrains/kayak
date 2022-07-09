#! /usr/bin/env python3
#
# Generate a signature for the contents of a file
#
# July-2022, Pat Welch, pat@mousebrains.com

from hashlib import sha512
import os.path

def fileSignature(fn:str) -> str:
    if not os.path.isfile(fn): return None
    with open(fn, "rb") as fp:
       h = sha512(fp.read())
       return h.hexdigest()

if __name__ == "__main__":
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("fn", nargs="+", help="Files to calculate signatures of")
    args = parser.parse_args()

    for fn in args.fn:
        sig = fileSignature(fn)
        print(fn, "->", sig)
