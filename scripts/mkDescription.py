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
from Parameter import getParameter
import os
import time
from tempfile import NamedTemporaryFile
from FileSignature import fileSignature
import sys

def mkDescription(cur, fn, criteria):
    sql = "SELECT columnName,type,prefix,suffix FROM description ORDER BY sortKey;"
    cur.execute(sql)
    fields = cur.fetchall()

    cols = []
    for row in fields:
        cols.append(row["columnName"])

    # Now extract information from section file
    sql = "SELECT " + ",".join(cols) + " FROM section ORDER BY sortName;"
    cur.execute(sql)
    for row in cur:
        logging.info("row %s", row)
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
            cur = db.cursor()
            fn = args.fn if args.fn else \
                os.path.join(getParameter(cur, "webPageDir"), "description.html")
            mkDescription(db.cursor(dictionary=True), fn, None)
    except:
        logging.exception("Unexpected exception")
