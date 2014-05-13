# upcloud-cvs-for-UPYUN
这个项目是受Git版本控制系统启发,通过clone远程云空间文件备份至本地,upcloud-cvs将跟踪记录用户对本地空间的管理操作(包括增加文件(夹),删除文件(夹)...),批量上传,下载等操作,从而实现云空间的高效有组织地管理.
## 安装方法

### 依赖包安装

```
$ sudo apt-get install libcurl4-openssl-dev       # 安装libcurl库
$ git clone https://github.com/upyun/c-sdk.git    # 安装又拍云API: c-sdk 
$ cd c-sdk
$ make
$ sudo cp libupyun.so /usr/local/lib/
$ sudo cp upyun.h /usr/local/include/
```

### upcloud-cvs安装

```
$ git clone git://gitcafe.com/xuefu_zhang/upcloud-cvs-for-UPYUN.git
$ make
$ sudo cp upc /usr/local/bin/
```

## 使用方法

upc使用上和git类似，通过clone云空间至本地后，用户就能在本地进行云空间的管理。add命令则可以将指定目录或文件添加至暂存区。
push则可以将暂存区中的跟踪文件（增加或删除）upload至云空间。reset可以清空暂存区，status能查看当前暂存区中的状态，
usage能获取当前空间已使用空间大小。

```
$ upc clone bucket_name@user_name
$ upc add .
$ upc push 

$ upc reset
$ upc status
$ upc usage
```

*代码写的有点乱，欢迎提交bug !*
