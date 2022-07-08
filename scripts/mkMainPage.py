#! /usr/bin/env python3
#
# This builds the entry page for levels, i.e. displays states
#
from argparse import ArgumentParser
from TPWUtils.DB import DB
from TPWUtils import Logger
import logging
import os
import os.path
import time
from tempfile import NamedTemporaryFile

def getParameter(cur, ident:str) -> str:
    cur.execute("SELECT value FROM parameters WHERE ident=%s;", (ident,))
    for row in cur: return row[0]
    return None
  
def loadFile(fn):
    if not os.path.isfile(fn): return str()

    with open(fn, "r") as fp:
        return fp.read()

def mkStates(tgt) -> list:
    sql = "CREATE TEMPORARY TABLE mkIndex"
    sql+= " SELECT src,count(src) AS n FROM data"
    sql+= " WHERE t>(CURRENT_TIMESTAMP - INTERVAL 10 day) GROUP BY src;"
    tgt.execute(sql)
    sql = "SELECT DISTINCT state.name,state.short FROM mkIndex"
    sql+= " INNER JOIN gauge2source ON mkIndex.src=gauge2source.src"
    sql+= " INNER JOIN section ON gauge2source.gauge=section.gauge"
    sql+= "                    AND (section.noShow IS NULL OR not section.noShow)"
    sql+= " INNER JOIN section2state ON section.id=section2state.section"
    sql+= " INNER JOIN state ON section2state.state=state.id"
    sql+= " ORDER BY state.name;"
    tgt.execute(sql)
    a = []
    for row in tgt: a.append((row[0], row[1]))
    tgt.execute("DROP TABLE mkIndex;")
    return a
  
def mkRow(key, label):
    line = "<tr>"
    line+= f"<th><a href='?P={key}.html'>{label}</a>&nbsp;</th>"
    line+= f"<td><a href='?P={key}.text'>Text Version</a></td>"
    line+= "</tr>\n"
    return line

def mkIndex(tgt, fn, head, tail): 
    logging.info("Making %s", fn) 
    states = mkStates(tgt)

    dirname = os.path.dirname(indexFile)
    os.makedirs(dirname, mode=0o775, exist_ok=True)

    with NamedTemporaryFile(mode="w", dir=dirname, delete=False) as fp:
        tfn = fp.name
        fp.write(head)
        fp.write("<table id=indexTable>\n")
        fp.write(mkRow("All", "All States"))
        for state in states: fp.write(mkRow(state[1], state[0]))
        fp.write("</table>\n")
        fp.write(tail)

    os.chmod(tfn, 0o664)
    os.rename(tfn, fn) 
    
parser = ArgumentParser()
parser.add_argument("-f", "--force", action="store_true", help="Force rebuilding main page")
DB.addArgs(parser) # Database related options
Logger.addArgs(parser) # Logging related options
args = parser.parse_args()

Logger.mkLogger(args, fmt="%(asctime)s %(levelname)s: %(message)s")

try:
    with DB(args) as tgtDB:
        cur = tgtDB.cursor()

        templateDir = os.path.join(getParameter(cur, "rootDir"), getParameter(cur, "templateDir"))
        templateHead = os.path.join(templateDir, getParameter(cur, "web.pre.filename"))
        templateTail = os.path.join(templateDir, getParameter(cur, "web.post.filename"))
        indexFile = os.path.join(getParameter(cur, "webPageDir"), "index.html")

        mTime = None if not os.path.isfile(indexFile) else os.path.getmtime(indexFile)
        qRebuild = args.force \
            or (mTime is None) \
            or (mTime < (time.time() - 86400)) \
            or (mTime <= (os.path.getmtime(templateHead) if os.path.isfile(templateHead) else 0)) \
            or (mTime <= (os.path.getmtime(templateTail) if os.path.isfile(templateTail) else 0))

        if qRebuild:
            mkIndex(cur, indexFile, loadFile(templateHead), loadFile(templateTail))
except:
    logging.exception("Unexpected exception")
