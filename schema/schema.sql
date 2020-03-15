-- Basic schema is:
--
-- A gauge is a basic unit
-- For each gauge there can be multiple data sources
-- All the data is stored in one table pointing with a source link
-- 
-- There is a table of all the states
-- There is a table for each guide book 
--
-- For each section
--  It may point to a gauge
--  Since a section may be in multiple states, there is a section2state table
--  Since a section may be in multiple guide books, there is a section2guidebook table
 
-- Gauge information
DROP TABLE IF EXISTS gauge CASCADE;
CREATE TABLE gauge (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- Gauge table index key
    name VARCHAR(256) NOT NULL UNIQUE, -- Descriptive name
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
    rating INTEGER REFERENCES rating(id) ON DELETE SET NULL -- Which rating supplied this gauge
); -- gauge

-- Data types, flow, gauge, temperature, ...
DROP TABLE IF EXISTS datatype;
CREATE TABLE datatype (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- url index
    name VARCHAR(16) UNIQUE NOT NULL, -- 'flow', ...
    units VARCHAR(16) NOT NULL,
    INDEX (name)
); -- datatype

INSERT INTO datatype (name, units) VALUES
    ('gauge', 'feet'),
    ('flow', 'CFS'),
    ('inflow', 'CFS'), -- For reservoirs, the inflow
    ('temperature', 'F')
    ;

-- URLs for data sources
DROP TABLE IF EXISTS url CASCADE;
CREATE TABLE url (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- url index
    url VARCHAR(512) NOT NULL UNIQUE, -- URL to process
    parser VARCHAR(32) NOT NULL, -- which parser to process the URL with
    hours SET ('0', '1', '2', '3', '4', '5', 
               '6', '7', '8', '9', '10', '11', 
               '12', '13', '14', '15', '16', '17', 
               '18', '19', '20', '21', '22', '23'), -- Hour constraint
    qFetch BOOLEAN NOT NULL DEFAULT TRUE, -- Should this URL be fetched?
    t TIMESTAMP, -- Last time URL was fetched
    INDEX(url),
    INDEX(parser),
    INDEX(hours),
    INDEX(qFetch)
); -- URLs for data source

-- Calculation sources
DROP TABLE IF EXISTS calc CASCADE;
CREATE TABLE calc (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- calc index
    dataType INTEGER REFERENCES datatype(id) ON DELETE CASCADE, -- Data type produced
    expr VARCHAR(512) NOT NULL, -- How to calculate value
    time TEXT NOT NULL, -- What to calculate time from
    note TEXT,
    UNIQUE(dataType, expr)
); -- Derived gauges


-- Rating tables
DROP TABLE IF EXISTS rating CASCADE;
CREATE TABLE rating (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- rating index
    url VARCHAR(512) NOT NULL, -- where data is pulled from
    parser VARCHAR(32) NOT NULL, -- which parser to use
    UNIQUE(parser, url)
); -- Transform flow to gauge or vice versa

DROP TABLE IF EXISTS ratingData CASCADE;
CREATE TABLE ratingData (
    rating INTEGER REFERENCES rating(id) ON DELETE CASCADE, -- which rating table this row is from
    gauge FLOAT NOT NULL, -- gauge height in feet
    flow FLOAT NOT NULL CHECK (flow>=0), -- flow in CFS
    PRIMARY KEY(rating, gauge), -- For gauge to flow transforms
    INDEX (rating, flow) -- For flow to gauge transforms
); -- ratingData

-- Data sources
DROP TABLE IF EXISTS source CASCADE;
CREATE TABLE source (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- data source index
    url INTEGER REFERENCES url(id) ON DELETE CASCADE, -- Which URL supplied this data source
    calc INTEGER REFERENCES calc(id) ON DELETE CASCADE, -- Which calculation supplied this data source
    name VARCHAR(256) NOT NULL, -- Name of this data source
    agency TEXT, -- Which agency provides this source
    CHECK (((url IS NOT NULL) AND (calc IS NULL)) OR
           ((url IS NULL) AND (calc IS NOT NULL))),
    UNIQUE(name, url, calc),
    INDEX(name),
    INDEX(url),
    INDEX(calc)
); -- Data source

-- Gauge to source mapping
DROP TABLE IF EXISTS gauge2source CASCADE;
CREATE TABLE gauge2source (
    gauge INTEGER REFERENCES gauge(id) ON DELETE CASCADE,
    src INTEGER REFERENCES source(id) ON DELETE CASCADE,
    PRIMARY KEY(gauge, src),
    INDEX(gauge),
    INDEX(src)
);

