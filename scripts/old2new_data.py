#! /usr/bin/env python3
# 
# Copy from old style database to new style
#
import mysql.connector
from mysql.connector.errors import Error
import MyLogger
import MyDB
import argparse

def loadDatatypes(tgt) -> dict:
    a = {}
    tgt.execute('SELECT name,id FROM datatype;')
    for row in tgt: a[row[0]] = row[1];
    return a

def loadSourceNames(tgt) -> dict:
    a = {}
    tgt.execute('SELECT id,name FROM source;')
    for row in tgt: a[row[1]] = row[0]
    return a
  
def loadname2url(cur) -> dict:
    a = {}
    cur.execute('SELECT url,name FROM url2name;')
    for row in cur: a[row[1]] = row[0]
    return a
 
def chompData(args, logger, tgt):
    oldDBname = 'levels_data' # Old database name
    
    # data type to id
    typ2id = loadDatatypes(tgt)
    name2src = loadSourceNames(tgt)

    # Open up the old data database
    src = mysql.connector.connect(host=args.dbhost, user=args.dbuser, passwd=args.dbpasswd,
                db=oldDBname)
    cur = src.cursor()

    name2url = loadname2url(cur)

    # Grab the existing data tables, flow_name 
    # items = set()
    cur.execute('SHOW TABLES;')
    tables = cur.fetchall()
    logger.info('There are %s tables', len(tables))

    for row in tables:
        tbl = row[0]
        parts = tbl.split('_')
        if len(parts) == 1: 
            logger.info('Skipping table %s', tbl)
            continue
        dt = parts[0]
        name = '_'.join(parts[1:])
        if name not in name2src:
            logger.warning('Do not know name %s for table %s', name, tbl)
            sql = 'INSERT INTO source (url,name) VALUES(%s,(SELECT id FROM url WHERE url=%s));'
            tgt.execute(sql, (name, name2url[name]))
            name2src[name] = tgt.lastrowid
        srcID = name2src[name]
        logger.info('Copying data dt=%s name=%s src=%s', dt, name, srcID)
        sql = 'INSERT IGNORE INTO data (src, dataType, value, t)' \
            + 'SELECT {} AS src,{} AS datatype, value, time AS t FROM {}.{}'.format(
               srcID, typ2id[dt], oldDBname, tbl)
        tgt.execute(sql)

    src.close()

parser = argparse.ArgumentParser()
MyDB.DB.addArgs(parser) # Database related arguments
MyLogger.addArgs(parser)
args = parser.parse_args()

logger = MyLogger.mkLogger(args, __name__)

tgt = MyDB.DB(args, logger)
curTgt = tgt.cursor()

curTgt.execute('START TRANSACTION;')
chompData(args, logger, curTgt)

curTgt.execute('COMMIT;')
tgt.close()
