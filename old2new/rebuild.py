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
import time
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

def addNegArg(parser:ArgumentParser, opt:str, helparg:str=None, negate:str="no") -> ArgumentParser:
    grp = parser.add_mutually_exclusive_group()
    grp.add_argument(f"--{opt}", action="store_true", dest=opt, default=True,
        help=f"Do {helparg}" if helparg else "Take action {opt}")
    grp.add_argument(f"--{negate}{opt}", action="store_false", dest=opt, 
        help=f"Don't do {helparg}" if helparg else "Don't take action {opt}")
    return parser

parser = ArgumentParser()
Logger.addArgs(parser)
parser.add_argument("schema", type=str, nargs="+",
	metavar="foo.sql", help="Schema file to load(s)")
parser.add_argument("--dbrc", type=str, metavar="filename", default="~/.kayakingrc",
	help="RC file to load")
addNegArg(parser, "schema", "load schema(s)")
addNegArg(parser, "info", "run copyInfo.py")
addNegArg(parser, "patch", "run patch.py")
addNegArg(parser, "data", "run copyData.py")
grp = parser.add_argument_group(description="Command paths")
grp.add_argument("--cmd_copyInfo", type=str, default="./copyInfo.py", help="copyInfo.py command")
grp.add_argument("--cmd_patch", type=str, default="./patch.py", help="patch.py command")
grp.add_argument("--cmd_copyData", type=str, default="./copyData.py", help="copyData.py command")
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

if args.schema:
    info = mkInfo(args.dbrc)
    for fn in args.schema:
        stime = time.time()
        if rebuild(fn, info): sys.exit(1)
        logging.info("Took %s seconds to process %s", time.time() - stime, fn)

if args.info: 
    stime = time.time()
    if runit([args.cmd_copyInfo, "--verbose"]): sys.exit(1)
    logging.info("Took %s seconds to copyInfo", time.time() - stime)
if args.patch:
    stime = time.time()
    if runit([args.cmd_patch, "--verbose"]): sys.exit(1)
    logging.info("Took %s seconds to patch", time.time() - stime)
if args.data:
    stime = time.time()
    if runit([args.cmd_copyData, "--verbose"]): sys.exit(1)
    logging.info("Took %s seconds to copyData", time.time() - stime)