-- Observations, calculations, ratings, ...
DROP TABLE IF EXISTS data CASCADE;
CREATE TABLE data (
    src INTEGER REFERENCES source(id) ON DELETE CASCADE, -- source of the data
    dataType INTEGER REFERENCES datatype(id) ON DELETE CASCADE, -- Data type produced
    value FLOAT NOT NULL, -- Observation
    t TIMESTAMP, -- When observation was taken
    CHECK (((datatype IN ('flow', 'inflow')) AND (value >= 0)) OR (value IS NOT NULL)),
    PRIMARY KEY(src, t, datatype),
    INDEX(src),
    INDEX(t)
);

-- River section
DROP TABLE IF EXISTS section CASCADE;
CREATE TABLE section (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- section table index key (Also used for hash value)
    tUpdate TIMESTAMP, -- Last time record was updated
    gauge INTEGER REFERENCES gauge(id)  ON DELETE SET NULL, -- which gauge to use
    name VARCHAR(64) not NULL UNIQUE, -- Name of the section, by default river(name)
    displayName TEXT NOT NULL, -- Name to be displayed
    sortName TEXT, -- How to sort within river sorting
    nature TEXT, -- Nature of the section
    description TEXT, -- Description of section
    difficulties TEXT, -- Description of difficulties in the section
    basin TEXT, -- Which basin the river is in
    basinArea FLOAT, -- Drainage area above this section in square miles
    elevation FLOAT, -- Elevation in feet of this section
    elevationLost FLOAT, -- Feet drop of this section
    length FLOAT, -- Length of the section in miles
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
    awID INTEGER -- American Whitewater river ID
);

-- Guide books
DROP TABLE IF EXISTS guideBook CASCADE;
CREATE TABLE guideBook (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- Guide book id
    title VARCHAR(256), -- Title of the guide book
    subTitle VARCHAR(256), -- Sub-title of the guide book
    edition VARCHAR(24), -- Which edition
    author TEXT, -- who wrote/published the guide book
    url TEXT, -- URL to the guide book
    UNIQUE (title, subTitle, edition) 
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

-- section to guide book  linkage
DROP TABLE IF EXISTS section2GuideBook;
CREATE TABLE section2GuideBook (
    section INTEGER REFERENCES section(id) ON DELETE CASCADE,
    guideBook INTEGER REFERENCES guideBook(id) ON DELETE CASCADE,
    page TEXT, -- page number
    run TEXT, -- run number
    url TEXT, -- url to particular section
    PRIMARY KEY(section, guideBook),
    INDEX (guideBook)
); -- section2GuideBook

-- List of states
DROP TABLE IF EXISTS state;
CREATE TABLE state (
    id INTEGER AUTO_INCREMENT PRIMARY KEY, -- Index of state
    short VARCHAR(2) NOT NULL UNIQUE, -- State name abbreviation
    name VARCHAR(24) NOT NULL UNIQUE, -- Full state name
    INDEX(short)
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
    name VARCHAR(20) UNIQUE, -- name of the level, low, okay, high, ...
    style VARCHAR(64) UNIQUE -- HTML style to use
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
    name VARCHAR(32) PRIMARY KEY, -- III, IV, ...
    description TEXT NOT NULL
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
    name VARCHAR(32) NOT NULL, -- display string
    low FLOAT, -- Lower limit, NULL -> no lower limit
    lowDatatype INTEGER NOT NULL REFERENCES dataType(id) ON DELETE CASCADE,
    high FLOAT, -- Upper limit, NULL -> no upper limit
    highDatatype INTEGER NOT NULL REFERENCES dataType(id) ON DELETE CASCADE,
    CHECK (low <= high),
    INDEX(section,name)
); -- Difficulty rating

--
-- Tables for creating web pages
--

-- Parameters
DROP TABLE IF EXISTS parameters CASCADE;
CREATE TABLE parameters (
    ident VARCHAR(64) PRIMARY KEY,
    value TEXT
); -- parameters

-- column descriptions
DROP TABLE IF EXISTS description CASCADE;
CREATE TABLE description (
    sortKey INTEGER UNIQUE, -- Sorting order of the rows
    columnName VARCHAR(32) UNIQUE, -- name of the columns
    type TEXT, -- noop, text, ...
    prefix TEXT, -- display prefix
    suffix TEXT, -- display suffix
    info TEXT
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

DELIMITER $$
CREATE PROCEDURE FetchData(
    IN gaugeID INTEGER,
    IN dt VARCHAR(8),
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

DELIMITER ;
