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
from stateAbbreviations import state2code
import sys

def copyTable(tgt, newTbl, oldCols, oldDB, oldTbl):
    cols = ",".join(oldCols) if oldCols else "*"
    sql = "INSERT IGNORE INTO " + newTbl
    sql+= " SELECT " + cols + " FROM " + oldDB + "." + oldTbl
    sql+= ";"
    tgt.execute(sql);

def chompURLs(tgt) -> dict:
    # First fill url table from levels_data.url2name as inactive URLs
    name2url = {}
    tgt.execute('SELECT url,name FROM levels_data.url2name;')
    rows = tgt.fetchall() # Get all the rows so I can reuse tgt
    sql = 'INSERT IGNORE INTO URL (url,qFetch) VALUES(%s,FALSE);'
    for row in rows:
      (url, name) = row
      name2url[name] = url
      tgt.execute(sql, (url,))

    tgt.execute('SELECT URL,parser,hours from levels_information.URLparse;')
    rows = tgt.fetchall() # Get all the rows so I can reuse tgt
    for row in rows:
        (url, parser, hours) = row
        args = [url, True]
        cols = ["url", "qFetch"]
        if parser and len(parser):
           args.append(parser)
           cols.append("parser")

        hrs = set(range(24))
        if hours: # 
            for hr in hours.split(','): hrs.remove(abs(int(hr)))
        args.append(','.join(map(str, hrs)))
        cols.append("hours")

        sql = "INSERT INTO URL (" + ",".join(cols) + ")"
        sql+= " VALUES (" + ",".join(["%s"] * len(args))
        sql+= ") ON DUPLICATE KEY UPDATE parser=VALUES(parser),hours=VALUES(hours),qFetch=TRUE;"
        tgt.execute(sql, args)

    # Now get url ids
    tgt.execute('SELECT id,url FROM URL;')

    url2id = {}
    for row in tgt: url2id[row[1]] = row[0]

    name2id = {}
    for name in name2url: name2id[name] = url2id[name2url[name]]

    return name2id

def procGauge(tgt, row):
    if row['db_name'] is None: return None

    mapping = {'bank_full': 'bankFull', 'flood_stage': 'floodStage', 'gauge_location': 'location',
        'latitude': 'latitude',
        'longitude': 'longitude',
        'StationNumber': 'stationID',
        'cbtt_id': 'cbttID',
        'geos_id': 'geosID',
        'nws_id': 'nwsID',
        'nwsli_id': 'nwsliID',
        'snotel_id': 'snotelID',
        'usgs_id': 'usgsID',
        'ratingID': 'rating'}

    names = ['name']
    fmt = ['%s']
    values = [row['db_name']]

    for key in mapping:
        if row[key] is not None: 
            names.append(mapping[key])
            values.append(row[key])
            fmt.append('%s')

    sql = 'INSERT IGNORE INTO gauge ({}) VALUES({});'.format(','.join(names), ','.join(fmt))
    tgt.execute(sql, values)
    tgt.execute('SELECT id FROM gauge WHERE name=%s', (values[0],))
    for item in tgt: return item[0]
    return None

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

def procRating(tgt, row:list) -> int:
    if row['cfs_to_gauge_data'] is None: return None
    url = row['cfs_to_gauge_data']
    parser = row['cfs_to_gauge_converter']
    tgt.execute('INSERT IGNORE INTO rating (url, parser) VALUES(%s, %s);', (url, parser))
    tgt.execute('SELECT id FROM rating WHERE parser=%s AND url=%s', (parser, url))
    for item in tgt: return item[0]
    return None

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

def mkSectionName(row:list, sectionNames:set) -> str:
    keys = ['name', 'cbtt_id', 'geos_id', 'nws_id', 'nwsli_id', 'snotel_id', 'usgs_id', 'sort_key']
    for key in keys:
        if (row[key] is not None) and (row[key] not in sectionNames):
            sectionNames.add(row[key])
            return row[key]

    logging.warning('Unable to build section name')
    for key in sorted(row): logging.warning('row[%s] = %s', key, row[key])
    raise Exception('Unable to build section name')

