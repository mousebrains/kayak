#! /usr/bin/env python3
# 
# Copy from old style levels_information database to new style in wkccLevels
#
# July-2022, Pat Welch, pat@mousebrains.com

from argparse import ArgumentParser
from TPWUtils import Logger
from TPWUtils.DB import DB
import logging
import re
from TPWUtils.int2hash import int2hash
import time
import sys

def execTimeit(tit:str, cur, sql:str, args:list=None) -> None:
    stime = time.time()
    try:
        cur.execute(sql, args);
        logging.info("Took %s seconds to %s", time.time() - stime, tit)
    except Exception as e:
        logging.error("Took %s seconds to %s", time.time() - stime, tit)
        logging.error("SQL\n%s", sql)
        if args: logging.error("ARGS %s", args)
        raise e

    
def copyTable(tgt, newTbl, oldCols, oldDB, oldTbl) -> None:
    cols = ",".join(oldCols) if oldCols else "*"
    sql = "INSERT IGNORE INTO " + newTbl
    sql+= " SELECT " + cols + " FROM " + oldDB + "." + oldTbl
    sql+= ";"
    execTimeit("Copy " + newTbl, tgt, sql)

def mkTempMaster(tgt) -> None:
    sql = "CREATE TEMPORARY TABLE tpwMaster SELECT"
    sql+= " *"
    # sql+= ",COALESCE(sort_key,display_name,name,usgs_id,cbtt_id,geos_id,nws_id,nwsli_id"
    # sql+= ",snotel_id) AS sectionName"
    # sql+= ",COALESCE(usgs_id,cbtt_id,geos_id,nws_id,nwsli_id,snotel_id,name,sort_key) AS gaugeName"
    sql+= " FROM levels_information.MergedMaster;"
    execTimeit("tpwMaster", tgt, sql)

def massageMaster(tgt) -> None:
    # adjust column mis-alignment for edited rows
    cols = {}
    cols['0z3'] = dict(
	watershed_type = "usgs_id",
	usgs_id = "userName",
	userName = "StationNumber",
	StationNumber = "usgs_id",
	state = "source_name",
	source_name = "sort_key",
	sort_key = "snotel_id",
	snotel_id = "section",
	section = "season",
	season = "scenery",
	scenery = "RunNumber",
	RunNumber = "river_name",
	river_name = "remoteness",
	remoteness = "region",
	region = "randomKey",
	randomKey = "PageNumber",
	pageNumber = "optimal_flow",
	optimal_flow = "nwsli_id",
	nwsli_id = "nws_id",
	nws_id = "notes",
	merged_dbs = "map_name",
	map_name = "low_flow",
	low_flow = "longitude",
	longitude = "length",
	length = "latitude",
	latitude = "high_flow",
	high_flow = "guide_book",
	guide_book = "gradient",
	gradient = "geos_id",
	geos_id = "gauge_location",
	gauge_location = "flood_stage",
	flood_stage = "NULL",
	features = "email",
	email = "elevation_lost",
	elevation_lost = "elevation",
	elevation = "drainage_area",
	drainage_area = "drainage",
	drainage = "display_name",
	display_name = "difficulties",
	difficulties = "description",
	)
    cols["7p3"] = dict(
	userName = "StationNumber",
	StationNumber = "usgs_id",
	randomKey = "PageNumber",
	PageNumber = "NULL",
	gauge_location = "flood_stage",
	flood_stage = "NULL",
	email = "elevation_lost",
	elevation_lost = "NULL",
	)
    cols["au"]  = dict(
	randomKey = "PageNumber",
	PageNumber = "NULL",
	email = "NULL",
	elevation_lost = "NULL",
	)
    cols["j5"]  = dict(
	userName = "StationNumber",
	StationNumber = "usgs_id",
	randomKey = "PageNumber",
	PageNumber = "NULL",
	email = "elevation_lost",
	elevation_lost = "elevation",
	elevation = "NULL",
	class_flow = "NULL",
	)
    cols["j5"]["class"] = "class_flow"
   
    for key in cols:
        fields = []
        for item in cols[key]: fields.append(item + "=" + cols[key][item]) 
        sql  = "UPDATE tpwMaster SET " + ",".join(fields) + " WHERE HashValue=%s"
        execTimeit(f"Massage {key}", tgt, sql, (key,))

