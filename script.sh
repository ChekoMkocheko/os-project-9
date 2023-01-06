#!/bin/sh
sudo gpart create -s GPT ada1
sudo gpart add -t freebsd-ufs -a 1M ada1
sudo newfs -U /dev/ada1p1
sudo mkdir /newdisk
sudo mount /dev/ada1p1 /newdisk/
cd /newdisk/
sudo chmod 1777 . 
cd /newdisk/
mkdir root_ch1
mkdir root_ch2
cd root_ch1
mkdir rt_ch11_files
mkdir rt_ch12_files
cd rt_ch11_files
cp ~/file_sys/gen_files/many/manyfiles.c .
gcc manyfiles.c
./a.out
cp ~/file_sys/gen_files/gen_files.c .
gcc gen_files.c
./a.out
mv test.txt rt_ch11_test.txt
cd /newdisk/root_ch2
mkdir rt_ch21_files
cd rt_ch21_files
cp ~/file_sys/gen_files/gen_files2.c .
gcc gen_files2.c
./a.out
mv test.txt rt_ch21_test.txt
cd /newdisk/root_ch1/rt_ch12_files
cp ~/file_sys/gen_files/gen_files3.c .
gcc gen_files3.c
./a.out
mv test.txt rt_ch12_test.txt
cd ..
touch text_two.txt
echo "wasn't slow" > text_two.txt
cd /newdisk/
mkdir manyfiles
cd manyfiles
cp ~/file_sys/gen_files/many/manyfiles.c .
gcc manyfiles.c
./a.out
cd /newdisk/
touch sam.txt
echo "Tired" > sam.txt
sudo umount /newdisk
sudo dd if=/dev/ada1p1 of=/home/cheko/partition.img
sudo umount /newdisk
sudo dd if=/dev/ada1p1 of=/home/cheko/partition.img

