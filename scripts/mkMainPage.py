#! /usr/bin/env python3
#
# This builds the entry page for levels, i.e. displays states
#
# July-2022, Pat Welch, pat@mousebrains.com
#
from argparse import ArgumentParser
from TPWUtils.DB import DB
from TPWUtils import Logger
import logging
from Parameter import getParameter
import os
import os.path
import time
from tempfile import NamedTemporaryFile
from FileSignature import fileSignature

def loadFile(fn:str):
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
  
def mkRow(key:str, label:str) -> str:
    line = "<tr>"
    line+= f"<th><a href='?P={key}.html'>{label}</a>&nbsp;</th>"
    line+= f"<td><a href='?P={key}.text'>Text Version</a></td>"
    line+= "</tr>\n"
    return line

def mkIndex(tgt, fn:str, head:str, tail:str): 
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

    sig0 = fileSignature(fn)
    sig1 = fileSignature(tfn)
    if not sig0 or not sig1 or (sig0 != sig1):
        logging.info("Moving %s -> %s", tfn, fn)
        os.chmod(tfn, 0o664)
        os.rename(tfn, fn) 
    else:
        logging.info("Unlinking %s", tfn)
        os.unlink(tfn)
    
parser = ArgumentParser()
DB.addArgs(parser) # Database related options
Logger.addArgs(parser) # Logging related options
parser.add_argument("-f", "--force", action="store_true", help="Force rebuilding main page")
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
        else:
            logging.info("No need to rebuild %s", indexFile)
except:
    logging.exception("Unexpected exception")
