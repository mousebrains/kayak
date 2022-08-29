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
    sql = "CREATE TEMPORARY TABLE tpwU2N SELECT"
    sql+= " url"
    sql+= ",name"
    sql+= ",max(time) AS t"
    sql+= " FROM levels_data.url2name"
    sql+= " GROUP BY url,name;"
    execTimeit("tpwU2N", tgt, sql)

    execTimeit("tpwU2N ALTER", tgt, 
	"ALTER TABLE tpwU2N ADD INDEX(name(512)), ADD INDEX(url(512))")

    sql = "CREATE TEMPORARY TABLE tpwURLs SELECT url,max(t) as t FROM tpwU2N GROUP BY url;"
    execTimeit("tpwURLs", tgt, sql)

    sql = "INSERT INTO URL (url,t,parser,hours,qFetch)"
    sql+= " SELECT u2n.url,u2n.t,pp.parser,pp.hours,FALSE AS qFetch"
    sql+= " FROM tpwURLs as u2n"
    sql+= " LEFT JOIN levels_information.URLparse as pp"
    sql+= " ON u2n.url=pp.URL"
    sql+= ";"
    execTimeit("URL", tgt, sql)
    execTimeit("Drop tpwURLs", tgt, "DROP TEMPORARY TABLE IF EXISTS tpwURLs;")
    execTimeit("NULL Parser", tgt, "UPDATE URL SET parser=NULL WHERE parser='';")

    execTimeit("U2N Alter", tgt, "ALTER TABLE tpwU2N ADD COLUMN id INTEGER;")

    sql = "UPDATE"
    sql+= " tpwU2N AS u2n"
    sql+= ",URL"
    sql+= " SET u2n.id=URL.id"
    sql+= " WHERE u2n.url=URL.url;"
    execTimeit("U2N ID", tgt, sql)

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

def procState(tgt) -> None:
    sql = "CREATE TEMPORARY TABLE tpwState SELECT"
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

def procGuideBook(tgt) -> None:
    sql = "CREATE TEMPORARY TABLE tpwGB SELECT"
    sql+= " sec.id AS section"
    sql+= ",NULL AS guideBook"
    sql+= ",PageNumber AS page"
    sql+= ",RunNumber AS run"
    sql+= ",NULL AS url"
    sql+= ",guide_book"
    sql+= " FROM tpwMaster AS mm"
    sql+= " LEFT JOIN tpwHash2Section AS sec ON mm.HashValue=sec.HashValue"
    sql+= " WHERE guide_book IS NOT NULL;"
    execTimeit("tpwGB", tgt, sql)
    execTimeit("tpwGB Alter", tgt, 
	"ALTER TABLE tpwGB"
	+ " MODIFY COLUMN guideBook INTEGER"
	+ ",MODIFY COLUMN url TEXT"
	+ ";")

    books = {
         'Soggy Sneakers': ('Soggy Sneakers', '4th'),
         'Soggy Sneakers 3rd Ed.': ('Soggy Sneakers', '3rd'),
         'Idaho - The Whitewater State': ('Idaho', '1st'),
         'Whitewater Rivers Of Washington':('A Guide to the Whitewater Rivers of Washington','2nd'),
         'Whitewater Rivers of Washington':('A Guide to the Whitewater Rivers of Washington','2nd')
         }
    execTimeit("UNSAFE", tgt, "SET SQL_SAFE_UPDATES=0;")
    for gb in books:
        book = books[gb]
        sql = "UPDATE tpwGB AS tpw, guideBook AS gb SET tpw.guideBook=gb.id"
        sql+= " WHERE tpw.guide_book=%s AND gb.title=%s AND gb.edition=%s;"
        execTimeit(f"tpwGB {gb}", tgt, sql, (gb, book[0], book[1]))

    execTimeit("sec2GB DELETE", tgt, "DELETE FROM section2guideBook;")
    execTimeit("SAFE", tgt, "SET SQL_SAFE_UPDATES=1;")
    execTimeit("tpwGB Alter Drop", tgt, "ALTER TABLE tpwGB DROP COLUMN guide_book;")
    execTimeit("sec2GB", tgt, "INSERT INTO section2guideBook SELECT * FROM tpwGB;")
    execTimeit("tpwGB Drop", tgt, "DROP TABLE tpwGB;")

