-- Database schema for WKCC water level's MySQL database
--
-- July-2022, Pat Welch, pat@mousebrains.com

DROP TABLE IF EXISTS rating CASCADE; -- Rating tables
CREATE TABLE rating ( -- Rating tables
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- rating index
  url TEXT NOT NULL, -- where data is pulled from
  parser TEXT NOT NULL, -- which parser to use
  t TIMESTAMP, -- Time of last data fetch
  UNIQUE(parser(256), url(512))
); -- Transform flow to gauge or vice versa

DROP TABLE IF EXISTS ratingData CASCADE;
CREATE TABLE ratingData (
  rating INTEGER REFERENCES rating(id) ON DELETE CASCADE ON UPDATE CASCADE, -- rating table row
  gauge FLOAT NOT NULL, -- gauge height in feet
  flow FLOAT NOT NULL CHECK (flow>=0), -- flow in CFS
  PRIMARY KEY(rating, gauge), -- For gauge to flow transforms
  INDEX (rating, flow) -- For flow to gauge transforms
); -- ratingData

DROP TABLE IF EXISTS gauge CASCADE; -- Gauge information
CREATE TABLE gauge ( -- Gauge information
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- Gauge table index key
  name TEXT NOT NULL, -- Descriptive name
  bankFull FLOAT CHECK(bankFull>0), -- Full to the normal bank, CFS
  floodStage FLOAT CHECK(floodStage>0), -- CFS when it is considered flooding
  location TEXT, -- Description of where the gauge is
  latitude FLOAT, -- Latitude in decimal degrees of the gauge
  longitude FLOAT, -- Longitude in decimal degrees of the gauge
  stationID TEXT,-- station number id
  cbttID TEXT,-- CBTT id
  geosID TEXT,-- GEOAS id
  nwsID TEXT, -- NWS id
  nwsliID TEXT, -- NWS li id
  snotelID TEXT, -- SNOTEL id
  usgsID TEXT, -- USGS id
  rating INTEGER REFERENCES rating(id) ON DELETE SET NULL, -- Which rating supplied this gauge
  UNIQUE(name(256))
); -- gauge

DROP TABLE IF EXISTS dataType CASCADE; -- Observation data types
CREATE TABLE dataType (
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- data type index
  name TEXT NOT NULL, -- 'flow', ...
  units TEXT NOT NULL,
  lowerLimit FLOAT NOT NULL DEFAULT -1e7, -- Minimum allowed value
  upperLimit FLOAT NOT NULL DEFAULT  1e7, -- Maximum allowed value
  UNIQUE (name(32)),
  INDEX (name(32))
); -- dataType

INSERT INTO dataType (name, units, lowerLimit, upperLimit) VALUES
    ('gauge',       'feet', -1e4, 5e5),
    ('flow',        'CFS',     0, 1e7),
    ('inflow',      'CFS',     0, 1e7), -- For reservoirs, the inflow
    ('temperature', 'F',      28, 140)
    ;

DROP TABLE IF EXISTS calc CASCADE; -- Calculation sources
CREATE TABLE calc ( -- Calculation sources
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- calc index
  dataType INTEGER REFERENCES datatype(id) ON DELETE CASCADE, -- Data type produced
  expr TEXT NOT NULL, -- How to calculate value
  time TEXT NOT NULL, -- What to calculate time from
  note TEXT,
  UNIQUE(dataType, expr(512))
); -- Derived gauges

DROP TABLE IF EXISTS url CASCADE; -- Data source
CREATE TABLE IF NOT EXISTS URL ( -- Data source URLs
  url TEXT, -- URL of the data source
  id INTEGER NOT NULL AUTO_INCREMENT UNIQUE, -- row id
  t TIMESTAMP, -- Time of last data fetch
  parser TEXT NOT NULL, -- which parser to process this URL with
  hours SET ('0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
	    '10', '11', '12', '13', '14', '15', '16', '17', '18', '19',
	    '20', '21', '22', '23'), -- hours to fetch the URL
  qFetch BOOLEAN NOT NULL DEFAULT TRUE, -- Should this URL be fetched?
  PRIMARY KEY(url(512)),
  INDEX(id),
  INDEX(parser(512)),
  INDEX(qFetch),
  INDEX(t)
); -- URL


