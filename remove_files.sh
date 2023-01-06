 #!/bin/sh
 sudo umount /newdisk
 sudo rm -rf /newdisk
 sudo gpart delete -i 1 ada1
