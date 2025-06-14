'''
@Author: wei
@Date: 2020-06-28 09:00:06
@LastEditors: Do not edit
@LastEditTime: 2020-06-28 09:40:56
@Description: file content
@FilePath: /script/py/oltp_run.py
'''

import os
import sys
from configparser import ConfigParser
import subprocess

# @param config_path
config_path = "./config.ini" # sys.argv[1]


cfg = ConfigParser()
cfg.read(config_path)

oltp_path = cfg.get('oltp_env','oltp_path')
workload = cfg.get('oltp_env','workload')
load_data = int(cfg.get('oltp_env','load_data'))
output_file = cfg.get('oltp_env','output_file')

node_count = int(cfg.get("run_env","node_count"))
out_dir = cfg.get("build_env","out_dir")

oltp_subcommand = "./oltpbenchmark -b "
create_subcommand = " --create=true --load=true"
execute_subcommand = " --execute=true -s 5 -o "

res = []

for i in range(1,node_count+1):
    oltp_command = oltp_subcommand + workload + " -c " + out_dir + "/" + workload  + "_" + str(i) + ".xml"
    if(load_data != 0):
        oltp_command += create_subcommand
    else:
        oltp_command += execute_subcommand + output_file
    r = subprocess.Popen(oltp_command,shell=True,cwd=oltp_path)
    res.append(r)

for j in range(1,node_count+1):
    print(res[j-1].communicate())