DROP TABLE IF EXISTS data CASCADE; -- Observations
CREATE TABLE IF NOT EXISTS data ( -- Observations
  id INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY, -- Row identification
  t TIMESTAMP NOT NULL, -- Time of the observation
  url INTEGER REFERENCES url(id) ON DELETE CASCADE ON UPDATE CASCADE, -- Data source URL
  dataType INTEGER REFERENCES dataType(id) ON DELETE CASCADE ON UPDATE CASCADE, -- data type
  value FLOAT, -- Value of the observation
  INDEX (t),
  INDEX(url)
); -- data

DROP TABLE IF EXISTS section CASCADE; -- River section
CREATE TABLE section ( -- River section
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- section table index key (Also used for hash value)
  tUpdate TIMESTAMP, -- Last time record was updated
  gauge INTEGER REFERENCES gauge(id)  ON DELETE SET NULL, -- which gauge to use
  name TEXT not NULL, -- Name of the section, by default river(name)
  displayName TEXT NOT NULL, -- Name to be displayed
  sortName TEXT, -- How to sort within river sorting
  nature TEXT, -- Nature of the section
  description TEXT, -- Description of section
  difficulties TEXT, -- Description of difficulties in the section
  basin TEXT, -- Which basin the river is in
  basinArea FLOAT, -- Drainage area above this section in square miles
  elevation FLOAT, -- Elevation in feet of this section
  elevationLost FLOAT, -- Feet drop of this section
  sectionLength FLOAT, -- Length of the section in miles
  gradient FLOAT, -- feet/mile gradient
  features TEXT, -- Description of features in the section
  latitude FLOAT, -- Latitude in decimal degrees of the section
  longitude FLOAT, -- Longitude in decimal degrees of the section
  latitudeStart FLOAT, -- Latitude in decimal degrees of the section
  longitudeStart FLOAT, -- Longitude in decimal degrees of the section
  latitudeEnd FLOAT, -- Latitude in decimal degrees of the section
  longitudeEnd FLOAT, -- Longitude in decimal degrees of the section
  mapName TEXT, -- Name of map to use
  noShow BOOLEAN, -- Should this section be displayed?
  notes TEXT, -- Extra notes on this section
  optimalFlow FLOAT, -- optimal flow level
  region TEXT, -- Region of the secction
  remoteness TEXT, -- How remote is the section
  scenery TEXT, -- What is the scenery like?
  season TEXT, -- When to run the section
  watershedType TEXT, -- What type of watershed is this section
  awID INTEGER, -- American Whitewater river ID
  UNIQUE(name(512))
);

DROP TABLE IF EXISTS guideBook CASCADE; -- Guide books
CREATE TABLE guideBook ( -- Guide books
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- Guide book id
  title TEXT, -- Title of the guide book
  subTitle TEXT, -- Sub-title of the guide book
  edition TEXT, -- Which edition
  author TEXT, -- who wrote/published the guide book
  url TEXT, -- URL to the guide book
  UNIQUE (title(256), subTitle(256), edition(32)) 
); -- guideBook

INSERT INTO guideBook (title, subTitle, edition, author, url) VALUES
  ('The Soggy Sneakers', 'Guide to Oregon Rivers', '2nd', 
              'Willamette Kayak and Canoe Club', 'https://www.wkcc.org'),
  ('Soggy Sneakers', 'Guide to Oregon Rivers', '3rd', 
              'Willamette Kayak and Canoe Club', 'https://www.wkcc.org'),
  ('Soggy Sneakers', 'Guide to Oregon Rivers', '4th', 
              'Willamette Kayak and Canoe Club', 'https://www.wkcc.org'),
  ('Soggy Sneakers', 'Guide to Oregon Rivers', '5th', 
              'Willamette Kayak and Canoe Club', 'https://www.wkcc.org'),
  ('Idaho', 'The Whitewater State', '1st', 'Grant Amaral', 
              'https://www.goodreads.com/book/show/1231857.Idaho_the_Whitewater_State'),
  ('A Guide to the Whitewater Rivers of Washington', '', '2nd', 'Jeff and Tonya Bennet',
              'https://www.goodreads.com/book/show/2625965-a-guide-to-the-whitewater-rivers-of-washington--over-320-trips-for-raft'),
  ('Paddling Oregon', '', '1st', 'Robb Keller', 
              'https://www.goodreads.com/book/show/1328369.Paddling_Oregon'),
  ('American Whitewater', '', '', 'American Whitewater',
              'https://www.americanwhitewater.org/content/River/view/?')
;