def chompURLs(tgt) -> None:
    sql = "CREATE TEMPORARY TABLE tpwURLs SELECT url,max(time) as t FROM levels_data.url2name"
    sql+= " GROUP BY url"
    sql+= ";"
    execTimeit("tpwURLs", tgt, sql)

    sql = "INSERT INTO URL (url,t,parser,hours,qFetch)"
    sql+= " SELECT u2n.url,u2n.t,pp.parser,pp.hours,FALSE AS qFetch"
    sql+= " FROM tpwURLs as u2n"
    sql+= " LEFT JOIN levels_information.URLparse as pp"
    sql+= " ON u2n.url=pp.URL"
    sql+= ";"
    execTimeit("url2name", tgt, sql)
    execTimeit("Drop tpwURLs", tgt, "DROP TEMPORARY TABLE IF EXISTS tpwURLs;")
    execTimeit("NULL Parser", tgt, "UPDATE URL SET parser=NULL WHERE parser='';")

def procRating(tgt) -> None:
    sql = "INSERT IGNORE INTO rating (url,parser,t)"
    sql+= " SELECT cfs_to_gauge_data AS url,cfs_to_gauge_converter AS parser, NULL as t"
    sql+= " FROM tpwMaster WHERE cfs_to_gauge_data IS NOT NULL;"
    execTimeit("rating", tgt, sql)

def procGauge(tgt):
    execTimeit("tpwGauge DROP", tgt, "DROP TEMPORARY TABLE IF EXISTS tpwGauge;")
    sql = "CREATE TEMPORARY TABLE tpwGauge"
    sql+= " SELECT"
    sql+= " COALESCE(usgs_id,cbtt_id,geos_id,nws_id,nwsli_id,snotel_id,db_name) AS name"
    sql+= ",COALESCE(bank_full) AS bankFull"
    sql+= ",COALESCE(flood_stage) AS floodStage"
    sql+= ",COALESCE(gauge_location) AS location"
    sql+= ",COALESCE(latitude) AS latitude"
    sql+= ",COALESCE(longitude) AS longitude"
    sql+= ",COALESCE(StationNumber) AS stationID"
    sql+= ",COALESCE(cbtt_id) AS cbtt_id"
    sql+= ",COALESCE(geos_id) AS geos_id"
    sql+= ",COALESCE(nws_id) AS nws_id"
    sql+= ",COALESCE(nwsli_id) AS nwsli_id"
    sql+= ",COALESCE(snotel_id) AS snotel_id"
    sql+= ",COALESCE(usgs_id) AS usgs_id"
    sql+= ",rr.id as rating"
    sql+= ",HashValue"
    sql+= ",db_name"
    sql+= " FROM tpwMaster"
    sql+= " LEFT JOIN rating AS rr"
    sql+= " ON cfs_to_gauge_data=rr.url AND cfs_to_gauge_converter=rr.parser"
    sql+= " WHERE db_name IS NOT NULL"
    sql+= " GROUP BY db_name"
    sql+= ";"
    execTimeit("tpwGauge", tgt, sql)

    sql = "ALTER TABLE tpwGauge ADD COLUMN id INTEGER AUTO_INCREMENT PRIMARY KEY FIRST;"
    execTimeit("tpwGauge id", tgt, sql)

    execTimeit("tpwHash2Gauge DROP", tgt, "DROP TEMPORARY TABLE IF EXISTS tpwHash2Gauge")
    sql = "CREATE TEMPORARY TABLE tpwHash2Gauge SELECT HashValue,id FROM tpwGauge;"
    execTimeit("tpwHash2Gauge", tgt, sql)

    execTimeit("tpwGauge DROP HashValue", tgt, "ALTER TABLE tpwGauge DROP COLUMN HashValue;")
    execTimeit("tpwGauge DROP db_name", tgt, "ALTER TABLE tpwGauge DROP COLUMN db_name;")

    execTimeit("UNSAFE", tgt, "SET SQL_SAFE_UPDATES=0;")
    execTimeit("gauge Delete", tgt, "DELETE FROM gauge;")
    execTimeit("SAFE", tgt, "SET SQL_SAFE_UPDATES=1;")
    execTimeit("gauge", tgt, "INSERT INTO gauge SELECT * FROM tpwGauge;")
    execTimeit("tpwGauge DROP", tgt, "DROP TEMPORARY TABLE tpwGauge")

