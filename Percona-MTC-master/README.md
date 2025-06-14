# Percona-MTC
share-storage percona-server with MTC plugin

## Introduction
+ This project is based on percona server which is a open source fork of MySQL.    
+ The MTC is a multi-master solution implemented as a plugin based on commit of `a07c0b61daf4e9a2ae1f17b60a7447d2256a2badt`.     
+ This project is used for evaluated performance for MTC solution in multi-master share-storage architecture.        
+ The MTC is compatible with MySQL，which can be accessed by normal mysql client.       

## Install 
Because the MTC is base on MySQL. 
Developers can build and install it reference by [Building MySQL from Source](https://dev.mysql.com/doc/mysql-sourcebuild-excerpt/8.0/en/).

Another option is to build and deploy MTC instances by referring to some of the tool scripts in this project, which are located in the scripts directory.

1. shell scripts : basic scripts for building and controling a percona server  instance.
````
# script 
  # percona_build_env.sh
  # percona_cmake.sh
  # percona_make.sh
  # percona_server_connect.sh
  # percona_server_init.sh
  # percona_server_run.sh
  # percona_server_shutdown.sh
````

2. python scripts : python scripts for running oltpbench、generating default mysql config files and deploying multiple percona-MTC instances
````
# script
  # py
    # build_config_file.py
    # oltp_run.py
    # percona_output_gen.py
    # percona_server_connect.py
    # percona_server_run.py
    # tpcc_load_memory.sql
  # template
````

## Evaluation
> [OLTPBenchmark](https://github.com/oltpbenchmark/oltpbench) is a multi-threaded load generator. The framework is designed to be able to produce variable rate, variable mixture load against any JDBC-enabled relational database. The framework also provides data collection features, e.g., per-transaction-type latency and throughput logs.

We use [oltpbenchmark/oltpbench](https://github.com/oltpbenchmark/oltpbench) to run TPC-C benchmark. Using MySQL JDBC driver, it is easy to run many types of benchmark to evaluate system performance.

Developers can also use other benchmark tools which are compatible with mysql. 

## Directory
+ **script** some tool scripts for percona-MTC
+ **percona-server** source code for percona-MTC , the interface of MTC is in `percona-server/plugin/multi_master_log_plugin/`.