DROP TABLE IF EXISTS section2GuideBook; -- section to guide book  linkage
CREATE TABLE section2GuideBook ( -- section to guide book  linkage
  section INTEGER REFERENCES section(id) ON DELETE CASCADE,
  guideBook INTEGER REFERENCES guideBook(id) ON DELETE CASCADE,
  page TEXT DEFAULT NULL, -- page number (May contain letters)
  run TEXT DEFAULT NULL, -- run number (May contain letters)
  url TEXT DEFAULT NULL, -- url to particular section
  PRIMARY KEY(section, guideBook),
  INDEX (guideBook)
); -- section2GuideBook

-- List of states
DROP TABLE IF EXISTS state;
CREATE TABLE state (
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- Index of state
  short TEXT NOT NULL, -- State name abbreviation
  name TEXT NOT NULL, -- Full state name
  UNIQUE(short(4)),
  INDEX(short(4)),
  UNIQUE(name(25))
); -- List of state

-- Which state does a section belong to
DROP TABLE IF EXISTS section2state CASCADE;
CREATE TABLE section2state (
  section INTEGER NOT NULL REFERENCES section(id) ON DELETE CASCADE, 
  state INTEGER NOT NULL REFERENCES state(id) ON DELETE CASCADE,
  PRIMARY KEY (section, state),
  INDEX (state)
); -- section -> state mapping

-- level classification, low, okay, high, ...
DROP TABLE IF EXISTS level;
CREATE TABLE level (
  id INTEGER AUTO_INCREMENT PRIMARY KEY, -- index
  name TEXT, -- name of the level, low, okay, high, ...
  style TEXT, -- HTML style to use
  UNIQUE(name(32)),
  UNIQUE(style(256))
); -- level

INSERT INTO level (name, style) VALUES 
    ('Low', 'lowFlow'),
    ('Okay', 'okayFlow'),
    ('High', 'highFlow');

-- section to level linkage
DROP TABLE IF EXISTS section2level;
CREATE TABLE section2level (
  section INTEGER NOT NULL REFERENCES section(id) ON DELETE CASCADE,
  level INTEGER NOT NULL REFERENCES level(id) ON DELETE CASCADE,
  low FLOAT, -- lower limit for this level, NULL is no lower limit
  lowDatatype INTEGER NOT NULL REFERENCES dataType(id) ON DELETE CASCADE,
  high FLOAT, -- upper limit for this level, NULL is no upper limit
  highDatatype INTEGER NOT NULL REFERENCES dataType(id) ON DELETE CASCADE,
  PRIMARY KEY (section, level),
  INDEX (level)
); -- section2level

-- Class descriptions
DROP TABLE IF EXISTS classDescription CASCADE;
CREATE TABLE classDescription (
  name TEXT, -- III, IV, ...
  description TEXT NOT NULL,
  PRIMARY KEY(name(32))
);