def procSection(tgt):
    execTimeit("Big enable", tgt, "SET SQL_BIG_SELECTS=1;")
    sql = "CREATE TEMPORARY TABLE tpwSec SELECT"
    sql+= " date AS tUpdate"
    sql+= ",gg.id AS gauge"
    sql+= ",COALESCE(sort_key,display_name,name,usgs_id,cbtt_id,geos_id,nws_id,nwsli_id,snotel_id) AS name"
    sql+= ",display_name AS displayName"
    sql+= ",sort_key AS sortName"
    sql+= ",Nature AS nature"
    sql+= ",description"
    sql+= ",difficulties"
    sql+= ",drainage AS basin"
    sql+= ",drainage_area AS basinArea"
    sql+= ",elevation"
    sql+= ",elevation_lost AS elevationLost"
    sql+= ",length AS distance"
    sql+= ",gradient"
    sql+= ",features"
    sql+= ",latitude"
    sql+= ",longitude"
    sql+= ",NULL AS latitudeStart"
    sql+= ",NULL AS longitudeStart"
    sql+= ",NULL AS latitudeEnd"
    sql+= ",NULL AS longitudeEnd"
    sql+= ",map_name AS mapName"
    sql+= ",no_show AS qHide"
    sql+= ",notes AS notes"
    sql+= ",optimal_flow AS optimalFlow"
    sql+= ",region"
    sql+= ",remoteness"
    sql+= ",scenery"
    sql+= ",season"
    sql+= ",watershed_type AS watershedType"
    sql+= ",NULL AS awID"
    sql+= ",mm.HashValue AS HashValue"
    sql+= " FROM tpwMaster as mm"
    sql+= " LEFT JOIN tpwHash2Gauge AS gg ON gg.HashValue=mm.HashValue"
    sql+= ";"
    execTimeit("tpwSec", tgt, sql)
    execTimeit("Big disable", tgt, "SET SQL_BIG_SELECTS=0;")

    execTimeit("tpwSec ID", tgt,
	"ALTER TABLE tpwSec ADD COLUMN id INTEGER AUTO_INCREMENT PRIMARY KEY FIRST;")
    execTimeit("tpwHash2Section", tgt,
	"CREATE TEMPORARY TABLE tpwHash2Section SELECT HashValue,id FROM tpwSec;");
    execTimeit("tpwSec drop Hash", tgt, "ALTER TABLE tpwSec DROP COLUMN HashValue;")
    execTimeit("section", tgt, "INSERT INTO section SELECT * from tpwSec;")

def procState(tgt):
    execTimeit("tpwState Drop", tgt, "DROP TABLE IF EXISTS tpwState;")    
    sql = "CREATE TABLE tpwState SELECT"
    sql+= " mm.HashValue"
    sql+= ",mm.state"
    sql+= ",st.id AS stateID"
    sql+= ",sec.id AS sectionID"
    sql+= " FROM tpwMaster AS mm"
    sql+= " LEFT JOIN state AS st ON st.name=mm.state"
    sql+= " LEFT JOIN tpwHash2Section AS sec ON sec.HashValue=mm.HashValue"
    sql+= " WHERE mm.state IS NOT NULL;"
    execTimeit("tpwState", tgt, sql)

    sql = "INSERT INTO section2state SELECT"
    sql+= " sectionID as section"
    sql+= ",stateID AS state"
    sql+= " FROM tpwState"
    sql+= " WHERE stateID is NOT NULL;"
    execTimeit("state single", tgt, sql)

    sql = "SELECT sectionID,state FROM tpwState WHERE stateID is NULL;"
    execTimeit("multi-state", tgt, sql)
    names = set()
    secState = {}
    for row in tgt:
        states = set()
        for state in row[1].split():
            state = state.replace("_", " ")
            states.add(state)
            names.add(state)
        secState[row[0]] = states
   
    sql = "SELECT id,name FROM state WHERE name IN ('" + "','".join(list(names)) + "');"
    execTimeit("state IDs", tgt, sql)
    name2id = {}
    for row in tgt: name2id[row[1]] = row[0]
   
    fields = [] 
    values = [] 
    for section in secState:
        for st in secState[section]:
            if st not in name2id: continue
            fields.append("(%s,%s)")
            values.extend((section, name2id[st]))
    sql = "INSERT INTO section2state VALUES" + ",".join(fields) + ";"
    execTimeit("multi-state", tgt, sql, values)


