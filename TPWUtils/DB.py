#
# Open a database connection
#
import mysql.connector
from argparse import ArgumentParser
import logging
import yaml
import os.path
import sys

class DB:
    """ Open a database connect and execute statments """
    def __init__(self, args:ArgumentParser, dbName:str = None) -> None:
        self.__info = self.__mkInfo(args, dbName)
        self.__db = None

    @staticmethod
    def addArgs(parser:ArgumentParser) -> None:
        grp = parser.add_argument_group(description="Database options")
        grp.add_argument("--dbhost", type=str, metavar="hostname", help="Hostname of database")
        grp.add_argument("--dbuser", type=str, metavar="username", help="Username of database")
        grp.add_argument("--dbpasswd", type=str, metavar="password", help="Password of database")
        grp.add_argument("--dbname", type=str, metavar="name", help="Name of database to open")
        grp.add_argument("--dbrc", type=str, metavar="filename", default="~/.kayakingrc",
                         help="RC file to load")

    def __enter__(self): # For with
        self.__open()
        return self

    def __exit__(self, excType, excValue, traceback): # For with
        self.close()


    def __del__(self):
        if self.__db is not None:
            self.__db.close()

    def __mkInfo(self, args:ArgumentParser, dbName:str) -> ArgumentParser:
        args.dbrc = os.path.abspath(os.path.expanduser(args.dbrc))
        if not os.path.isfile(args.dbrc):
            logging.error("%s does not exist", args.dbrc)
            sys.exit(1)
        try:
            with open(args.dbrc, 'rb') as fp: info = yaml.safe_load(fp)
            if args.dbhost is not None: info["hostname"] = args.dbhost
            if args.dbuser is not None: info["username"] = args.dbuser
            if args.dbpasswd is not None: info["password"] = args.dbpasswd
            if args.dbname is not None: info["database"] = args.dbname
            if dbName: info["database"] = dbName
        except:
            logging.exception("Error processing %s", args.dbrc)
            return None

        for key in ["hostname", "username", "password", "database"]:
            if key not in info:
                logging.error("%s is not in %s", key, args.dbrc)

        return info

    def __open(self):
        if self.__db is None and self.__info:
            info = self.__info
            self.__db = mysql.connector.connect(
                        host=info["hostname"],
                        user=info["username"],
                        passwd=info["password"],
                        database=info["database"])

    def cursor(self, dictionary:bool=False):
        if self.__db is None: self.__open()
        return self.__db.cursor(dictionary=dictionary)

    def close(self):
        if self.__db is not None:
            self.__db.close()
            self.__db = None