def procClass(tgt) -> None: # Class rating
    sql = "CREATE TEMPORARY TABLE tpwClass SELECT"
    sql+= " sec.id AS section"
    sql+= ",mm.class"
    sql+= ",mm.class_flow"
    sql+= " FROM tpwMaster AS mm"
    sql+= " LEFT JOIN tpwHash2Section AS sec ON mm.HashValue=sec.HashValue"
    sql+= " WHERE mm.class IS NOT NULL OR mm.class_flow IS NOT NULL;"
    execTimeit("tpwClass", tgt, sql)

    # Unbounded descriptions
    sql = "INSERT INTO class (section,name)"
    sql+= " SELECT section,class FROM tpwClass"
    sql+= " WHERE class_flow IS NULL;"
    execTimeit("UNBOUNDED", tgt, sql)

    # Dynamic descriptions
    sql = "SELECT section,class_flow FROM tpwClass WHERE class_flow IS NOT NULL;"
    execTimeit("BOUNDED", tgt, sql)
    data = []
    values = []
    for row in tgt:
        (section, items) = row
        for item in items.split(","):
            fields = item.split()
            label = fields[0]
            low = fields[1]
            high = fields[2] if len(fields) > 2 else None
            lDT = "gauge" if "ft" in low else "flow"
            hDT = None if high is None else ("gauge" if "ft" in high else "flow")
            data.extend((section, label, low, lDT, high, hDT))
            values.append("%s,%s"
		+ ",%s,(SELECT id FROM dataType WHERE name=%s)"
		+ ",%s,(SELECT id FROM dataType WHERE name=%s)")
    if data:
        sql = "INSERT INTO class VALUES (" + "),(".join(values) + ");"
        execTimeit("Dynamic", tgt, sql, data)
    execTimeit("tpwClass Drop", tgt, "DROP TABLE IF EXISTS tpwClass;")
            
