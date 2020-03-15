#! /usr/bin/env python3
#
# Fetch pages from various data sources and store them in an internal database
#
# Mar-2020, Pat Welch, pat@mousebrains.com

import MyLogger
import MyDB
import argparse

parser = argparse.ArgumentParser(description='Fetch URLs and store them in internal database')
grp = parser.add_argument_group('Fetch related options')
grp.add_argument('--dryrun', '-d', action='store_true', help='Do not update the database')
grp.add_argument('--fetch', '-f', action='store_true', help='Only fetch pages, do not parse them')
grp.add_argument('--ignore', '-i', action='store_true', help='Ignore all constraints')
grp.add_argument('--name', '-n', action='store_true', help='Display URL being fetched')
grp.add_argument('--outdir', '-o', type=str, metavar='dir',
                 help='Directory to write fetched pages into')
grp.add_argument('--parser', '-p', type=str, metavar='parser',
                 help='Type of parser to select from database')
grp.add_argument('--prepend', '-P', type=str, metavar='prefix',
                 help='Prepend this director to all URLs being requested, file://dir/urls')
grp.add_argument('--type', '-t', type=str, metavar='parser', help='Type of parser to use')
grp.add_argument('--url', '-u', type=str, metavar='url', help='URL to select database records on')
grp.add_argument('--URL', '-U', type=str, metavar='url', help='URL to fetch and parse')

MyDB.DB.addArgs(parser) # Add in database options
MyLogger.addArgs(parser) # Add in logging related options

args = parser.parse_args()

logger = MyLogger.mkLogger(args, __name__)
logger.info('Args=%s', args)

db = MyDB.DB(args, logger)