def procSection(tgt, row):
    # for key in sorted(row): logging.info('row[%s] = %s', key, row[key])

    mapping = {'gaugeID': 'gauge', 
               'sort_key': 'sortName',
               'Nature': 'nature',
               'description': 'description',
               'difficulties': 'difficulties',
               'display_name': 'displayName',
               'drainage': 'basin',
               'drainage_area': 'basinArea',
               'elevation': 'elevation',
               'elevation_lost': 'elevationLost',
               'length': 'length',
               'gradient': 'gradient',
               'high_flow': 'highFlow',
               'features': 'features',
               'latitude': 'latitude',
               'longitude': 'longitude',
               'low_flow': 'lowFlow',
               'map_name': 'mapName',
               'no_show': 'noShow',
               'notes': 'notes',
               'optimal_flow': 'optimalFlow',
               'region': 'region',
               'remoteness': 'remoteness',
               'scenery': 'scenery',
               'season': 'season',
               'watershed_type': 'watershedType'
              };

    names = ['name']
    fmt = ['%s']
    values = [row['sectionName']]

    for key in mapping:
        if row[key] is not None: 
            names.append(mapping[key])
            values.append(row[key])
            fmt.append('%s')

    sql = 'INSERT INTO section ({}) VALUES({});'.format(','.join(names), ','.join(fmt))

    tgt.execute(sql, values)
    tgt.execute('SELECT id FROM section WHERE name=%s', (values[0],))
    for item in tgt: return item[0]
    return None

def buildState(tgt, row, states) -> dict:
    if row['state'] is None: return None
    ids = []
    for state in row['state'].split():
        if state not in states:
            name = re.sub('_', ' ', state)
            codigo = state2code[name] if name in state2code else None
            tgt.execute('INSERT INTO state (short,name) VALUES(%s,%s);', (codigo, name))
            states[state] = tgt.lastrowid
        ids.append(states[state])
          
    return ids

def procState(tgt, row, states) -> None:
    if row['stateID'] is None: return
    for state in row['stateID']:
        tgt.execute('INSERT INTO section2state (section,state) VALUES(%s,%s);',
                    (row['sectionID'], state));
      
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
    cur.execute('SELECT * FROM Master ORDER BY river_name;')
    rows = cur.fetchall()
    hash2row = {}
    states = {}
    sectionNames = set()
    datatypes = mkDatatypes(tgt)

    for row in rows: # populate gauge table
        hash2row[row['HashValue']] = row
        row['sectionName'] = mkSectionName(row, sectionNames)
        row['ratingID'] = procRating(tgt, row)
        row['gaugeID'] = procGauge(tgt, row)
        row['stateID'] = buildState(tgt, row, states)

    for row in rows: # Populate calc table, which needs a fully populated hash2row dictionary
        row['calcID'] = procCalc(tgt, row, hash2row, datatypes)
        procSource(tgt, row, name2url)
        row['sectionID'] = procSection(tgt, row)
        procState(tgt, row, states)
        procGuideBook(tgt, row)
        procStatus(tgt, row, datatypes)
        procClass(tgt, row, datatypes)

def chompInfo(tgt):
    name = "levels_information"
    copyTable(tgt, 'parameters', None, name, 'Parameters')
    copyTable(tgt, 'description', None, name, 'Description')
    copyTable(tgt, 'edit', None, name, 'Edit')
    copyTable(tgt, 'mapBuilder', None, name, 'MapBuilder')

parser = ArgumentParser()
DB.addArgs(parser) # Database related arguments
Logger.addArgs(parser)
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

with DB(args) as tgtDB, DB(args, "levels_information") as srcDB:
    tgt = tgtDB.cursor()
    tgt.execute("BEGIN;")
    chompInfo(tgt)
    name2url = chompURLs(tgt)
    src = srcDB.cursor(dictionary=True)
    chompMaster(tgt, src, name2url)
    tgt.execute("COMMIT;")