def procLevel(tgt) -> None: # Low,Okay,High indicator
    sql = "CREATE TEMPORARY TABLE tpwLevel SELECT"
    sql+= " sec.id AS section"
    sql+= ",mm.low_flow"
    sql+= ",mm.high_flow"
    sql+= ",REGEXP_REPLACE(mm.low_flow, 'ft', '') AS lowVal"
    sql+= ",REGEXP_REPLACE(mm.high_flow, 'ft', '') AS highVal"
    sql+= ",0 AS lowDT"
    sql+= ",0 AS highDT"
    sql+= " FROM tpwMaster AS mm"
    sql+= " LEFT JOIN tpwHash2Section AS sec ON mm.HashValue=sec.HashValue"
    sql+= " WHERE mm.low_flow IS NOT NULL OR mm.high_flow IS NOT NULL;"
    execTimeit("tpwLevel", tgt, sql)

    execTimeit("UNSAFE", tgt, "SET SQL_SAFE_UPDATES=0;")

    sql = "UPDATE tpwLevel AS ll,dataType AS dt SET"
    sql+= " ll.lowDT=dt.id"
    sql+= " WHERE ll.low_flow=ll.lowVal AND dt.name='flow';"
    execTimeit("tpwLevel Low Flow DT", tgt, sql)

    sql = "UPDATE tpwLevel AS ll,dataType AS dt SET"
    sql+= " ll.lowDT=dt.id"
    sql+= " WHERE ll.low_flow!=ll.lowVal AND dt.name='gauge';"
    execTimeit("tpwLevel Low Gauge DT", tgt, sql)

    sql = "UPDATE tpwLevel AS ll,dataType AS dt SET"
    sql+= " ll.highDT=dt.id"
    sql+= " WHERE ll.high_flow=ll.highVal AND dt.name='flow';"
    execTimeit("tpwLevel High Flow DT", tgt, sql)

    sql = "UPDATE tpwLevel AS ll,dataType AS dt SET"
    sql+= " ll.highDT=dt.id"
    sql+= " WHERE ll.high_flow!=ll.highVal AND dt.name='gauge';"
    execTimeit("tpwLevel High Gauge DT", tgt, sql)

    execTimeit("sec2lev DELETE", tgt, "DELETE FROM section2level;")
    execTimeit("SAFE", tgt, "SET SQL_SAFE_UPDATES=1;")

    sql = "INSERT INTO section2level (section,level,high,highDatatype)"
    sql+= " SELECT section,ll.id,lowVal,lowDT FROM tpwLevel"
    sql+= " LEFT JOIN level AS ll ON ll.name='Low'"
    sql+= " WHERE low_flow IS NOT NULL;"
    execTimeit("sec2lev low", tgt, sql)

    sql = "INSERT INTO section2level (section,level,low,lowDatatype,high,highDataType)"
    sql+= " SELECT section,ll.id,lowVal,lowDT,highVal,highDT FROM tpwLevel"
    sql+= " LEFT JOIN level AS ll ON ll.name='Okay'"
    sql+= " WHERE low_flow IS NOT NULL AND high_flow IS NOT NULL;"
    execTimeit("sec2lev okay", tgt, sql)

    sql = "INSERT INTO section2level (section,level,low,lowDatatype)"
    sql+= " SELECT section,ll.id,highVal,highDT FROM tpwLevel"
    sql+= " LEFT JOIN level AS ll ON ll.name='High'"
    sql+= " WHERE high_flow IS NOT NULL;"
    execTimeit("sec2lev high", tgt, sql)

    execTimeit("tpwLevel Drop", tgt, "DROP TABLE IF EXISTS tpwLevel;")

def procSource(tgt) -> None:
    sql = "CREATE TEMPORARY TABLE tpwSrc0 SELECT"
    sql+= " gg.id AS gauge"
    sql+= ",mm.db_name"
    sql+= ",mm.merged_dbs"
    sql+= ",mm.calc_expr"
    sql+= ",mm.calc_type"
    sql+= ",mm.calc_time"
    sql+= ",mm.calc_notes"
    sql+= ",mm.source_name as agency"
    sql+= " FROM tpwMaster AS mm"
    sql+= " LEFT JOIN tpwHash2Gauge AS gg ON gg.HashValue=mm.HashValue"
    sql+= " WHERE db_name IS NOT NULL OR merged_dbs IS NOT NULL OR calc_expr IS NOT NULL;"
    execTimeit("tpwSrc0", tgt, sql)

    procNotCalc(tgt)
    procCalc(tgt)
    execTimeit("tpwSrc0 Drop", tgt, "DROP TABLE IF EXISTS tpwSrc0")

