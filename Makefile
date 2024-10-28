# contrib/pg_inzent_helper/Makefile
MODULE_big = pg_inzent_helper
OBJS = pg_inzent_helper.o
PGFILEDESC = "pg_inzent_helper - Helper for eXperDB"

EXTENSION = pg_inzent_helper
DATA = pg_inzent_helper--1.0.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

