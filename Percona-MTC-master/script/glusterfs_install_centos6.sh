# !/bin/bash

wget https://buildlogs.centos.org/centos/6/storage/x86_64/gluster-3.12/glusterfs-3.12.2-1.el6.x86_64.rpm --no-check-certificate

wget https://buildlogs.centos.org/centos/6/storage/x86_64/gluster-3.12/glusterfs-api-3.12.2-1.el6.x86_64.rpm --no-check-certificate

wget https://buildlogs.centos.org/centos/6/storage/x86_64/gluster-3.12/glusterfs-cli-3.12.2-1.el6.x86_64.rpm --no-check-certificate

wget https://buildlogs.centos.org/centos/6/storage/x86_64/gluster-3.12/glusterfs-client-xlators-3.12.2-1.el6.x86_64.rpm --no-check-certificate

wget https://buildlogs.centos.org/centos/6/storage/x86_64/gluster-3.12/glusterfs-fuse-3.12.2-1.el6.x86_64.rpm --no-check-certificate

wget https://buildlogs.centos.org/centos/6/storage/x86_64/gluster-3.12/glusterfs-libs-3.12.2-1.el6.x86_64.rpm --no-check-certificate

wget https://buildlogs.centos.org/centos/6/storage/x86_64/gluster-3.12/glusterfs-server-3.12.2-1.el6.x86_64.rpm --no-check-certificate

sudo rpm -ivh glusterfs-libs-3.12.2-1.el6.x86_64.rpm

sudo rpm -ivh glusterfs-3.12.2-1.el6.x86_64.rpm

sudo rpm -ivh glusterfs-client-xlators-3.12.2-1.el6.x86_64.rpm

sudo rpm -ivh glusterfs-fuse-3.12.2-1.el6.x86_64.rpm

sudo rpm -ivh glusterfs-cli-3.12.2-1.el6.x86_64.rpm

sudo rpm -ivh glusterfs-api-3.12.2-1.el6.x86_64.rpm

sudo rpm -ivh glusterfs-server-3.12.2-1.el6.x86_64.rpm

## http://www.rpmfind.net/linux/rpm2html/search.php?query=liburcu-bp.so.1()(64bit)

# gluster volume create gv0 stripe 3 transport tcp 10.11.1.191:/home/weixiaoxian/share-storage/data/brick1/gv0 10.11.1.193:/home/weixiaoxian/share-storage/data/brick1/gv0 10.11.1.194:/home/weixiaoxian/share-storage/data/brick1/gv0

# mount -t glusterfs 10.11.1.191:/gv0 /home/weixiaoxian/share-storage/mnt