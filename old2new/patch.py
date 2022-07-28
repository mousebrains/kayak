#! /usr/bin/env python3
# 
# Patch up output of copyInfo for new database structure
#
# July-2022, Pat Welch, pat@mousebrains.com

from argparse import ArgumentParser
from TPWUtils import Logger
from TPWUtils.DB import DB
import logging

def patchDescription(tgt) -> None: # Patch up the info, so after copyInfo.py
    tgt.execute("SET SQL_SAFE_UPDATES=0;") # Turn off safe updates

    # Drop rows
    sql = "DELETE FROM description WHERE columnName=%s;"
    for item in ("Calc_expr", "Calc_notes", "Calc_time", "RunNumber", "PageNumber",
		"Bank_Full", "Flood_Stage", "Gauge_Location",
		):
        tgt.execute(sql, (item,))

    # Change columnName
    sql = "UPDATE description SET columnName=%s WHERE columnName=%s;"
    for item in ("Latitude", "Longitude", "Region", "Length", "Remoteness", "Season", "Gradient",
                 "Scenery", "Features", "Remoteness", "Nature", "Difficulties", "Description",
                 ("id", "hashValue"),
                 ("Drainage", "Gauge"),
                 ("Low_Flow", "lowFlow"),
                 ("Optimal_Flow", "optimalFlow"),
                 ("High_Flow", "highFlow"),
                 ("displayName", "display_name"),
                 ("elevationLost", "Elevation_Lost"),
                 ("watershedType", "Watershed_type"),
                 ):
       if not isinstance(item, tuple):
           item = (item.lower(), item)
       tgt.execute(sql, item)

    # Insert new records
    for item in (
		(1450, "elevation", "text", "Elevation", "Feet<br />", "Patched in"),
		):
        tgt.execute("REPLACE INTO description VALUES(%s,%s,%s,%s,%s,%s);", item)

    # Change fields filtered on columName
    for item in (
		("id", ("type", "hash")),
		("dislayName", ("type", "h1")),
		("Gauge", ("type", "gauge"), ("prefix", None), ("Suffix", None), ("sortKey", 600)),
		("Class", ("type", "class")),
		("State", ("type", "state")),
		("Guide_Book", ("type", "guideBook")),
		("Notes", ("type", "text"), ("prefix", "Notes:<br /><pre>"), ("suffix", "</pre><br />")),
		):
        toSet = []
        values = []
        for a in item[1:]:
            toSet.append(a[0] + "=%s")
            values.append(a[1])
        values.append(item[0])
        sql = "UPDATE description SET " + ",".join(toSet) + " WHERE columnName=%s"
        tgt.execute(sql, values)

    tgt.execute("SET SQL_SAFE_UPDATES=1;") # Turn on safe updates

parser = ArgumentParser()
DB.addArgs(parser) # Database related arguments
Logger.addArgs(parser)
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

with DB(args) as tgtDB:
    tgt = tgtDB.cursor()
    tgt.execute("BEGIN;")
    # patchDescription(tgt) # No longer being used
    tgt.execute("COMMIT;")
