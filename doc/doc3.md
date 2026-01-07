## 关于跨平台开发磁盘镜像不兼容的Bug

如果在 `Windows` 上开发好的系统想要在 `Ubuntu` 上运行，需要删掉 `Windows` 上创建的 `master.img`，在 `Ubuntu` 上使用 `bximage` 重新创建一个磁盘，磁盘信息如下。从 `ubuntu` 到 `Windows` 的迁移同理。
```bash
========================================================================
                                bximage
  Disk Image Creation / Conversion / Resize and Commit Tool for Bochs
         $Id: bximage.cc 13481 2018-03-30 21:04:04Z vruppert $
========================================================================

1. Create new floppy or hard disk image
2. Convert hard disk image to other format (mode)
3. Resize hard disk image
4. Commit 'undoable' redolog to base image
5. Disk image info

0. Quit

Please choose one [0] 1

Create image

Do you want to create a floppy disk image or a hard disk image?
Please type hd or fd. [hd] hd

What kind of image should I create?
Please type flat, sparse, growing, vpc or vmware4. flat

Choose the size of hard disk sectors.
Please type 512, 1024 or 4096. [512] 

Enter the hard disk size in megabytes, between 10 and 8257535
[10] 32

What should be the name of the image?
[c.img] master.img

Creating hard disk image 'master.img' with CHS=65/16/63 (sector size = 512)

The following line should appear in your bochsrc:
  ata0-master: type=disk, path="master.img", mode=flat
```

[返回README](../README.md)