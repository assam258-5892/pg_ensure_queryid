# pg_inzent_helper

# Table of contents
- [Installation](#installation)
- [GUC](#guc)

# Installation
1. Build
```bash
$ make
$ make install
```

2. Modify postgresql.conf
```bash
shared_preload_libraries = 'pg_inzent_helper'
```

3. Restart postgres
```bash
pg_ctl restart
```

# GUC
pg_inzent_helper.use_query_id_tracking
- on : enable query_id tracking on extended PreferQueryMode (default)
- off : disable query_id tracking on extended PreferQueryMode
