#! /usr/bin/env python3
#
# This builds a human parseable version of the information
# available for each section
#
# The main verion of this script is a PHP script, but this builds a complete database
#
# July-2022, Pat Welch, pat@mousebrains.com
#
import logging
from TPWUtils.int2hash import int2hash
from Parameter import getParameter
import os
import time
from tempfile import NamedTemporaryFile
from FileSignature import fileSignature
import sys

def mkNumber(val:float, units:str) -> str:
    if units != "Feet": val = int(val)
    return f"{val:,}{units}"

def mkHash(val) -> str:
    if val is None: logging.error("Hash value is None")
    return int2hash(0 if val is None else val)

def mkHeader(cur, prefix:str, suffix:str, val, hashVal:str, ident:int) -> str:
    if val is None: return None
    return f"<h1 id='{hashVal}'>{prefix}{val}{suffix}</h1>"

def mkText(cur, prefix:str, suffix:str, val, hashVal:str, ident:int) -> str:
    if val is None: return None
    return prefix + str(val) + suffix

def mkClass(cur, prefix:str, suffix:str, val, hashVal:str, ident:int) -> str:
    sql = "SELECT class.name,low,ldt.units,high,hdt.units FROM class"
    sql+= " LEFT JOIN dataType as ldt ON lowDatatype=ldt.id"
    sql+= " LEFT JOIN dataType as hdt ON highDatatype=hdt.id"
    sql+= " WHERE section=%s"
    sql+= " ORDER BY low,high"
    sql+= ";"
    cur.execute(sql, (ident,))
    rows = cur.fetchall()
    if not rows: return None # No class descriptions
    generic = None
    sequence = []
    for row in rows:
        if row[1] == None and row[2] == None:
            generic = row[0]
        else:
            if row[1] is not None:
                a = mkNumber(row[1], row[2])
                if not sequence or (sequence[-1] != a): # first or not same
                    sequence.append(a)
            sequence.append(row[0])
            if row[3] is not None: sequence.append(mkNumber(row[3], row[4]))

    if not sequence:
        output = generic
    else:
        output = "" if generic is None else (output + ", ")
        output += " &le; ".join(sequence)
    return prefix + output + suffix

def mkState(cur, prefix:str, suffix:str, val, hashVal:str, ident:int) -> str:
    sql = "SELECT name FROM section2state"
    sql+= " LEFT JOIN state ON state=id"
    sql+= " WHERE section=%s ORDER BY name;"
    cur.execute(sql, (ident,))
    states = []
    for row in cur: states.append(row[0])
    if not states: return None
    return prefix + ", ".join(states) + suffix

def mkGuideBook(cur, prefix:str, suffix:str, val, hashVal:str, ident:int) -> str:
    sql = "SELECT page,run,sgb.url,title,subtitle,edition,author,gb.url FROM guideBook as gb"
    sql+= " LEFT JOIN section2GuideBook AS sgb ON gb.id=sgb.guideBook"
    sql+= " WHERE sgb.section=%s;"
    cur.execute(sql, (ident,))
    rows = cur.fetchall()
    if not rows: return None
    entries = []
    for row in rows:
        entry = ""
        if row[0] is not None or row[1] is not None:
            if row[2] is not None: entry += f"<a href='{row[2]}'>"
            a = []
            if row[1] is not None: a.append(f"Run: {row[1]}")
            if row[0] is not None: a.append(f"Page: {row[0]}")
            entry += " ".join(a)
            if row[2] is not None: entry += "</a>"
            entry += " "
        if row[-1] is not None: entry += f"<a href='{row[-1]}'>"
        if row[3] is not None: entry += f"<h3>{row[3]}</h3>"
        if row[4] is not None: entry += f": <h4>{row[4]}</h4>"
        if row[5] is not None: entry += f" <i>{row[5]} edition</i>"
        if row[6] is not None: entry += f" By <i>{row[6]}</i>"
        if row[-1] is not None: entry += "</a>"
        entries.append(entry)
    return "\n".join(entries)

def mkDescription(cur, fields:tuple, row:dict) -> str:
    typFuncs = {
               "hash": None,
               "noop": None,
               "DB": None,
               "ptxt": None,
               "URL": None,
               "h1": mkHeader,
               "text": mkText,
               "class": mkClass,
               "state": mkState,
               "guideBook": mkGuideBook,
               }
    ident = row["id"]
    hashVal = int2hash(ident)
    output = [f"<div id='{hashVal}'>"]
    for field in fields:
        (name, typ, prefix, suffix) = field
        val = row[name] if name in row else None
        if typ in typFuncs:
            if typFuncs[typ]:
                a = typFuncs[typ](cur, prefix, suffix, val, hashVal, ident)
                if a: output.append(a)
        else:
            logging.warning("%s -> %s %s %s %s", name, val, typ, prefix, suffix)
    output.append("</div>")
    # for key in sorted(row): logging.info("row[%s] -> %s", key, row[key])
    return "\n".join(output)

def loadRows(cur0, cur1, fn:str, criteria:dict[str]) -> None:
    sql = "SELECT columnName,type,prefix,suffix FROM description ORDER BY sortKey;"
    cur0.execute(sql)
    fields = cur0.fetchall()

    # Now extract information from section file
    sql = "SELECT * FROM section"
    sql+= " WHERE noShow is NULL"
    critVal = []
    if criteria:
        critName = []
        for key in criteria:
            critName.append(key + "=%s")
            critVal.append(criteria[key])
        sql+= " AND " + " AND ".join(critName)
    sql+= " ORDER BY sortName;"

    cur1.execute(sql, critVal)
    output = []
    for row in cur1:
        a = mkDescription(cur0, fields, row)
        if a: output.append(a)

    logging.info("Output\n%s", "\n".join(output))
    sys.exit(1)

    cols = ["section.display_name",
           ]
    sql = "SELECT " + ",".join(cols)
    sql+= " FROM section"
    if criteria is not None: sql += " WHERE " + criteria
    sql += ";"
    logging.info("%s", sql)

if __name__ == "__main__":
    from argparse import ArgumentParser
    from TPWUtils import Logger
    from TPWUtils.DB import DB

    parser = ArgumentParser()
    DB.addArgs(parser) # Database related options
    Logger.addArgs(parser) # Logging related options
    parser.add_argument("-f", "--force", action="store_true",
        help="Force regeneration of descriptions")
    parser.add_argument("--fn", type=str, help="Output filename, overriding default")

    args = parser.parse_args()

    Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

    try:
        with DB(args) as db:
            cur0 = db.cursor(buffered=True)
            fn = args.fn if args.fn else \
                os.path.join(getParameter(cur0, "webPageDir"), "description.html")
            loadRows(cur0, db.cursor(buffered=True, dictionary=True), fn, None)
    except:
        logging.exception("Unexpected exception")
