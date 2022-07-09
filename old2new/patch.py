#! /usr/bin/env python3
# 
# Patch up output of copyInfo for new database structure
#
from argparse import ArgumentParser
from TPWUtils import Logger
from TPWUtils.DB import DB
import logging

def patch(tgt) -> None: # Patch up the info, so after copyInfo.py
    tgt.execute("UPDATE description SET columnName='id',type='hash' WHERE sortKey=0;")
    tgt.execute("UPDATE description SET columnName='displayName',type='h1' WHERE sortKey=1;")
    tgt.execute("UPDATE description SET type='class' WHERE columnName='Class';")
    tgt.execute("UPDATE description SET type='state' WHERE columnName='State';")

parser = ArgumentParser()
DB.addArgs(parser) # Database related arguments
Logger.addArgs(parser)
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

with DB(args) as tgtDB:
    tgt = tgtDB.cursor()
    tgt.execute("BEGIN;")
    patch(tgt)
    tgt.execute("COMMIT;")
