#! /usr/bin/env python3
#
# Completely rebuild everything from the old database
#
# July-2022, Pat Welch, pat@mousebrains.com

from argparse import ArgumentParser
from TPWUtils import Logger
import yaml
import subprocess
import os.path
import logging
import sys

def runit(cmd:tuple, input:bytes=None) -> bool:
    s = subprocess.run(cmd,
		input=input,
		shell=False,
		capture_output=True)
    if s.returncode:
        logging.warning("Return code %s", s.returncode)
    if s.stderr:
        try:
            logging.warning("STDERR\n%s", str(s.stderr, "UTF-8"))
        except:
            logging.warning("STDERR\n%s", s.stderr)
    if s.stdout:
        try:
            logging.info("STDOUT\n%s", str(s.stdout, "UTF-8"))
        except:
            logging.info("STDOUT\n%s", s.stdout)
    return s.returncode

def mkInfo(fn:str) -> dict:
    fn = os.path.abspath(os.path.expanduser(fn))
    with open(fn, "r") as fp: return yaml.safe_load(fp)

def rebuild(fn:str, info:dict) -> bool: # Patch up the info, so after copyInfo.py
    with open(os.path.abspath(os.path.expanduser(fn)), "rb") as fp: sql = fp.read()

    cmd = ("/usr/bin/mysql",
	"-u", info["username"],
	f"--password={info['password']}",
	"-h", info["hostname"],
	info["database"])

    return runit(cmd, sql)

parser = ArgumentParser()
Logger.addArgs(parser)
parser.add_argument("schema", type=str, nargs="+",
	metavar="foo.sql", help="Schema file to load(s)")
parser.add_argument("--dbrc", type=str, metavar="filename", default="~/.kayakingrc",
	help="RC file to load")
parser.add_argument("--noschema", action="store_true", help="Don't load schema(s)")
parser.add_argument("--noinfo", action="store_true", help="Don't run copyInfo.py")
parser.add_argument("--nopatch", action="store_true", help="Don't run patch.py")
parser.add_argument("--nodata", action="store_true", help="Don't run copyData.py")
parser.add_argument("--copyInfo", type=str, default="./copyInfo.py", help="copyInfo.py command")
parser.add_argument("--patch", type=str, default="./patch.py", help="patch.py command")
parser.add_argument("--copyData", type=str, default="./copyData.py", help="copyData.py command")
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

if not args.noschema:
    info = mkInfo(args.dbrc)
    for fn in args.schema:
        if rebuild(fn, info): sys.exit(1)

if not args.noinfo: 
    if runit([args.copyInfo, "--verbose"]): sys.exit(1)
if not args.nopatch:
    if runit([args.patch, "--verbose"]): sys.exit(1)
if not args.nodata:
    if runit([args.copyData, "--verbose"]): sys.exit(1)
