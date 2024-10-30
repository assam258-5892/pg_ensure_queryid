# contrib/pg_ensure_queryid/Makefile
MODULE_big = pg_ensure_queryid
OBJS = pg_ensure_queryid.o
PGFILEDESC = "pg_ensure_queryid - Helper for eXperDB"

EXTENSION = pg_ensure_queryid
DATA = pg_ensure_queryid--1.0.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

