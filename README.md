# upcloud-cvs
这个项目是受Git版本控制系统启发,通过clone远程云空间文件备份至本地,upcloud-cvs将跟踪记录用户对本地空间的管理操作(包括增加文件(夹),删除文件(夹), 修改文件...),批量上传,下载等操作,从而实现云空间的高效有组织地管理.

## 演示

![showtime](https://raw.githubusercontent.com/xuefu/upcloud-cvs/master/show.gif)

## 安装方法

### 依赖包安装

> 依赖包的安装可尝试直接运行`upcloud-cvs`中的`install-dependency`脚本

* [libcurl库](https://github.com/bagder/curl)
* [又拍云API: c-sdk](https://github.com/upyun/c-sdk)

### upcloud-cvs安装

```
$ git clone https://github.com/xuefu/upcloud-cvs.git
$ make
$ make install
```

## 使用方法

upc使用上和git类似，通过clone云空间至本地后，用户就能在本地进行云空间的管理。add命令则可以将指定目录或文件添加至暂存区(建议在本地空间根目录下直接`upc add .`)。push则可以将暂存区中的跟踪文件（增加,删除或修改）upload至云空间。reset可以清空暂存区，status能查看当前暂存区中的状态，usage能获取当前空间已使用空间大小。

```
$ upc clone bucket_name@user_name  # 将云空间文件克隆至本地目录，该目录名字即为云空间名。一旦克隆成功，该云空间
             # 的一些信息(用户名，空间名)就保存在本地空间配置文件中，但为了安全起见，用户名密码未保存

$ upc add .  # 强烈建议在本地空间根目录使用这个命令将更新加入至暂存区，其它add指定目录或文件可能会有未知bug
$ upc push   # 将暂存区中的更改(包括文件增加，删除和修改)推送至云空间

$ upc reset  # 清空暂存区
$ upc status # 查看当前暂存区中的状态
$ upc usage  # 获取云空间已使用空间大小
```

## LICENSE

本项目基于MIT协议发布
MIT: http://www.opensource.org/licenses/MIT