INSERT INTO classDescription VALUES
  ('I', 'Fast moving water with riffles and small waves. Few obstructions, all obvious and easily missed with little training. Risk to swimmers is slight; self-rescue is easy'),
  ('II', 'Straightforward rapids with wide, clear channels which are evident without scouting. Occasional maneuvering may be required, but rocks and medium-sized waves are easily avoided by trained paddlers. Swimmers are seldom injured and group assistance, while helpful, is seldom needed. Rapids that are at the upper end of this difficulty range are designated Class II+.'),
  ('III', 'Rapids with moderate, irregular waves which may be difficult to avoid and which can swamp an open canoe. Complex maneuvers in fast current and good boat control in tight passages or around ledges are often required; large waves or strainers may be present but are easily avoided. Strong eddies and powerful current effects can be found, particularly on large-volume rivers. Scouting is advisable for inexperienced parties. Injuries while swimming are rare; self-rescue is usually easy but group assistance may be required to avoid long swims. Rapids that are at the lower or upper end of this difficulty range are designated Class III- or Class III+ respectively.'),
  ('IV', 'Intense, powerful but predictable rapids requiring precise boat handling in turbulent water. Depending on the character of the river, it may feature large, unavoidable waves and holes or constricted passages demanding fast maneuvers under pressure. A fast, reliable eddy turn may be needed to initiate maneuvers, scout rapids, or rest. Rapids may require "must make" moves above dangerous hazards. Scouting may be necessary the first time down. Risk of injury to swimmers is moderate to high, and water conditions may make self-rescue difficult. Group assistance for rescue is often essential but requires practiced skills. For kayakers, a strong roll is highly recommended. Rapids that are at the lower or upper end of this difficulty range are designated Class IV- or Class IV+ respectively.'),
  ('V', 'Extremely long, obstructed, or very violent rapids which expose a paddler to added risk. Drops may contain large, unavoidable waves and holes or steep, congested chutes with complex, demanding routes. Rapids may continue for long distances between pools, demanding a high level of fitness. What eddies exist may be small, turbulent, or difficult to reach. At the high end of the scale, several of these factors may be combined. Scouting is recommended but may be difficult. Swims are dangerous, and rescue is often difficult even for experts. Proper equipment, extensive experience, and practiced rescue skills are essential.
Because of the large range of difficulty that exists beyond Class IV, Class V is an open-ended, multiple-level scale designated by class 5.0, 5.1, 5.2, etc. Each of these levels is an order of magnitude more difficult than the last. That is, going from Class 5.0 to Class 5.1 is a similar order of magnitude as increasing from Class IV to Class 5.0.'),
  ('VI', 'Runs of this classification are rarely attempted and often exemplify the extremes of difficulty, unpredictability and danger. The consequences of errors are severe and rescue may be impossible. For teams of experts only, at favorable water levels, after close personal inspection and taking all precautions. After a Class VI rapid has been run many times, its rating may be changed to an appropriate Class 5.x rating.')
;

-- How to calculate class of a section
DROP TABLE IF EXISTS class CASCADE;
CREATE TABLE class (
  section INTEGER REFERENCES section(id) ON DELETE CASCADE, -- Which section this applies to
  name TEXT NOT NULL, -- display string
  low FLOAT, -- Lower limit, NULL -> no lower limit
  lowDatatype INTEGER NOT NULL REFERENCES dataType(id) ON DELETE CASCADE,
  high FLOAT, -- Upper limit, NULL -> no upper limit
  highDatatype INTEGER NOT NULL REFERENCES dataType(id) ON DELETE CASCADE,
  CHECK (low <= high),
  INDEX(section,name(256))
); -- Difficulty rating

--
-- Tables for creating web pages
--

-- Parameters
DROP TABLE IF EXISTS parameters CASCADE;
CREATE TABLE parameters (
  ident TEXT,
  value TEXT,
  PRIMARY KEY(ident(512))
); -- parameters

-- column descriptions
DROP TABLE IF EXISTS description CASCADE;
CREATE TABLE description (
  sortKey INTEGER UNIQUE, -- Sorting order of the rows
  columnName TEXT, -- name of the columns
  type TEXT, -- noop, text, ...
  prefix TEXT, -- display prefix
  suffix TEXT, -- display suffix
  info TEXT,
  UNIQUE(columnName(128))
); -- description


-- How build a web page
DROP TABLE IF EXISTS builder CASCADE;
CREATE TABLE builder(
  sortKey INTEGER PRIMARY KEY,
  qUse BOOLEAN,
  type TEXT,
  field TEXT,
  length INTEGER,
  nameText TEXT,
  nameHTML TEXT
); -- builder

-- web page description
DROP TABLE IF EXISTS description CASCADE;
CREATE TABLE description (
  sortKey INTEGER PRIMARY KEY,
  columnName TEXT,
  type TEXT,
  prefix TEXT,
  suffix TEXT,
  info TEXT
); -- description

DROP TABLE IF EXISTS edit CASCADE;
CREATE TABLE edit (
  sortKey INTEGER PRIMARY KEY,
  type TEXT,
  field TEXT,
  width INTEGER,
  height INTEGER,
  description TEXT,
  footnote TEXT
); -- edit

DROP TABLE IF EXISTS mapBuilder CASCADE;
CREATE TABLE mapBuilder (
  sortKey INTEGER PRIMARY KEY,
  qUse BOOLEAN,
  type TEXT,
  field TEXT,
  length INTEGER,
  nameText TEXT,
  nameHTML TEXT
); -- mapBuilder

-- and now for some stored procedures

DROP PROCEDURE IF EXISTS FetchData;

DELIMITER $$ -- change delimiter from semicolon to $$ for stored procedures
CREATE PROCEDURE FetchData(
  IN gaugeID INTEGER,
  IN dt TEXT,
  IN n INTEGER,
  OUT value FLOAT,
  OUT t TIMESTAMP)
BEGIN
  SELECT value, t FROM data
  WHERE src IN (SELECT src FROM gauge2source WHERE gauge=gaugeID)
        AND
        datatype=(SELECT id FROM datatype WHERE name=dt)
  ORDER BY t DESC
  LIMIT n
  ;
END $$

DELIMITER ; -- restore delimiter to semicolon