def parseCalc(expr, hash2row) -> str:
    ''' Replace old style hash::name::typ with updated hash and names '''
    pattern = r'([0-9A-Za-z]+)::([\w.:]+)::(flow|gauge)'
    a = expr
    for item in re.finditer(pattern, expr):
        a0 = item.group(0)
        (hval, name, typ) = item.groups()
        if hval not in hash2row:
            logging.error('Hash value, %s, not known', hval)
            logging.error('expr %s', expr)
            raise Exception('HashValue({}) in expression {} unknown'.format(hval, expr))
        row = hash2row[hval]
        gaugeID = int2hash(row['gaugeID'])
        name = row['db_name']
        a1 = '{}::{}::{}'.format(gaugeID, name, typ)
        a = a.replace(a0, a1)
    return a
    
def procCalc(tgt, row, hash2row, datatypes) -> None:
    if row['calc_expr'] is None: return None
    expr = parseCalc(row['calc_expr'], hash2row)
    t = parseCalc(row['calc_time'], hash2row)
    sql = 'INSERT INTO calc (dataType,expr,time,note) VALUES(%s,%s,%s,%s);'
    tgt.execute(sql, (datatypes[row['calc_type']], expr, t, row['calc_notes']))

    return tgt.lastrowid

def mkSource(tgt, row, name, url, calc) -> None:
    tgt.execute('INSERT INTO source (url,calc,name,agency) VALUES(%s,%s,%s,%s);',
               (url, calc, name, row['source_name']));
    srcID = tgt.lastrowid
    tgt.execute('INSERT INTO gauge2source (gauge,src) VALUES(%s,%s);', (row['gaugeID'], srcID))
    
def procSource(tgt, row, name2url) -> None:
    if row['db_name'] is None and row['merged_dbs'] is None and row['calcID'] is None:
        return None
    if row['merged_dbs'] is not None:
        if row['calcID'] is not None:
            for key in sorted(row): logging.error('row[%s] = %s', key, row[key])
            raise Exception('Both merged_dbs and calcID are defined')
        for name in row['merged_dbs'].split():
            if name in name2url:
                mkSource(tgt, row, name, name2url[name], None)
            else:
                logging.warning('merged db name=%s not known', name)
    elif row['calcID'] is not None:
        mkSource(tgt, row, row['db_name'], None, row['calcID'])
    else: # use db_name
        name = row['db_name']
        if name in name2url:
            mkSource(tgt, row, name, name2url[name], None)
        else:
            logging.warning('db_name=%s not known', name)

def procGuideBook(tgt, row):
    if row['guide_book'] is None: return None
    mapping = {
         'Soggy Sneakers': ('Soggy Sneakers', '4th'),
         'Soggy Sneakers 3rd Ed.': ('Soggy Sneakers', '3rd'),
         'Idaho - The Whitewater State': ('Idaho', '1st'),
         'Whitewater Rivers Of Washington':('A Guide to the Whitewater Rivers of Washington','2nd'),
         'Whitewater Rivers of Washington':('A Guide to the Whitewater Rivers of Washington','2nd')
         }

    item = mapping[row['guide_book']]

    sql = 'INSERT INTO section2GuideBook (section,page,run,guideBook)' \
        + ' SELECT %s as section, %s as page, %s as run, id as guideBook' \
        + ' FROM guideBook WHERE title=%s AND edition=%s;'
  
    tgt.execute(sql, (row['sectionID'], row['PageNumber'], row['RunNumber'], item[0], item[1]))
   
def procStatus(tgt, row, datatypes): # Low/Okay/High
    if row['low_flow'] is None and row['high_flow'] is None: return
    lFlow = row['low_flow']
    hFlow = row['high_flow']
   
    lDT = datatypes['flow']
    hDT = datatypes['flow']

    if lFlow is not None and lFlow[-2:] == 'ft':
        lFlow = lFlow[0:-2]
        lDT = datatypes['gauge']
 
    if hFlow is not None and hFlow[-2:] == 'ft':
        hFlow = hFlow[0:-2]
        hDT = datatypes['gauge']

    sql = 'INSERT INTO section2level (section,low,lowDatatype,high,highDatatype,level)' \
        + ' VALUES(%s,%s,%s,%s,%s,(SELECT id FROM level WHERE name=%s));'

    if lFlow is None: 
        tgt.execute(sql, (row['sectionID'], None, hDT, hFlow, hDT, 'Okay'))
        tgt.execute(sql, (row['sectionID'], hFlow, hDT, None, hDT, 'High'))
    elif hFlow is None:
        tgt.execute(sql, (row['sectionID'], None, lDT, lFlow, lDT, 'Low'))
        tgt.execute(sql, (row['sectionID'], lFlow, lDT, None, lDT, 'Okay'))
    else: # Both high and low flows defined
        tgt.execute(sql, (row['sectionID'], None, lDT, lFlow, lDT, 'Low'))
        tgt.execute(sql, (row['sectionID'], lFlow, lDT, hFlow, hDT, 'Okay'))
        tgt.execute(sql, (row['sectionID'], hFlow, hDT, None, hDT, 'High'))

