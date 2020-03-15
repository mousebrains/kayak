#
# Open a database connection
#
import mysql.connector
import argparse
import logging
import json
import os.path

class DB:
    """ Open a database connect and execute statments """
    def __init__(self, args:argparse.ArgumentParser, logger:logging.Logger) -> None:
        self.logger = logger
        self.args = self.__setupArgs(args)
        self.db = None

    @staticmethod
    def addArgs(parser:argparse.ArgumentParser) -> None:
        grp = parser.add_argument_group(description='Database options')
        grp.add_argument('--dbhost', type=str, metavar='hostname', help='Hostname of database')
        grp.add_argument('--dbuser', type=str, metavar='username', help='Username of database')
        grp.add_argument('--dbpasswd', type=str, metavar='password', help='Password of database')
        grp.add_argument('--dbname', type=str, metavar='name', help='Name of database to open')
        grp.add_argument('--dbrc', type=str, metavar='filename',
                         help='RC file to load, by default ~/.kayakingrc')

    def __del(self):
        if self.db is not None:
            self.db.close()

    def __setupArgs(self, args:argparse.ArgumentParser) -> argparse.ArgumentParser:
        if args.dbrc is None:
            args.dbrc = '~/.kayakingrc'
        args.dbrc = os.path.expanduser(args.dbrc)
        info = {}
        if os.path.isfile(args.dbrc): # Load info
            with open(args.dbrc, 'rb') as fp:
                info = json.load(fp)
        if args.dbhost is None and ('host' in info): args.dbhost = info['host']
        if args.dbuser is None and ('user' in info): args.dbuser = info['user']
        if args.dbpasswd is None and ('password' in info): args.dbpasswd = info['password']
        if args.dbname is None and ('database' in info): args.dbname = info['database']
        return args
 

    def __open(self):
        if self.db is None:
            args = self.args
            self.db = mysql.connector.connect(
                        host=args.dbhost, 
                        user=args.dbuser, 
                        passwd=args.dbpasswd,
                        database=args.dbname)

    def cursor(self):
        if self.db is None: self.__open()
        return self.db.cursor()

    def close(self):
        if self.db is not None:
            self.db.close()
            self.db = None
