-------------------------------------------------------------------------------
-- XXX
-- just a sample yet
-- later full settings depending on application will be developed here
--
-------------------------------------------------------------------------------

-- schema version : 1
-- used for migration and schema upgrade
PRAGMA  user_version = 1;

-- file size shirnks automatically
PRAGMA  auto_vacuum = FULL;

-- obvious
PRAGMA  encoding = "UTF-8";

-------------------------------------------------------------------------------
--
--
-- CLI configuration
--
--
-------------------------------------------------------------------------------
CREATE TABLE cli (
  prompt            TEXT NOT NULL
);

CREATE TABLE cli_telnet (
  enabled           INTEGER,
  port              INTEGER
);

CREATE TABLE cli_serial (
  enabled           INTEGER,
  port              TEXT NOT NULL,
  baud              INTEGER,
  data_bit          INTEGER,
  stop_bit          INTEGER,
  parity            TEXT NOT NULL
);

-- CLI defaults
INSERT INTO cli (prompt) values
                ("macron_bog> ");
INSERT INTO cli_telnet (enabled,  port) values
                       (1, 9090);
INSERT INTO cli_serial (enabled, port, baud, data_bit, stop_bit, parity) values
                       (0, "/dev/ttyUSB0", 115200, 8, 1, "none");
