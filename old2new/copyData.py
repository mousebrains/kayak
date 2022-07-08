#! /usr/bin/env python3
# 
# Copy from old style database to new style
#
from argparse import ArgumentParser
from TPWUtils import Logger
from TPWUtils.DB import DB
import logging

def mkMapping(tgt, sql) -> dict:
    a = {}
    tgt.execute(sql)
    for row in tgt:
        a[row[0]] = row[1]
    return a

def chompData(tgt, src):
    oldDBname = "levels_data" # Old database name
    
    # data type to id
    typ2id = mkMapping(tgt, "SELECT name,id FROM dataType;")
    name2src = mkMapping(tgt, "SELECT name,id FROM source;")
    name2url = mkMapping(src, "SELECT name,url FROM url2name;")

    # Grab the existing data tables, flow_name 
    # items = set()
    src.execute("SHOW TABLES;")
    tables = src.fetchall()
    logging.info("There are %s tables", len(tables))

    for row in tables:
        tbl = row[0]
        parts = tbl.split("_")
        if len(parts) == 1: 
            logging.info("Skipping table %s", tbl)
            continue
        dt = parts[0]
        name = "_".join(parts[1:])
        if name not in name2src:
            logging.warning("Do not know name %s for table %s", name, tbl)
            sql = "INSERT INTO source (url,name) VALUES(%s,(SELECT id FROM URL WHERE url=%s));"
            tgt.execute(sql, (name, name2url[name]))
            name2src[name] = tgt.lastrowid
        srcID = name2src[name]
        logging.info("Copying data dt=%s name=%s src=%s", dt, name, srcID)
        sql = "INSERT IGNORE INTO data (src, dataType, value, t)" 
        sql+=f" SELECT {srcID} AS src,{typ2id[dt]} AS datatype, value, time AS t"
        sql+=f" FROM {oldDBname}.{tbl};"
        tgt.execute(sql)

parser = ArgumentParser()
DB.addArgs(parser) # Database related arguments
Logger.addArgs(parser)
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

with DB(args) as tgtDB, DB(args, "levels_data") as srcDB:
    tgt = tgtDB.cursor()
    tgt.execute("BEGIN;")
    chompData(tgt, srcDB.cursor())
    tgt.execute("COMMIT;")