def procNotCalc(tgt) -> None:
    sql = "CREATE TEMPORARY TABLE tpwSrc SELECT"
    sql+= " gauge"
    sql+= ",NULL AS url"
    sql+= ",NULL AS calc"
    sql+= ",db_name AS name"
    sql+= ",agency"
    sql+= " FROM tpwSrc0 AS tpw"
    sql+= " WHERE db_name IS NOT NULL AND merged_dbs IS NULL AND calc_expr IS NULL;"
    execTimeit("tpwSrc Simple", tgt, sql)

    execTimeit("Merged", tgt, "SELECT gauge,merged_dbs FROM tpwSrc0 WHERE merged_dbs IS NOT NULL;")
    data = []
    values = []
    for row in tgt:
        (gauge, items) = row
        for item in items.split():
            data.extend([gauge, item])
            values.append("%s,%s")
    sql = "INSERT INTO tpwSrc (gauge,name) VALUES (" + "),(".join(values) + ");"
    execTimeit("Merged", tgt, sql, data)

    execTimeit("tpwSrc Alter", tgt,
	"ALTER TABLE tpwSrc MODIFY COLUMN url INTEGER, MODIFY COLUMN calc INTEGER;")

    execTimeit("UNSAFE", tgt, "SET SQL_SAFE_UPDATES=0;")
    sql = "UPDATE tpwSrc AS src,tpwU2N AS u2n SET src.url=u2n.id WHERE src.name=u2n.name;"
    execTimeit("URL2ID", tgt, sql)
    execTimeit("Null merged", tgt, "UPDATE tpwSrc SET url=NULL WHERE url=0;")
    execTimeit("Null merged", tgt, "UPDATE tpwSrc SET calc=NULL WHERE calc=0;")
    execTimeit("SAFE", tgt, "SET SQL_SAFE_UPDATES=1;")

    execTimeit("Add ID", tgt,
	"ALTER TABLE tpwSrc ADD COLUMN id INTEGER AUTO_INCREMENT PRIMARY KEY;")

    execTimeit("Source", tgt,
	"INSERT INTO source (id,url,name,agency)"
	+ " SELECT id,url,name,agency FROM tpwSrc WHERE url IS NOT NULL;")
    execTimeit("G2S", tgt, "INSERT INTO gauge2source SELECT gauge,id FROM tpwSrc;")
    execTimeit("tpwSrc Drop", tgt, "DROP TABLE tpwSrc;")

def procCalc(tgt) -> None:
    execTimeit("Calc", tgt, 
	"CREATE TEMPORARY TABLE tpwCalc"
	+ " SELECT gauge,db_name,calc_expr,calc_type,calc_time,calc_notes,agency FROM tpwSrc0"
	+ " WHERE calc_expr IS NOT NULL AND calc_time IS NOT NULL and calc_type IS NOT NULL;")
    execTimeit("Add ID", tgt,
	"ALTER TABLE tpwCalc ADD COLUMN id INTEGER AUTO_INCREMENT PRIMARY KEY;")
    sql = "INSERT INTO calc"
    sql+= " SELECT"
    sql+= " cc.id"
    sql+= ",dt.id AS dataType"
    sql+= ",cc.calc_expr AS expr"
    sql+= ",cc.calc_time AS time"
    sql+= ",cc.calc_notes AS note"
    sql+= " FROM tpwCalc AS cc"
    sql+= " LEFT JOIN dataType AS dt ON dt.name=cc.calc_type;"
    execTimeit("calc", tgt, sql)
    execTimeit("g2c", tgt, "INSERT INTO gauge2source SELECT gauge,id FROM tpwCalc;")
    execTimeit("tpwCalc Drop", tgt, "DROP TABLE IF EXISTS tpwCalc")
    
def translateCalc(tgt, columnName:str) -> None:
    execTimeit("xlat", tgt, "SELECT id," + columnName + " FROM calc;")
    for row in tgt:
        (calcID, expr) = row
        print(calcID, expr)
    return
    result = []
    toDrop = set(("flow", "gauge", "temperature"))
    pattern = r"([a-z0-9]{1,3}::[A-Za-z0-9.]+::(" + r"|".join(toDrop) + r"))"
    for field in re.split(pattern, expr):
        if field in toDrop: continue
        matches = re.fullmatch(r"([a-z0-9]{1,3})::[A-Za-z0-9.]+::(flow|gauge|temperature)", field)
        if matches:
            hashValue = matches[1]
            dataType = matches[2]
        print("Field", field, matches is not None)

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
    procGauge(tgt) # After rating
    procSource(tgt) # After gauge
    translateCalc(tgt, "expr") # After source
    # translateCalc(tgt, "time") # After source
    # procSection(tgt) # After gauge
    # procState(tgt) # After section
    # procGuideBook(tgt) # After section
    # procClass(tgt) # After section
    # procLevel(tgt) # After section
    tgt.execute("COMMIT;")