def procClass(tgt, row, datatypes): # Class rating
    if row['class'] is None and row['class_flow'] is None: return
    sql = 'INSERT INTO class (section,name,low,lowDatatype,high,highDatatype)' \
        + ' VALUES(%s,%s,%s,%s,%s,%s);'

    if row['class_flow'] is None:
        dt = datatypes['flow']
        tgt.execute(sql, (row['sectionID'], row['class'], None, dt, None, dt))
        return

    cnt = 0 
    for item in row['class_flow'].split(','):
        cnt += 1
        fields = item.split()
        name = fields[0]
        lFlow = fields[1]
        hFlow = fields[2] if len(fields) > 2 else None
        lDT = datatypes['flow']
        hDT = lDT
        if lFlow[-2:] == 'ft':
            lDT = datatypes['gauge']
            lFlow = lFlow[0:-2]
        if hFlow is not None and hFlow[-2:] == 'ft':
            hDT = datatypes['gauge']
            hFlow = hFlow[0:-2]
        if hFlow is not None: # Both sides specified
            tgt.execute(sql, (row['sectionID'], name, lFlow, lDT, hFlow, hDT))
        elif cnt == 1: # Only one side specified on first entry
            tgt.execute(sql, (row['sectionID'], name, None, lDT, lFlow, lDT))
        else: # Only one side specified on not first entry
            tgt.execute(sql, (row['sectionID'], name, hFlow, hDT, None, hDT))

def mkDatatypes(tgt) -> dict:
    a = {}
    tgt.execute('SELECT id,name FROM dataType;')
    for row in tgt: 
       a[row[1]] = row[0]
    return a

def chompMaster(tgt, cur, name2url:dict):
    procRating(tgt) # Independent
    procGauge(tgt) # after rating
    procSection(tgt) # after gauge
    # procCalc(tgt)
    # procState(tgt) # After section construction
    return True
    hash2row = {}
    states = {}
    sectionNames = set()
    datatypes = mkDatatypes(tgt)

    for row in rows: # populate gauge table
        hash2row[row['HashValue']] = row
        row['sectionName'] = mkSectionName(row, sectionNames)
        # row['stateID'] = buildState(tgt, row, states)

    for row in rows: # Populate calc table, which needs a fully populated hash2row dictionary
        row['calcID'] = procCalc(tgt, row, hash2row, datatypes)
        procSource(tgt, row, name2url)
        row['sectionID'] = procSection(tgt, row)
        # procState(tgt, row, states)
        procGuideBook(tgt, row)
        procStatus(tgt, row, datatypes)
        procClass(tgt, row, datatypes)

parser = ArgumentParser()
DB.addArgs(parser) # Database related arguments
Logger.addArgs(parser)
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

with DB(args) as tgtDB, DB(args, "levels_information") as srcDB:
    tgt = tgtDB.cursor()
    tgt.execute("BEGIN;")
    copyTable(tgt, "parameters", None, "levels_information", "Parameters")
    copyTable(tgt, "edit",       None, "levels_information", "Edit")
    copyTable(tgt, "mapBuilder", None, "levels_information", "MapBuilder")
    name2url = chompURLs(tgt)
    mkTempMaster(tgt) # Create a name for this table using coalesce
    massageMaster(tgt) # Massage some rows
    procRating(tgt)
    procGauge(tgt) # after rating
    procSection(tgt) # after gauge
    procState(tgt) # After section
    # src = srcDB.cursor(dictionary=True)
    # chompMaster(tgt, src, name2url)
    tgt.execute("COMMIT;")
