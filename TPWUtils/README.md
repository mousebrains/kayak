# This is a subset of my TPWUtils collection of Python 3 utilities

- `Logger.py` set up a logger which supports console, rolling file, and/or SMTP logging methods 
  - `addArgs(parser:argparse.ArgumentParser)` adds command line arguments for setting up logging
  - `mkLogger(args:argparse.ArgumentParser, fmt:str, name:str)` uses the args to setup the logger
    - *fmt* is the logging message format, by default "%(asctime)s %(threadName)s %(levelname)s: %(message)s"
    - *name* is the logger to setup
