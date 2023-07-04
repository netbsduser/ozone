$ pid := 'f$getjpi ("","pid")'
$ on warning then exit
$loop:
$ assign/user 'pid'.tmp ftptest_dat
$ assign/user ftptest.syi sys$input
$ ftp 65.85.95.150 'p1'
$ diff ftptest.dat 'pid'.tmp
$ if $severity .ne. 1 then exit
$ purge 'pid'.tmp
$ goto loop
